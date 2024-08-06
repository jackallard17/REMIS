#include <LcdMenu.h>
#include <utils/commandProccesors.h>
#include <Encoder.h>
#include <ezButton.h>

#include <Pins.h>
#include <Menu/Menu.h>

REMISMenu menu;

ezButton toggle(TOGGLE);
ezButton trigger(TRIGGER);

void setup() {
    Serial.begin(9600);
}

void loop() {
    menu.checkDebugInput();
    menu.pollRotaryInput();
}