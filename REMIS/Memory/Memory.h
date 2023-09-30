#ifndef MEMORY_H
#define MEMORY_H

#include <EEPROM.h>
#include "Arduino.h"

int getRPM();

float getFlowRate();

private void writeIntToEEPROM(int value, int address);

private int readIntFromEEPROM(int address);

#endif