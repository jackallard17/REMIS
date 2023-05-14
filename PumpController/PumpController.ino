#include <Stepper.h>
#include <Encoder.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

using namespace std;

LiquidCrystal_I2C lcd(0x27, 2, 16);
Encoder encoder(2, 3);

// Digital pins D2 - D7
#define ROT_CLK 2 // rotary encoder clock
#define ROT_DT 3  // rotary encoder direction
#define ROT_SW 4  // rotary encoder switch (press in)

#define STEPPER_STEP 5
#define STEPPER_DIR 9

#define LCD_SDA 22
#define LCD_SDL 21

#define TOGGLE 8
#define TRIGGER 9

#define STEPS 200

uint8_t mushroom[8] = {
    0b00000,
    0b00100,
    0b01110,
    0b11111,
    0b00100,
    0b00100,
    0b00100,
    0b00100};

String menuItems[] = {"Injector Mode", "Flow Rate", "Settings"}; // debug/diagnostics mode is hidden
int mainIndex = 0;
int prevIndex;
bool submenuVisited = false;
bool pumpRunning = false;

Stepper stepper(STEPS, STEPPER_DIR, STEPPER_STEP);

// persistent properties, stored between power cycles
float frequency;  // frequency of the pump in Hz
int injectorMode; // 0 = toggle, 1 = continuous, 2 = dose mode
int dose;         // dose amount in mL

float period;
long step_delay_microseconds;

float ml_per_rev = 200;

volatile int last_CLK_state = LOW;
volatile int last_DT_state = LOW;

void setup()
{
  Serial.begin(9600);

  // LCD init and startup message
  lcd.init();
  lcd.backlight();
  lcd.createChar(0, mushroom);
  lcd.setCursor(0, 0);
  lcd.print("REMIS       v0.0");
  lcd.setCursor(0, 12);
  drawMushrooms();

  // on first startup, initialize the persistent properties
  // if (EEPROM.read(0) == 255)
  // {
  writeIntToEEPROM(1100, 0);
  EEPROM.write(2, 0);
  EEPROM.write(3, 10);
  // }

  frequency = readIntFromEEPROM(0);
  frequency = frequency;
  injectorMode = EEPROM.read(2);
  dose = EEPROM.read(3);

  // print all three of these values to the serial monitor
  Serial.print("Frequency: ");
  Serial.println(frequency);
  Serial.print("Injector Mode: ");
  Serial.println(injectorMode);
  Serial.print("Dose: ");
  Serial.println(dose);

  period = 1.0 / frequency; // period of the pump in seconds
  step_delay_microseconds = (period / 2) * 1000000;

  // inputs
  pinMode(TOGGLE, INPUT_PULLUP);
  pinMode(TRIGGER, INPUT_PULLUP);
  pinMode(ROT_CLK, INPUT_PULLUP);
  pinMode(ROT_DT, INPUT_PULLUP);
  pinMode(ROT_SW, INPUT);

  attachInterrupt(digitalPinToInterrupt(ROT_CLK), updateFrequency, CHANGE);

  pinMode(LED_BUILTIN, OUTPUT); // Set onboard LED pin as output

  // outputs
  pinMode(STEPPER_DIR, OUTPUT);
  pinMode(STEPPER_STEP, OUTPUT);

  lcd.clear();
  lcd.print(menuItems[mainIndex]);
  prevIndex = (encoder.read() / 4) % 3; // store inital state of rotary encoder

  digitalWrite(10, HIGH);
}

void loop()
{
  // manual mode is triggered by the toggle switch
  if (digitalRead(TOGGLE) == LOW)
  {
    runPump();
  }
  else if (digitalRead(TRIGGER) == LOW && injectorMode == 0)
  {
    runPump();
  }
  else if (digitalRead(TRIGGER) == LOW && injectorMode == 1)
  {
    int steps = 0;
    int doseSteps = dose * ml_per_rev;

    while (steps < doseSteps)
    {
      runPump();
      steps++;
    }
  }
  else
  {
    digitalWrite(STEPPER_STEP, LOW);
    digitalWrite(LED_BUILTIN, LOW);
    if (pumpRunning == true)
    {
      lcd.clear();
      lcd.print(menuItems[mainIndex]);
      pumpRunning = false;
    }
  }

  if (!pumpRunning)
  {
    mainMenu();
  }

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
}

void mainMenu()
{
  delay(50); // debounce

  if (submenuVisited)
  {
    lcd.clear();
    lcd.print(menuItems[mainIndex]);
    submenuVisited = false;
  }

  int mainIndex = (encoder.read() / 4) % 3; // calculate the index

  if (mainIndex < 0) // if the index is negative
  {
    mainIndex += 3; // wrap around to the last item
  }

  if (mainIndex != prevIndex)
  {
    lcd.clear();
    lcd.print(menuItems[mainIndex]);
    prevIndex = mainIndex;
  }

  if (digitalRead(ROT_SW) == LOW) // if rotary button pressed:
  {
    lcd.clear();
    submenuVisited = true;
    delay(150);
    // enter the submenu for the current menu item
    switch (mainIndex)
    {
    case 0:
      injectorModeMenu();
      break;
    case 1:
      flowRateMenu();
      break;
    case 2:
      settingsMenu();
      break;
    }
  }
}

void injectorModeMenu()
{
  String injectorModeMenuItems[] = {"Continuous", "Dose Mode", "Back"};
  bool optionSelected = false;
  int index = 0;
  lcd.print(injectorModeMenuItems[index]);

  while (!optionSelected)
  {
    int index = (encoder.read() / 4) % 3; // calculate the index

    if (index < 0) // if the index is negative
    {
      index += 3; // wrap around to the last item
    }

    if (index != prevIndex)
    {
      lcd.clear();
      lcd.print(injectorModeMenuItems[index]);
      prevIndex = index;
    }

    if (digitalRead(ROT_SW) == LOW) // if rotary button pressed:
    {
      optionSelected = true;
      delay(100);
      if (index == 0)
      {
        injectorMode = 0;
        // EEPROM.write(2, 0);
      }
      else if (index == 1)
      {
        injectorMode = 1;
        // EEPROM.write(2, 1);
      }
      else if (index == 2)
      {
        mainMenu();
      }
    }
  }
}

void flowRateMenu()
{
}

void settingsMenu()
{
  String settingsMenuItems[] = {"Set Dose", "Calibrate", "Back"};
  bool optionSelected = false;
  int index = 0;
  lcd.print(settingsMenuItems[index]);

  while (!optionSelected)
  {
    int index = (encoder.read() / 4) % 3; // calculate the index

    if (index < 0) // if the index is negative
    {
      index += 3; // wrap around to the last item
    }

    if (index != prevIndex)
    {
      lcd.clear();
      lcd.print(settingsMenuItems[index]);
      prevIndex = index;
    }

    if (digitalRead(ROT_SW) == LOW) // if rotary button pressed:
    {
      optionSelected = true;
      delay(100);
      switch (index)
      {
      case 0: // set dose
        break;
      case 1: // calibrate
        calibrate();
      case 2: // back
        mainMenu();
      }
    }
  }
}

void calibrate()
{
  bool calibrating = true;
  bool dispensing = false;
  lcd.clear();
  lcd.print("Dispense 100ml");

  unsigned long start_time;

  while (calibrating)
  {
    if (digitalRead(TOGGLE) == LOW || digitalRead(TRIGGER) == LOW)
    {
      if (!dispensing)
      {
        start_time = millis(); // Start the timer
        runPump();
        dispensing = true;
      }
      else
      {
        runPump();
      }
    }
    else
    {
      if (dispensing)
      {
        calibrating = false;
      }
    }
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Calibration Complete");

  unsigned long elapsed_time = millis() - start_time;         // Calculate the elapsed time in milliseconds
  float num_revs = getRPM() * (elapsed_time / 1000.0) / 60.0; // Calculate the number of revolutions
  float flow_rate = 100 / (elapsed_time / 60000.0);           // 100 ml divided by the elapsed time in minutes (gives ml/min)

  ml_per_rev = 100 / num_revs; // Calculate the volume dispensed per revolution, in ml/rev

  lcd.setCursor(0, 1);
  lcd.print(flow_rate);
  lcd.print(" ml/min");

  delay(3000);
}

// run the pump in manual mode, triggered by the toggle switch
//  pump frequency is set by frequency variable
void runPump()
{
  if (pumpRunning == false)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Pump Running...");
    lcd.setCursor(0, 1);
    lcd.print(getRPM());
    lcd.print(" RPM");
    pumpRunning = true;
  }

  digitalWrite(STEPPER_STEP, HIGH);
  digitalWrite(LED_BUILTIN, HIGH);
  delayMicroseconds(step_delay_microseconds);
  digitalWrite(STEPPER_STEP, LOW);
  digitalWrite(LED_BUILTIN, LOW);
  delayMicroseconds(step_delay_microseconds);
}

void updateFrequency()
{
  int CLK_state = digitalRead(ROT_CLK);
  int DT_state = digitalRead(ROT_DT);
  if (CLK_state != last_CLK_state)
  {
    if (DT_state != CLK_state)
    {
      frequency += 10;
    }
    else
    {
      frequency -= 10;
    }

    period = 1.0 / frequency;
    step_delay_microseconds = (period / 2) * 1000000;
    last_CLK_state = CLK_state;
  }
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
    lcd.print(digitalRead(TOGGLE));

    lcd.setCursor(10, 0);
    lcd.print("Tr:");
    lcd.print(digitalRead(TRIGGER));

    lcd.setCursor(0, 1);
    lcd.print("RC:");                // rotary code (raw binary)
    lcd.print(digitalRead(ROT_CLK)); // clk
    lcd.print(digitalRead(ROT_DT));  // dt
    lcd.print(digitalRead(ROT_SW));  // sw

    lcd.setCursor(10, 1);
    lcd.print("RP:"); // rotary position (decoded)
    lcd.print(encoder.read());

    delay(100);

    if (digitalRead(ROT_SW) == LOW)
    {
      break;
    }
  }
}

int getRPM()
{
  float rpmFloat = (frequency * 60.0) / 200.0;
  return (int)rpmFloat;
}

void writeIntToEEPROM(int value, int address)
{
  uint8_t highByte = (value >> 8) & 0xFF; // extract the high byte
  uint8_t lowByte = value & 0xFF;         // extract the low byte
  EEPROM.write(address, highByte);        // write the high byte to EEPROM
  EEPROM.write(address + 1, lowByte);     // write the low byte to EEPROM
}

int readIntFromEEPROM(int address)
{
  uint8_t highByte = EEPROM.read(address);    // read the high byte from EEPROM
  uint8_t lowByte = EEPROM.read(address + 1); // read the low byte from EEPROM
  int value = (highByte << 8) | lowByte;      // combine the high and low bytes into a 16-bit int
  return value;
}

float getFlowRate()
{
  return ml_per_rev * getRPM() / 60;
}

void drawMushrooms()
{
  lcd.setCursor(0, 1);
  lcd.write((byte)0);
  delay(100);
  lcd.write((byte)0);
  delay(100);
  lcd.write((byte)0);
  delay(100);
  lcd.write((byte)0);
  delay(100);
  lcd.write((byte)0);
  delay(100);
  lcd.write((byte)0);
  delay(100);
  lcd.write((byte)0);
  delay(100);
  lcd.write((byte)0);
  delay(100);
  lcd.write((byte)0);
  delay(100);
  lcd.write((byte)0);
  delay(100);
  lcd.write((byte)0);
  delay(100);
  lcd.write((byte)0);
  delay(100);
  lcd.write((byte)0);
  delay(100);
  lcd.write((byte)0);
  delay(100);
  lcd.write((byte)0);
  delay(100);
  lcd.write((byte)0);
}