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
#define TOGGLE 7
#define TRIGGER 8

// stepper motor bindings
#define STEPPER_STEP 5
#define STEPPER_DIR 6

// lcd bindings
#define LCD_SDA 22
#define LCD_SDL 21

LiquidCrystal_I2C lcd(0x27, 2, 16);

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

void debugMode()
{
  while(true)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("T:");
    lcd.print(digitalRead(TOGGLE));
    lcd.setCursor(9, 0);
    lcd.print("Tr:");
    lcd.print(digitalRead(TRIGGER));

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

void checkDebugInput()
{
   // if the rotary encoder button is held for 5 seconds, enter debug mode
  if (digitalRead(ROT_SW) == LOW)
  {
    int i = 0;
    while (i < 500)
    {
      if (digitalRead(ROT_SW) == HIGH)
      {
        break;
      }
      delay(10);
      i++;
    }
    if (i == 500)
    {
      debugMode();
    }
  }
}

void setup() {
    Serial.begin(9600);
    lcd.init();
    lcd.backlight();

    // Initialize LcdMenu with the menu items
    menu.setupLcdWithMenu(0x27, mainMenu);
}

void loop() {
    // if rot_clk is held for 5 seconds, enter debug mode
    checkDebugInput();

    if (!Serial.available()) return;
    char command = Serial.read();

    processMenuCommand(menu, command, UP, DOWN, ENTER, BACK);
}