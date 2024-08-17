#include <LcdMenu.h>
#include <utils/commandProccesors.h>
#include <Encoder.h>
#include <ezButton.h>
#include <LiquidCrystal_I2C.h>

//#include <./Pump/Pump.h>
#include <Pins.h>

int prevEncoderValue = 0;

Encoder encoder(2, 3);

LiquidCrystal_I2C lcd(0x27, 2, 16);

ezButton toggle(TOGGLE, INPUT_PULLUP);
ezButton trigger(TRIGGER, INPUT_PULLUP);

float frequency = 1100; // frequency of the pump in Hz
float period = 1.0 / frequency; // period of the pump in seconds
long step_delay_microseconds = (period / 2) * 1000000;

MAIN_MENU(
    ITEM_BASIC("Injector Mode"),
    ITEM_BASIC("Calibrate"),
    ITEM_BASIC("Pump Speed"));

LcdMenu menu(LCD_ROWS, LCD_COLS);

int8_t mushroom[8] = {
    0b00000,
    0b00100,
    0b01110,
    0b11111,
    0b00100,
    0b00100,
    0b00100,
    0b00100
};

byte batteryIcon[8] = {
  B00100,
  B01110,
  B01110,
  B01110,
  B01110,
  B01110,
  B01110,
  B00000
};

void drawMushrooms()
{
  lcd.setCursor(0, 1);
  for (int i = 0; i < 16; i++)
  {
    lcd.write((byte)0);
    delay(100);
  }
}

void debugMode()
{
  while (true)
  {
    toggle.loop();

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("T:");
    lcd.print(digitalRead(TOGGLE));
    lcd.print(toggle.getState());
    lcd.setCursor(9, 0);
    lcd.print("Tr:");
    lcd.print(digitalRead(TRIGGER));
    lcd.print(digitalRead(6));
    lcd.print(digitalRead(9));
    lcd.print(digitalRead(10));
    lcd.print(trigger.getState());

    lcd.setCursor(0, 1);
    lcd.print("RC:");
    lcd.print(digitalRead(ROT_CLK));
    // lcd.print(digitalRead(ROT_DT));
    lcd.print(digitalRead(ROT_SW));
    lcd.setCursor(9, 1);
    lcd.print("RP:");
    lcd.print(encoder.read());

    delay(100);
  }
}

void checkDebugInput()
{
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

void pollRotaryInput()
{
  if (ROT_SW == LOW)
  {
    menu.enter();
  }

  int value = encoder.read();
  Serial.println(value);

  if (value > prevEncoderValue)
  {
    menu.up();
  }
  else if (value < prevEncoderValue)
  {
    menu.down();
  }
  else
  {
    return;
  }

  prevEncoderValue = value;
}

void step()
{
  digitalWrite(STEPPER_STEP, HIGH);
  delayMicroseconds(step_delay_microseconds);
  digitalWrite(STEPPER_STEP, LOW);
  delayMicroseconds(step_delay_microseconds);
}

void runPump()
{
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Pump Running...");
  lcd.setCursor(0, 1);
  lcd.print(frequency);
  lcd.print(" RPM");

  while (toggle.isPressed())
  {
    step();
  }
}

void setup()
{
  Serial.begin(9600);

  // LCD init and startup message
  lcd.init();
  lcd.backlight();
  //lcd.createChar(0, mushroom);
  //lcd.createChar(1, batteryIcon);
  lcd.setCursor(0, 0);
  lcd.print("REMIS       v0.0");
  lcd.setCursor(0, 12);
  drawMushrooms();

  menu.setupLcdWithMenu(0x27, mainMenu);

  pinMode(TOGGLE, INPUT);
  pinMode(TRIGGER, INPUT);

  toggle.setDebounceTime(50);
  trigger.setDebounceTime(50);
}

void loop()
{
  checkDebugInput();

  if (encoder.read() != prevEncoderValue)
  {
    delay(150);
    pollRotaryInput();
  }

}