#include <Stepper.h>

#include <Encoder.h>
#include <LiquidCrystal_I2C.h>
#include <stdint.h>
#include <Wire.h>

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

Stepper stepper(STEPS, STEPPER_DIR, STEPPER_STEP);

// pump properties
int frequency = 1000;                          // frequency of the pump in Hz
float period = 1.0 / frequency;                    // period of the pump in seconds
long step_delay_microseconds = (period / 2) * 1000000;

void setup()
{
  // LCD init and startup message
  lcd.init();
  lcd.backlight();
  lcd.createChar(0, mushroom);
  lcd.setCursor(0, 0);
  lcd.print("REMIS       v0.0");
  lcd.setCursor(0, 12);
  drawMushrooms();

  // inputs
  pinMode(TOGGLE, INPUT_PULLUP);
  pinMode(TRIGGER, INPUT_PULLUP);
  pinMode(ROT_CLK, INPUT);
  pinMode(ROT_DT, INPUT);
  pinMode(ROT_SW, INPUT);
  pinMode(LED_BUILTIN, OUTPUT); // Set onboard LED pin as output

  // outputs
  pinMode(STEPPER_DIR, OUTPUT);
  pinMode(STEPPER_STEP, OUTPUT);

  Serial.begin(9600);

  lcd.clear();
  lcd.print(menuItems[mainIndex]);
  prevIndex = (encoder.read() / 4) % 3; // store inital state of rotary encoder

  digitalWrite(10, HIGH);
}

void loop()
{
  //manual mode is triggered by the toggle switch
  int switchState = digitalRead(TOGGLE);
  if (switchState == LOW) {
    runPump();
  } else {
    digitalWrite(STEPPER_STEP, LOW);
    digitalWrite(LED_BUILTIN, LOW);
  }

  if (submenuVisited)
  {
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
    delay(100);
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
  String injectorModeMenuItems[] = {"Toggle", "Continuous", "Dose Mode"};
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
      switch (index)
      {
      case 0:
        break;
      case 1:
        break;
      case 2:
        break;
      }
    }
  }
}

void flowRateMenu()
{
}

void settingsMenu()
{
  String settingsMenuItems[] = {"Set Dose", "Calibrate"};
  bool optionSelected = false;
  int index = 0;
  lcd.print(settingsMenuItems[index]);

  while (!optionSelected)
  {
    int index = (encoder.read() / 4) % 2; // calculate the index

    if (index < 0) // if the index is negative
    {
      index += 2; // wrap around to the last item
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
      case 0:
        break;
      case 1:
        break;
      }
    }
  }
}

void calibrate()
{
}

// run the pump in manual mode, triggered by the toggle switch
//  pump frequency is set by frequency variable
void runPump()
{
  digitalWrite(STEPPER_STEP, HIGH);
  digitalWrite(LED_BUILTIN, HIGH);
  delayMicroseconds(step_delay_microseconds);
  digitalWrite(STEPPER_STEP, LOW);
  digitalWrite(LED_BUILTIN, LOW);
  delayMicroseconds(step_delay_microseconds);
}

void debugMode()
{
  while (ROT_SW != LOW)
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
  }
}

void injectionAnimation()
{
  for (int i = 0; i < 6; i++) {
    lcd.setCursor(i, 0);
    lcd.write(byte(0));
    delay(500);
    lcd.setCursor(i, 0);
    lcd.print(" ");
  }
  for (int i = 5; i >= 0; i--) {
    lcd.setCursor(i, 1);
    lcd.write(byte(0));
    delay(500);
    lcd.setCursor(i, 1);
    lcd.print(" ");
  }
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