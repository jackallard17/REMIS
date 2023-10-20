#include "Pump.h"
#include "../Pins.h"

LiquidCrystal_I2C lcd(0x27, 2, 16);
Encoder encoder(2, 3);

#define STEPS 200

Stepper stepper(STEPS, STEPPER_DIR, STEPPER_STEP);

void Pump::setup()
{
  Serial.begin(9600);

  // LCD init and startup message
  lcd.init();
  lcd.backlight();
  lcd.createChar(0, mushroom);
  lcd.createChar(1, batteryIcon);
  lcd.setCursor(0, 0);
  lcd.print("REMIS       v0.0");
  lcd.setCursor(0, 12);
  drawMushrooms();

  // on first startup, initialize the persistent properties to default values
  if (EEPROM.read(0) == 255)
  {
    writeIntToEEPROM(1100, 0);
    EEPROM.write(2, 0);
    EEPROM.write(3, 10);
  }

  frequency = readIntFromEEPROM(0);
  frequency = frequency;
  injectorMode = EEPROM.read(2);
  dose = EEPROM.read(3);

  period = 1.0 / frequency; // period of the pump in seconds
  step_delay_microseconds = (period / 2) * 1000000;

  // inputs
  pinMode(TOGGLE, INPUT_PULLUP);
  pinMode(TRIGGER, INPUT_PULLUP);
  pinMode(ROT_CLK, INPUT_PULLUP);
  pinMode(ROT_DT, INPUT_PULLUP);
  pinMode(ROT_SW, INPUT);

  attachInterrupt(digitalPinToInterrupt(ROT_CLK), updateFrequency, CHANGE);

  pinMode(LED_BUILTIN, OUTPUT); // Set onboard LED pin as output

  // outputs
  pinMode(STEPPER_DIR, OUTPUT);
  pinMode(STEPPER_STEP, OUTPUT);

  lcd.clear();
  lcd.print(menuItems[mainIndex]);
  prevIndex = (encoder.read() / 4) % 3; // store inital state of rotary encoder

  digitalWrite(10, HIGH);
}

void Pump::loop()
{
  // manual mode is triggered by the toggle switch
  if (digitalRead(TOGGLE) == LOW)
  {
    runPump();
  }
  else if (digitalRead(TRIGGER) == LOW && injectorMode == 0)
  {
    runPump();
  }
  else if (digitalRead(TRIGGER) == LOW && injectorMode == 1)
  {
    int steps = 0;
    int doseSteps = dose * ml_per_rev;

    while (steps < doseSteps)
    {
      runPump();
      steps++;
    }
  }
  else
  {
    digitalWrite(STEPPER_STEP, LOW);
    digitalWrite(LED_BUILTIN, LOW);
    if (pumpRunning == true)
    {
      lcd.clear();
      lcd.print(menuItems[mainIndex]);
      pumpRunning = false;
    }
  }

  if (!pumpRunning)
  {
    checkInputsAndRunPump();
    dashboard();
  }

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

void Pump::calibrate()
{
  bool calibrating = true;
  bool dispensing = false;
  lcd.clear();
  lcd.print("Dispense 100ml");

  unsigned long start_time;

  while (calibrating)
  {
    if (digitalRead(TOGGLE) == LOW || digitalRead(TRIGGER) == LOW)
    {
      if (!dispensing)
      {
        start_time = millis(); // Start the timer
        runPump();
        dispensing = true;
      }
      else
      {
        runPump();
      }
    }
    else
    {
      if (dispensing)
      {
        calibrating = false;
      }
    }
  }

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Calibration Complete");

  unsigned long elapsed_time = millis() - start_time;         // Calculate the elapsed time in milliseconds
  float num_revs = getRPM() * (elapsed_time / 1000.0) / 60.0; // Calculate the number of revolutions
  float flow_rate = 100 / (elapsed_time / 60000.0);           // 100 ml divided by the elapsed time in minutes (gives ml/min)

  ml_per_rev = 100 / num_revs; // Calculate the volume dispensed per revolution, in ml/rev

  lcd.setCursor(0, 1);
  lcd.print(flow_rate);
  lcd.print(" ml/min");

  delay(3000);
}

// run the pump in manual mode, triggered by the toggle switch
//  pump frequency is set by frequency variable
void Pump::runPump()
{
  if (pumpRunning == false)
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Pump Running...");
    lcd.setCursor(0, 1);
    lcd.print(getRPM());
    lcd.print(" RPM");
    pumpRunning = true;
  }

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

