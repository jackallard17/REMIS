#include <LcdMenu.h>
#include <utils/commandProccesors.h>
#include <Encoder.h>
#include <ezButton.h>

#include <Pins.h>
#include <PumpController/PumpController.h>

Encoder encoder(2, 3);

LiquidCrystal_I2C lcd(0x27, 2, 16);

ezButton toggle(TOGGLE);

MAIN_MENU(
    ITEM_BASIC("Injector Mode"),
    ITEM_BASIC("Calibrate"),
    ITEM_BASIC("Pump Speed")
);

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

    menu.setupLcdWithMenu(0x27, mainMenu);
}

void loop() {
    checkDebugInput();

    if (!Serial.available()) return;
    char command = Serial.read();

    processMenuCommand(menu, command, UP, DOWN, ENTER, BACK);
}