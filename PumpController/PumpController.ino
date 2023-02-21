#include <LiquidCrystal_I2C.h>
#include <stdint.h>
#include <Wire.h>

using namespace std;

LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);

//Digital pins D2 - D7
#define ROT_CLK 5
#define ROT_DT 6
#define ROT_SW 7
#define STEPPER_DIR 8
#define STEPPER_STEP 9

//Analog pins A4 - A7. The LCD pins, (23 and 24) will be set to digital mode.
#define LCD_SDA 23
#define LCD_SDL 24
#define TRIGGER 25
#define TOGGLE 26

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
