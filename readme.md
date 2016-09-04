#README
using a philips videopac g7000 keypad on a cheap arduinio uno with CH340 controller as a usb keyboard

use Arduino 1.05r2 or the sketch won't compile
build usb schematic like on 
http://www.practicalarduino.com/projects/virtual-usb-keyboard
code based on:
http://www.practicalarduino.com/projects/virtual-usb-keyboard
http://blog.petrockblock.com/2012/05/19/usb-keyboard-with-arduino-and-v-usb-library-an-example/
other references:
https://englishjavadrinker.blogspot.co.at/2013/10/the-caffeine-button.html
if(soldering directly to usb cable, swap cables 2, 3 (DIO2 connected to D-, DIO5, DIO4 connected to D+)
only use 250mW or 500mW Zener Diodes, not 500mW.
download VirtualUsb library from https://code.google.com/archive/p/vusb-for-arduino/
extract UsbKeyboard to Arduino libraries folder
you can modify usbconfig.h to include your domain as vendor and a device name
look for:
```
#define USB_CFG_VENDOR_NAME
#define USB_CFG_VENDOR_NAME_LEN
#define USB_CFG_DEVICE_NAME
#define USB_CFG_DEVICE_NAME_LEN
```
TIMER0 is deactivated. therefor millis(), delay(), etc. do not work
Serial() may not work because of TIMER0. If Serial is used, PINS 0,1 are reserved for Serial()
used Keypad is a 8x6 keypad from a Philips Videopac G7000 with a dedicated reset button, which is used as shift key or to reset a game in Emulationstation.