#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>   //F. Malpartida LCD's driver
#include <menu.h>                //menu macros and objects
#include <menuIO/lcdOut.h>       //malpartidas lcd menu output
#include <menuIO/serialIn.h>     //Serial input
#include <menuIO/encoderIn.h>    //quadrature encoder driver and fake stream
#include <menuIO/keyIn.h>        //keyboard driver and fake stream (for the encoder button)
#include <menuIO/chainStream.h>  // concatenate multiple input streams (this allows adding a button to the encoder)
#include <Stepper.h>

// globals
#define STEPPER_STEP 5
#define STEPPER_DIR 6

// Digital pins D2 - D7
#define ROT_CLK 2  // rotary encoder clock
#define ROT_DT 3   // rotary encoder direction
#define ROT_SW 4   // rotary encoder switch (press in)

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


void setup() {
}

void loop() {
  lcd.createChar(0, mushroom);
  lcd.createChar(1, batteryIcon);
  lcd.setCursor(0, 0);
  lcd.print("REMIS       v0.5");
  lcd.setCursor(0, 12);
  drawMushrooms();
}