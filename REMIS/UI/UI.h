#ifndef UI_H
#define UI_H

#include "LiquidCrystal_I2C.h"

class UI
{

    string menuItems[];
    int mainIndex;
    int prevIndex;
    volatile bool submenuVisited;
    volatile bool pumpRunning;
    volatile bool frequencyUpdated;
    volatile bool menuRedrawNeeded;

    UI();

    void drawMushrooms();

    void dashboard();

    void injectorModeMenu();

    void settingsMenu();

    void debugMode();
}
#endif
