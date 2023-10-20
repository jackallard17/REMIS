#ifndef UI_H
#define UI_H

#include "LiquidCrystal_I2C.h"
#include "Encoder.h"

    class UI
    {
    public:
        UI();

        void drawMushrooms();

        void dashboard();

        void injectorModeMenu();

        void flowRateMenu();

        void settingsMenu();

        void debugMode();

    private:
        int mainIndex;
        int prevIndex;
        volatile bool submenuVisited;
        volatile bool pumpRunning;
        volatile bool frequencyUpdated;
        volatile bool menuRedrawNeeded;
    };

#endif