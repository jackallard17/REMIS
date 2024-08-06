#include <LcdMenu.h>

class REMISMenu
{
    private:
        LcdMenu menu;

    public:
        REMISMenu();
        void pollRotaryInput();
        void debugMode();
        void checkDebugInput();
};