# REMIS
Source code for the Remarkably Efficient Mass Inoculation System (REMIS) by Fungaia, a peristaltic pump/injector system designed for large-scale mushroom cultivation via liquid culture.

## Installation
1. Clone this repo
2. Install the dependencies
3. Upload the sketch to the Arduino nano board, either using the Arduino IDE or the command line interface (CLI)

This code is designed to run on an Arduino Nano board, connected to the following inputs:
- **Rotary encoder** for navigating the device's UI
- **Toggle switch** for using the pump's manual mode 
- **Button** which is used as a trigger for the inocculation gun 

And the following outputs:
- **LCD (I2C) display** for displaying the UI
- **Stepper Motor Driver** which takes the signals generated by the Arduino board and sends them to the pump
