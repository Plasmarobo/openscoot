# openscoot
An open source CAN based escooter controller

# Primary design
Focus on throttle response, ebraking performance, and power management.

## Throttle smoothing and braking
The throttle response should be smoothed, with different coefficients for increase and decrease.
Braking performance needs to be maintained, and should override throttle smoothing.

## Ebraking & Speed Control
Handled by VESC

## GPS Tracking and positioning

## Anti-theft/Authentication

## Power management and reporting
Handled by VESC

## Connection Managment

### Wifi

### LoRA

### Cell
NOT IMPLEMENTED

## Display

## Power profiles

# Known bugs/limitations

# HardwareStack

## ESP32-S2 Reverse TFT
Available pins:

Available interfaces:
SPI - CAN, LORA
I2C - SSD, NFC
Serial1 - GPS
### TFT
Uses `SPI`
Uses Pin - CS (D7)
Uses Pin - Backlite (45 internal / )
Uses Pin - Pwr (21 internal / )

## OLED
Uses `I2C`
Addr

## GPS Featherwing
Uses `Serial1`

## CAN Featherwing
Uses `SPI`
Uses Pin D5 - CS
Uses Pin D6 - INT

## LORA Featherwing
Uses `SPI`
Configure Pin C (D9)- CS
Configure Pin D (D10) - INT
Configure Pin E (D11) - RST

## Seven segment featherwing
Uses `I2C`
Addr

## NFC eeprom
Uses `I2C`
Configure Pin 12 - INT
Addr

# Analog Throttle
A0 - GPIO10 - 
