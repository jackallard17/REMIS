#include <LcdMenu.h>
#include <Encoder.h>
#include <ezButton.h>

#include <Pins.h>
#include <Menu/Menu.h>

REMISMenu* remisMenu = new REMISMenu();

ezButton toggle(TOGGLE);
ezButton trigger(TRIGGER);

void setup() {
    Serial.begin(9600);
}

void loop() {
    remisMenu->checkDebugInput();
    remisMenu->pollRotaryInput();
    Serial.println("Hello World");
}