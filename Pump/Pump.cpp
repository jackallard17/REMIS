#include <LiquidCrystal_I2C.h>

#include "Pump.h"
#include "../Pins.h"

#define STEPS 200

Stepper stepper(STEPS, STEPPER_DIR, STEPPER_STEP);

Pump::Pump(LiquidCrystal_I2C lcd)
{
  this->lcd = lcd;
}

void Pump::setup

void Pump::runPump()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Pump Running...");
  lcd.setCursor(0, 1);
  lcd.print(getRPM());
  lcd.print(" RPM");

  digitalWrite(STEPPER_STEP, HIGH);
  digitalWrite(LED_BUILTIN, HIGH);
  delayMicroseconds(step_delay_microseconds);
  digitalWrite(STEPPER_STEP, LOW);
  digitalWrite(LED_BUILTIN, LOW);
  delayMicroseconds(step_delay_microseconds);
}

void Pump::checkInputsAndRunPump()
{
  if (digitalRead(TRIGGER) == LOW)
  {
    runPump();
  }
  else
  {
    if (pumpRunning)
    {
      lcd.clear();
      menuRedrawNeeded = true;
    }

    pumpRunning = false;
  }
}

void Pump::updateFrequency()
{
  if (pumpRunning)
  {
    int CLK_state = digitalRead(ROT_CLK);
    int DT_state = digitalRead(ROT_DT);
    if (CLK_state != last_CLK_state)
    {
      if (DT_state != CLK_state)
      {
        frequency += 10;
      }
      else
      {
        frequency -= 10;
      }

      period = 1.0 / frequency;
      step_delay_microseconds = (period / 2) * 1000000;
      last_CLK_state = CLK_state;

      frequencyUpdated = true;
    }
  }
}
