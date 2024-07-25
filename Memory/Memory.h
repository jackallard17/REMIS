#ifndef MEMORY_H
#define MEMORY_H

#include <EEPROM.h>
#include "Arduino.h"

class Memory
{
public:
    int getRPM();

    float getFlowRate();

private:
    void writeIntToEEPROM(int value, int address);

    int readIntFromEEPROM(int address);
};

#endif