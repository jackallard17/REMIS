
#include "Memory.h"

class Memory
{

    int getRPM()
    {
        float rpmFloat = (frequency * 60.0) / 200.0;
        return (int)rpmFloat;
    }

    float getFlowRate()
    {
        return ml_per_rev * getRPM() / 60;
    }

private
    void writeIntToEEPROM(int value, int address)
    {
        uint8_t highByte = (value >> 8) & 0xFF; // extract the high byte
        uint8_t lowByte = value & 0xFF;         // extract the low byte
        EEPROM.write(address, highByte);        // write the high byte to EEPROM
        EEPROM.write(address + 1, lowByte);     // write the low byte to EEPROM
    }

private
    int readIntFromEEPROM(int address)
    {
        uint8_t highByte = EEPROM.read(address);    // read the high byte from EEPROM
        uint8_t lowByte = EEPROM.read(address + 1); // read the low byte from EEPROM
        int value = (highByte << 8) | lowByte;      // combine the high and low bytes into a 16-bit int
        return value;
    }
};