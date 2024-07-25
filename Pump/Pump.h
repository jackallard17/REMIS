#ifndef PUMP_H
#define PUMP_H

#include <Stepper.h>

using namespace std;

class Pump
{
private:
    // persistent properties, stored between power cycles
    float frequency;  // frequency of the pump in Hz
    int injectorMode; // 0 = toggle, 1 = continuous, 2 = dose mode
    int dose;         // dose amount in mL

    float period;
    long step_delay_microseconds;

    float ml_per_rev = 200;

    volatile int last_CLK_state = LOW;
    volatile int last_DT_state = LOW;

    void updateFrequency();
    void runPump();

public:
    void setup();

    void loop();

    void calibrate();

    void checkInputsAndRunPump();
};

#endif