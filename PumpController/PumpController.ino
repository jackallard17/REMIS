#include <LiquidCrystal.h>
#include <stdint.h>
#include <Wire.h>
using namespace std;

//Digital pins D2 - D7
int ROT_CLK = 5;
int ROT_DT = 6;
int ROT_SW = 7;
int STEPPER_DIR = 8;
int STEPPER_STEP = 9;

//Analog pins A4 - A7
int LCD_SDA = 23;
int LCD_SDL = 24;
int TRIGGER = 25;
int TOGGLE = 26;


void setup() {

}

void loop () {

}

uint8_t mushroom[8] = {
    0b00000,
    0b00100,
    0b01110,
    0b11111,
    0b00100,
    0b00100,
    0b00100,
    0b00100};
