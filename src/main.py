from machine import Pin
import time

led = Pin("LED", Pin.OUT)

trigger = Pin(1, Pin.IN, Pin.PULL_DOWN)
toggle = Pin(15, Pin.IN, Pin.PULL_DOWN)

def main():
    print('program started')
    led.value(0)

    while True:
         if toggle.value() == 1:
             print('toggle on')
             led.value(1)
         if trigger.value() == 1:
             print('trigger pressed')
             led.value(1)
    

main()