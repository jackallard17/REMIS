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

#define STEPPER_DIR 5
#define STEPPER_STEP 6

#define LCD_SDA 22
#define LCD_SDL 21
#define TOGGLE 8
#define TRIGGER 9

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
int menuItemIndex = 0;
int prevIndex;

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

  // outputs
  pinMode(STEPPER_DIR, OUTPUT);
  pinMode(STEPPER_STEP, OUTPUT);

  Serial.begin(9600);

  lcd.clear();
  lcd.print(menuItems[menuItemIndex]);
  prevIndex = (encoder.read() / 4) % 3; // store inital state of rotary encoder
}

void loop()
{
  int menuItemIndex = (encoder.read() / 4) % 3; // calculate the index

  if (menuItemIndex < 0) // if the index is negative
  {
    menuItemIndex += 3; // wrap around to the last item
  }

  if (digitalRead(ROT_SW) == LOW) // if rotary button pressed:
  {
    lcd.clear();
    // enter the submenu for the current menu item
    switch (menuItemIndex)
    {
    case 0:
      injectorModeMenu();
    case 1:
      flowRateMenu();
    case 2:
      calibrate();
    case 3:
      settingsMenu();
    }
  }

  if (menuItemIndex != prevIndex)
  {
    lcd.clear();
    lcd.print(menuItems[menuItemIndex]);
    prevIndex = menuItemIndex;
  }
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

void injectorModeMenu()
{
  String menuItems[] = {"Toggle", "Continuous", "Dose Mode"};
  bool optionSelected = false;
  lcd.print(menuItems[menuItemIndex]);

  while (!optionSelected)
  {
    int menuItemIndex = (encoder.read() / 4) % 3; // calculate the index

    if (menuItemIndex < 0) // if the index is negative
    {
      menuItemIndex += 3; // wrap around to the last item
    }

    if (digitalRead(ROT_SW) == LOW) // if rotary button pressed:
    {
      optionSelected = true;
      // enter the submenu for the current menu item
      switch (menuItemIndex)
      {
      case 0:
        // set injectorMode variable to 0 (toggle mode)
        break;
        // set injectorMode variable to 1 (continuous mode)
        break;
      case 2:
        // set injectorMode variable to 2 (volumetric mode)
        break;
      }
    }

    if (menuItemIndex != prevIndex)
    {
      lcd.clear();
      lcd.print(menuItems[menuItemIndex]);
      prevIndex = menuItemIndex;
    }
  }
}

void flowRateMenu()
{
}

void settingsMenu()
{
}

void calibrate()
{
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