#include "UI.h"

LiquidCrystal_I2C lcd(0x27, 2, 16);
Encoder encoder(2, 3);

uint8_t mushroom[8] = {
    0b00000,
    0b00100,
    0b01110,
    0b11111,
    0b00100,
    0b00100,
    0b00100,
    0b00100};

byte batteryIcon[8] = {
    B00100,
    B01110,
    B01110,
    B01110,
    B01110,
    B01110,
    B01110,
    B00000};


UI::UI()
{
    lcd.init();
    lcd.backlight();
    lcd.createChar(0, mushroom);
    lcd.createChar(1, batteryIcon);

    // menuItems[] = {"Settings"}; // debug/diagnostics mode is hidden
    mainIndex = 0;
    prevIndex;
    submenuVisited = false;
    pumpRunning = false;
    frequencyUpdated = false;
    menuRedrawNeeded = false;

    drawMushrooms();
    dashboard();
}

void UI::drawMushrooms()
{
    lcd.setCursor(0, 1);
    for (int i = 0; i < 16; i++)
    {
        lcd.write((byte)0);
        delay(100);
    }
}

void UI::dashboard()
{
    if (submenuVisited)
    {
        lcd.clear();
        submenuVisited = false;
    }

    // print the current injector mode in the top left corner and the battery percentage in the top right corner
    lcd.setCursor(0, 0);
    lcd.print("REMIS");
    lcd.print("      ");
    lcd.write((byte)1);
    lcd.print("100");
    lcd.print("%");

    lcd.setCursor(0, 1);

    // if the rotary encoder is turned, enter settings menu
    if (encoder.read() != prevIndex)
    {
        settingsMenu();
    }
}


void UI::injectorModeMenu()
{
    String injectorModeMenuItems[] = {"Continuous", "Dose Mode", "Back"};
    bool optionSelected = false;
    int index = 0;

    delay(100);

    while (!optionSelected)
    {
        // checkInputsAndRunPump();

        int index = (encoder.read() / 4) % 3; // calculate the index

        if (index < 0) // if the index is negative
        {
            index += 3; // wrap around to the last item
        }

        if (index != prevIndex || menuRedrawNeeded)
        {
            lcd.clear();
            menuRedrawNeeded = false;

            // Display menu items with '>' to mark the selected item
            if (index == 0)
            {
                lcd.print("> ");
                lcd.print(injectorModeMenuItems[0]);
                lcd.setCursor(0, 1);
                lcd.print("  ");
                lcd.print(injectorModeMenuItems[1]);
            }
            else if (index == 1)
            {
                lcd.print("  ");
                lcd.print(injectorModeMenuItems[0]);
                lcd.setCursor(0, 1);
                lcd.print("> ");
                lcd.print(injectorModeMenuItems[1]);
            }
            else if (index == 2)
            {
                lcd.print("  ");
                lcd.print(injectorModeMenuItems[1]);
                lcd.setCursor(0, 1);
                lcd.print("> ");
                lcd.print(injectorModeMenuItems[2]);
            }

            prevIndex = index;
        }

        if (digitalRead(ROT_SW) == LOW) // if rotary button pressed:
        {
            optionSelected = true;
            delay(100);
            if (index == 0)
            {
                // injectorMode = 0;
                EEPROM.write(2, 0);
            }
            else if (index == 1)
            {
                // injectorMode = 1;
                EEPROM.write(2, 1);
            }
            else if (index == 2)
            {
                settingsMenu();
            }
        }
    }
}

void UI::flowRateMenu()
{
}

void UI::settingsMenu()
{
    String settingsMenuItems[] = {"Injector Mode", "Flow Rate", "Set Dose", "Calibrate", "Back"};
    bool optionSelected = false;
    int index = 0;

    delay(100);

    while (!optionSelected)
    {
        // checkInputsAndRunPump();

        int index = (encoder.read() / 4) % 4; // calculate the index

        if (index < 0) // if the index is negative
        {
            index += 4; // wrap around to the last item
        }

        if (index != prevIndex || menuRedrawNeeded)
        {
            lcd.clear();
            menuRedrawNeeded = false;

            // Display menu items with '>' to mark the selected item
            if (index == 0)
            {
                lcd.print("> ");
                lcd.print(settingsMenuItems[0]);
                lcd.setCursor(0, 1);
                lcd.print("  ");
                lcd.print(settingsMenuItems[1]);
            }
            else if (index == 1)
            {
                lcd.print("  ");
                lcd.print(settingsMenuItems[1]);
                lcd.setCursor(0, 1);
                lcd.print("> ");
                lcd.print(settingsMenuItems[2]);
            }
            else if (index == 2)
            {
                lcd.print("  ");
                lcd.print(settingsMenuItems[2]);
                lcd.setCursor(0, 1);
                lcd.print("> ");
                lcd.print(settingsMenuItems[3]);
            }
            else if (index == 3)
            {
                lcd.print("  ");
                lcd.print(settingsMenuItems[3]);
                lcd.setCursor(0, 1);
                lcd.print("> ");
                lcd.print(settingsMenuItems[4]);
            }
            else if (index == 4)
            {
                lcd.print("  ");
                lcd.print(settingsMenuItems[4]);
                lcd.setCursor(0, 1);
                lcd.print("> ");
                lcd.print(settingsMenuItems[4]);
            }

            prevIndex = index;
        }

        if (digitalRead(ROT_SW) == LOW) // if rotary button pressed:
        {
            optionSelected = true;
            delay(100);
            lcd.clear();
            switch (index)
            {
            case 0:
                injectorModeMenu();
                break;
            case 1:
                flowRateMenu();
                break;
            case 2:
                break;
            case 3:
                dashboard();
                break;
            case 4:
                // calibrate();
                break;
            }
        }
    }
}

void UI::debugMode()
{
    // print debug mode on screen
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Debug Mode");

    delay(1000);
    lcd.clear();

    while (true)
    {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("T:");
        lcd.print(digitalRead(TOGGLE));

        lcd.setCursor(10, 0);
        lcd.print("Tr:");
        lcd.print(digitalRead(TRIGGER));

        lcd.setCursor(0, 1);
        lcd.print("RC:");                // rotary code (raw binary)
        lcd.print(digitalRead(ROT_CLK)); // clk
        lcd.print(digitalRead(ROT_DT));  // dt
        lcd.print(digitalRead(ROT_SW));  // sw

        lcd.setCursor(10, 1);
        lcd.print("RP:"); // rotary position (decoded)
        lcd.print(encoder.read());

        delay(100);

        if (digitalRead(ROT_SW) == LOW)
        {
            break;
        }
    }
}
