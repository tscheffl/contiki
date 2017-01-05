#!/bin/bash
# This script displays the memory layout of the programm
# and flashes it to the mikrocontroller using 'avrdude'

make clean
make TARGET=avr-zigbit

avr-size mqtt-client.avr-zigbit -C --mcu=atmega1281

echo "Weiter: Taste drücken!"
read -n 1 -s

sudo avrdude -c jtag3 -P usb -p m1281 -B3 -U flash:w:mqtt-client.avr-zigbit

sudo avrdude -c jtag3 -P usb -p m1281 -B3 -U eeprom:w:mqtt-client.avr-zigbit 

