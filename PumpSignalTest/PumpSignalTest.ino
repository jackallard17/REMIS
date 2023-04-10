#include <Encoder.h>
#include <LiquidCrystal_I2C.h>
#include <stdint.h>
#include <Wire.h>


void setup() {
  // put your setup code here, to run once:
  pinMode(2, OUTPUT);

}

void loop() {
    digitalWrite(2, HIGH);

    delayMicroseconds(400);

    digitalWrite(2, LOW);

    delayMicroseconds(400);


}
