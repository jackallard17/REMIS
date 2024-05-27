#include <Arduino.h>

#ifndef ARDUINO_SAM_DUE

  #include <Wire.h>
  #include <LiquidCrystal_I2C.h>//F. Malpartida LCD's driver
  #include <menu.h>//menu macros and objects
  #include <menuIO/lcdOut.h>//malpartidas lcd menu output
  #include <menuIO/serialIn.h>//Serial input
  #include <menuIO/encoderIn.h>//quadrature encoder driver and fake stream
  #include <menuIO/keyIn.h>//keyboard driver and fake stream (for the encoder button)
  #include <menuIO/chainStream.h>// concatenate multiple input streams (this allows adding a button to the encoder)
  #include <Stepper.h>

  // globals
  #define STEPPER_STEP 5
  #define STEPPER_DIR 6

  // Digital pins D2 - D7
  #define ROT_CLK 2 // rotary encoder clock
  #define ROT_DT 3  // rotary encoder direction
  #define ROT_SW 4  // rotary encoder switch (press in)

  #define LCD_SDA 22
  #define LCD_SDL 21

  #define TOGGLESWITCH 7
  #define TRIGGER 8

  #define STEPS 200

  #define MAX_DEPTH 2

  volatile bool pumpRunning = false;
  volatile bool dashboardDisplayed = true;
  volatile bool frequencyUpdated = false;

  float period;
  long step_delay_microseconds;

  float ml_per_rev = 200;

  volatile int last_CLK_state = LOW;
  volatile int last_DT_state = LOW;

  Stepper stepper(STEPS, STEPPER_DIR, STEPPER_STEP);

  // persistent properties, stored between power cycles
  float frequency;  // frequency of the pump in Hz
  int dose;         // dose amount in mL

  uint8_t mushroom[8] = {
    0b00000,
    0b00100,
    0b01110,
    0b11111,
    0b00100,
    0b00100,
    0b00100,
    0b00100
};

byte batteryIcon[8] = {
  B00100,
  B01110,
  B01110,
  B01110,
  B01110,
  B01110,
  B01110,
  B00000
};

  using namespace Menu;

  LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address and pinout

  // Encoder
  #define encA 2
  #define encB 3
  #define encBtn 4

  encoderIn<encA,encB> encoder;
  #define ENC_SENSIVITY 4
  encoderInStream<encA,encB> encStream(encoder,ENC_SENSIVITY);

  keyMap encBtn_map[]={{-encBtn,defaultNavCodes[enterCmd].ch}};
  keyIn<1> encButton(encBtn_map);

  serialIn serial(Serial);

  //input from the encoder + encoder button + serial
  menuIn* inputsList[]={&encStream,&encButton,&serial};
  chainStream<3> in(inputsList);//3 is the number of inputs

  result doCalibration(eventMask e, prompt &item);

  result showEvent(eventMask e,navNode& nav,prompt& item) {
    Serial.print("event: ");
    Serial.println(e);
    return proceed;
  }

  int test=420;

  result action1(eventMask e,navNode& nav, prompt &item) {
    Serial.print("action1 event: ");
    Serial.print(e);
    Serial.println(", proceed menu");
    Serial.flush();
    return proceed;
  }

  result action2(eventMask e,navNode& nav, prompt &item) {
    Serial.print("action2 event: ");
    Serial.print(e);
    Serial.println(", quiting menu.");
    Serial.flush();
    return quit;
  }

  int injectorMode=1;

  TOGGLE(injectorMode,setInjectorMode,"Mode: ",doNothing,noEvent,noStyle//,doExit,enterEvent,noStyle
    ,VALUE("Dose",0,doNothing,noEvent)
    ,VALUE("Cont.",1,doNothing,noEvent)
    ,VALUE("Toggle.",2,doNothing,noEvent)
  );

  int selTest=0;
  SELECT(selTest,selMenu,"Select",doNothing,noEvent,noStyle
    ,VALUE("Zero",0,doNothing,noEvent)
    ,VALUE("One",1,doNothing,noEvent)
    ,VALUE("Two",2,doNothing,noEvent)
  );

  int chooseTest=-1;
  CHOOSE(chooseTest,chooseMenu,"Choose",doNothing,noEvent,noStyle
    ,VALUE("First",1,doNothing,noEvent)
    ,VALUE("Second",2,doNothing,noEvent)
    ,VALUE("Third",3,doNothing,noEvent)
    ,VALUE("Last",-1,doNothing,noEvent)
  );

  class altPrompt:public prompt {
  public:
    altPrompt(constMEM promptShadow& p):prompt(p) {}
    Used printTo(navRoot &root,bool sel,menuOut& out, idx_t idx,idx_t len,idx_t) override {
      return out.printRaw(F("special prompt!"),len);;
    }
  };

  MENU(subMenu,"",showEvent,anyEvent,noStyle
    ,OP("Sub1",showEvent,anyEvent)
    ,OP("Sub2",showEvent,anyEvent)
    ,OP("Sub3",showEvent,anyEvent)
    ,altOP(altPrompt,"",showEvent,anyEvent)
    ,EXIT("<Back")
  );

  char* constMEM hexDigit MEMMODE="0123456789ABCDEF";
  char* constMEM hexNr[] MEMMODE={"0","x",hexDigit,hexDigit};
  char buf1[]="0x11";

  MENU(mainMenu,"Main menu",doNothing,noEvent,noStyle
    ,FIELD(test,"Speed: ","RPM",0,500,10,1,doNothing,noEvent,noStyle)
    ,OP("================",doNothing,enterEvent)
    ,SUBMENU(setInjectorMode)
    ,OP("Calibrate",doCalibration,enterEvent)
    ,EXIT("<Back")
  );

  MENU_OUTPUTS(out,MAX_DEPTH
    ,LCD_OUT(lcd,{0,0,16,2})
    ,NONE
  );
  NAVROOT(nav,mainMenu,MAX_DEPTH,in,out);//the navigation root object

  result alert(menuOut& o,idleEvent e) {
    if (e==idling) {
      o.setCursor(0,0);
      o.print("alert test");
      o.setCursor(0,1);
      o.print("[select] to continue...");
    }
    return proceed;
  }

result doCalibration(eventMask e, prompt &item) {
    bool calibrationComplete = false;

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Dispense 100mL");
    lcd.setCursor(0, 1);
    lcd.print("Press to cancel");

    if (digitalRead(encBtn) == LOW)
    {
      nav.idleOn(idle);
    }

    while (!calibrationComplete)
    {
      checkPumpInputs();
    }

    return proceed;
  }

  result idle(menuOut& o,idleEvent e) {
    lcd.setCursor(0, 0);
    o.print("REMIS");
    o.setCursor(15, 0);
    o.write(1);

    o.setCursor(0, 1);
    if (injectorMode == 0) {
      o.print("Dose");
    } else if (injectorMode == 1) {
      o.print("Cont.");
    } else if (injectorMode == 2) {
      o.print("Toggle");
    }
    o.setCursor(10, 1);
    o.print(test);
    o.print("RPM");

    return proceed;
  }

  void drawMushrooms()
{
  lcd.setCursor(0, 1);
  for (int i = 0; i < 16; i++)
  {
    lcd.write((byte)0);
    delay(100);
  }
}

// enter pump loop on toggle/trigger input
// we use the interrupt to stop the pump
bool checkPumpInputs()
{
  if (digitalRead(TOGGLESWITCH) == HIGH)
  {
    runPump();
  }

  if (digitalRead(TRIGGER) == HIGH)
  {
    runPump();
  }

  return false;
}

void stopPumpISR() // ISR for stopping the pump
{
  stopPump();
}

void runPump()
{
  if (pumpRunning == false)
  {
    displayPumpRunning();
    pumpRunning = true;
    dashboardDisplayed = false;
    attachInterrupt(digitalPinToInterrupt(TOGGLESWITCH), stopPumpISR, CHANGE); // attach interrupt to TOGGLESWITCH pin
    attachInterrupt(digitalPinToInterrupt(TRIGGER), stopPumpISR, CHANGE); // attach interrupt to TRIGGER pin

  }

  while (pumpRunning)
  {
    pumpStep();
  }
}

void pumpStep()
{
  digitalWrite(STEPPER_STEP, HIGH);
  delayMicroseconds(step_delay_microseconds);
  digitalWrite(STEPPER_STEP, LOW);
  delayMicroseconds(step_delay_microseconds);
}

void displayPumpRunning()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Pump Running...");
}

void stopPump()
{
  if (pumpRunning)
  {
    pumpRunning = false;
    dashboardDisplayed = true;
    detachInterrupt(digitalPinToInterrupt(TOGGLESWITCH)); // detach the interrupt when not needed
    detachInterrupt(digitalPinToInterrupt(TRIGGER)); // detach the interrupt when not needed
  }
}

int getRPM()
{
  float rpmFloat = (frequency * 60.0) / 200.0;
  return (int)rpmFloat;
}

void debugMode()
{
  // print debug mode on screen
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Debug Mode");

  delay(1000);
  lcd.clear();

  while (true)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("T:");
    lcd.print(digitalRead(TOGGLESWITCH));

    lcd.setCursor(10, 0);
    lcd.print("Tr:");
    lcd.print(digitalRead(TRIGGER));

    lcd.setCursor(0, 1);
    lcd.print("RC:");                // rotary code (raw binary)
    lcd.print(digitalRead(ROT_CLK)); // clk
    lcd.print(digitalRead(ROT_DT));  // dt
    lcd.print(digitalRead(ROT_SW));  // sw

    delay(100);

    if (digitalRead(ROT_SW) == LOW)
    {
      break;
    }
  }
}

  void setup() 
  {
    pinMode(encBtn,INPUT_PULLUP);
    Serial.begin(115200);
    while(!Serial);
    encoder.begin();
    lcd.begin(16,2);
    nav.idleTask=idle;//point a function to be used when menu is suspended
    mainMenu[1].enabled=disabledStatus;
    nav.showTitle=false;

    lcd.createChar(0, mushroom);
    lcd.createChar(1, batteryIcon);
    lcd.setCursor(0, 0);
    lcd.print("REMIS       v0.5");
    lcd.setCursor(0, 12);
    drawMushrooms();

    nav.idleOn(idle);

    step_delay_microseconds = 500;
  }

  void showDashboard()
  {
    if (!dashboardDisplayed)
    {
      nav.idleOn(idle);
      dashboardDisplayed = true;
    }
  }

  void loop() {
    nav.poll();

    // if the rotary encoder button is held for 5 seconds, enter debug mode
  if (digitalRead(ROT_SW) == LOW)
  {
    int i = 0;
    while (i < 500)
    {
      if (digitalRead(ROT_SW) == HIGH)
      {
        break;
      }
      delay(10);
      i++;
    }
    if (i == 500)
    {
      debugMode();
    }
  }

    if (!checkPumpInputs())
    {
      showDashboard();
    }
  }

#endif