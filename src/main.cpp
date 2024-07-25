/*
 Basic Menu

 https://lcdmenu.forntoh.dev/examples/basic

*/

#include <LcdMenu.h>
#include <utils/commandProccesors.h>
#include <Encoder.h>

Encoder encoder(2, 3);

// rotary encoder bindings
#define ROT_CLK 2 // rotary encoder clock
#define ROT_DT 3  // rotary encoder direction
#define ROT_SW 4  // rotary encoder switch (press in)

// pump input bindings
#define TOGGLE 8
#define TRIGGER 9

// stepper motor bindings
#define STEPPER_STEP 5
#define STEPPER_DIR 9

// lcd bindings
#define LCD_SDA 22
#define LCD_SDL 21

#define STEPS 200

// menu bindings
#define LCD_ROWS 2
#define LCD_COLS 16

// Configure keyboard keys (ASCII)
#define UP 56     // NUMPAD 8
#define DOWN 50   // NUMPAD 2
#define ENTER 53  // NUMPAD 5
#define BACK 55   // NUMPAD 7

// Initialize the main menu items
MAIN_MENU(
    ITEM_BASIC("Injector Mode"),
    ITEM_BASIC("Calibrate"),
    ITEM_BASIC("Pump Speed")
);
// Construct the LcdMenu
LcdMenu menu(LCD_ROWS, LCD_COLS);

void setup() {
    Serial.begin(9600);
    // Initialize LcdMenu with the menu items
    menu.setupLcdWithMenu(0x27, mainMenu);
}

void loop() {
    if (!Serial.available()) return;
    char command = Serial.read();

    processMenuCommand(menu, command, UP, DOWN, ENTER, BACK);
}