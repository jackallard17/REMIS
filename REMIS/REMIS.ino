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
  #define STEPPER_DIR 9

  // Digital pins D2 - D7
  #define ROT_CLK 2 // rotary encoder clock
  #define ROT_DT 3  // rotary encoder direction
  #define ROT_SW 4  // rotary encoder switch (press in)

  #define LCD_SDA 22
  #define LCD_SDL 21

  #define TOGGLESWITCH 8
  #define TRIGGER 9

  #define STEPS 200

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

  //LiquidCrystal_I2C lcd(0x27);//, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address
  LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  // Set the LCD I2C address and pinout

  // Encoder /////////////////////////////////////
  #define encA 2
  #define encB 3
  //this encoder has a button here
  #define encBtn 4

  encoderIn<encA,encB> encoder;//simple quad encoder driver
  #define ENC_SENSIVITY 4
  encoderInStream<encA,encB> encStream(encoder,ENC_SENSIVITY);// simple quad encoder fake Stream

  //a keyboard with only one key as the encoder button
  keyMap encBtn_map[]={{-encBtn,defaultNavCodes[enterCmd].ch}};//negative pin numbers use internal pull-up, this is on when low
  keyIn<1> encButton(encBtn_map);//1 is the number of keys

  serialIn serial(Serial);

  //input from the encoder + encoder button + serial
  menuIn* inputsList[]={&encStream,&encButton,&serial};
  chainStream<3> in(inputsList);//3 is the number of inputs

  #define LEDPIN A3

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

  //customizing a prompt look!
  //by extending the prompt class
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

  /*extern menu mainMenu;
  TOGGLE((mainMenu[1].enabled),togOp,"Op 2:",doNothing,noEvent,noStyle
    ,VALUE("Enabled",enabledStatus,doNothing,noEvent)
    ,VALUE("disabled",disabledStatus,doNothing,noEvent)
  );*/

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
    //,OP("Calibrate",action2,focusEvent)
    //,SUBMENU(subMenu)
    // ,OP("Op1",action1,anyEvent)
    // //,SUBMENU(togOp)
    // ,OP("LED On",myLedOn,enterEvent)
    // ,OP("LED Off",myLedOff,enterEvent)
    // ,SUBMENU(selMenu)
    // ,SUBMENU(chooseMenu)
    // ,OP("Alert test",doAlert,enterEvent)
    // ,EDIT("Hex",buf1,hexNr,doNothing,noEvent,noStyle)

  #define MAX_DEPTH 2

  /*const panel panels[] MEMMODE={{0,0,16,2}};
  navNode* nodes[sizeof(panels)/sizeof(panel)];
  panelsList pList(panels,nodes,1);
  idx_t tops[MAX_DEPTH];
  lcdOut outLCD(&lcd,tops,pList);//output device for LCD
  menuOut* constMEM outputs[] MEMMODE={&outLCD};//list of output devices
  outputsList out(outputs,1);//outputs list with 2 outputs
  */

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
    // nav.idleOn(alert);

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
    //print REMIS on left side of screen and battery icon on right
    o.print("REMIS");
    o.setCursor(15, 0);
    o.write(1);

    // on the next line, display the current mode on left and rpm on right
    o.setCursor(0, 1);
    //if else block to print the textual injector mode, 0 = dose, 1 = continuous, 2 = toggle
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

    // switch(e) {
    //   case idleStart:o.print("suspending menu!");break;
    //   case idling:o.print("suspended...");break;
    //   case idleEnd:o.print("resuming menu.");break;
    // }
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

bool checkPumpInputs()
{
  if (digitalRead(TOGGLESWITCH) == HIGH)
  {
    runPump();
  }
  else if (digitalRead(TOGGLESWITCH) == LOW)
  {
    stopPump();
  }

  if (digitalRead(TRIGGER) == HIGH)
  {
    runPump();
  }
  else if (digitalRead(TRIGGER) == LOW)
  {
    stopPump();
  }

  return false;
}

void runPump()
{
  if (pumpRunning == false)
  {
    displayPumpRunning();
    pumpRunning = true;
    dashboardDisplayed = false;
  }

  digitalWrite(STEPPER_STEP, HIGH);
  digitalWrite(LED_BUILTIN, HIGH);
  delayMicroseconds(step_delay_microseconds);
  digitalWrite(STEPPER_STEP, LOW);
  digitalWrite(LED_BUILTIN, LOW);
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
  pumpRunning = false;
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
    pinMode(LEDPIN,OUTPUT);
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

    // if (digitalRead(TOGGLESWITCH) == LOW)
    // {
    //   // runPump();
    // }
    // else if (digitalRead(TOGGLESWITCH) == HIGH)
    // {
    //   pumpRunning = false;
    //   nav.idleOn(idle);
    // }
  }

#endif