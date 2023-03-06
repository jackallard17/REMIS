#include <Encoder.h>
#include <LiquidCrystal_I2C.h>
#include <stdint.h>
#include <Wire.h>

using namespace std;

LiquidCrystal_I2C lcd(0x27, 2, 16);
Encoder encoder(5,6);

long prevPosition;

// Digital pins D2 - D7
#define ROT_CLK 5 // rotary encoder clock
#define ROT_DT 6 // rotary encoder direction
#define ROT_SW 7 // rotary encoder switch (press in)
#define STEPPER_DIR 8
#define STEPPER_STEP 9

// Analog pins A4 - A7. The LCD pins, (23 and 24) will be set to digital mode.
// #define LCD_SDA 23
// #define LCD_SDL 24
#define LCD_SDA 22
#define LCD_SDL 21
#define TRIGGER 20
#define TOGGLE 10

//threshold voltages to parse analog signal
int triggerThreshold;

uint8_t mushroom[8] = {
    0b00000,
    0b00100,
    0b01110,
    0b11111,
    0b00100,
    0b00100,
    0b00100,
    0b00100};

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
  pinMode(TRIGGER, INPUT);
  pinMode(TOGGLE, INPUT);
  pinMode(ROT_CLK, INPUT);
  pinMode(ROT_DT, INPUT);
  pinMode(ROT_SW, INPUT);

  // outputs
  pinMode(STEPPER_DIR, OUTPUT);
  pinMode(STEPPER_STEP, OUTPUT);

  Serial.begin(9600);

  lcd.clear();
}


void loop()
{
  debugMode();

  // long newPosition = encoder.read();

  // if (newPosition != prevPosition)
  // {
  //   prevPosition= newPosition;
  //   Serial.println(newPosition);
  //   lcd.print(newPosition);
  // }

  // if (analogRead(TRIGGER) > 1000) 
  // {
  //   lcd.print("trigger pressed");
  // } else 
  // {
  //   lcd.clear();
  // }
}

void debugMode()
{
  while(true)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("T:");
    // lcd.print(digitalRead(TOGGLE));
    lcd.print(digitalRead(10));
    lcd.setCursor(9, 0);
    lcd.print("Tr:");
    lcd.print(analogRead(TRIGGER));

    lcd.setCursor(0, 1);
    lcd.print("RC:");
    lcd.print(digitalRead(ROT_CLK));
    lcd.print(digitalRead(ROT_DT));
    lcd.print(digitalRead(ROT_SW));
    lcd.setCursor(9, 1);
    lcd.print("RP:");
    lcd.print(encoder.read());

    delay(100);
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