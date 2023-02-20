#include <LiquidCrystal.h>
#include <stdint.h>
#include <Wire.h>
using namespace std;

int LCD_SDA = 6;


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
