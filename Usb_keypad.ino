/**
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
//#define USB_CFG_VENDOR_NAME
//#define USB_CFG_VENDOR_NAME_LEN
//#define USB_CFG_DEVICE_NAME
//#define USB_CFG_DEVICE_NAME_LEN

TIMER0 is deactivated. therefor millis(), delay(), etc. do not work
Serial() may not work because of TIMER0. If Serial is used, PINS 0,1 are reserved for Serial()
used Keypad is a 8x6 keypad from a Philips Videopac G7000 with a dedicated reset button, which is used as shift key or to reset a game in Emulationstation
**/

#include "UsbKeyboard.h"
//some keys that were not defined in UsbKeyboard.h
//see UsbKeyboard.h for reference
#define KEY_DEL    42
#define KEY_SLASH  56
#define KEY_PLUS   87
#define KEY_MINUS  56
#define KEY_STAR  85
#define KEY_SLASH  84
#define KEY_EQUALS 103  //workaround send KEY_0 + MOD_SHIFT_LEFT
#define KEY_DOT    55
#define KEY_QUESTION 45  //workaround gets checked in loop and always sends MOD_SHIFT_LEFT + KEY_QUESTION
//keys that are specially modified by the shiftkey
#define KEY_ESCAPE 41
#define KEY_RIGHT_ARROW 79
#define KEY_LEFT_ARROW 80
#define KEY_DOWN_ARROW 81
#define KEY_UP_ARROW 82


const int NOKEY = -1;
const int DEBOUNCE = 150;
const int GAMEPAD_BUTTON_B = KEY_S;

const int RESET_GAME_COMB[] = {GAMEPAD_BUTTON_B, MOD_SHIFT_RIGHT};
const int RESET_CONSOLE_COMB[] = {KEY_ENTER, MOD_SHIFT_RIGHT};

//shift/reset/shift+B(back to emulationstation) Button
//defined as INPUT_PULLUP. connect button to pin0 and ground
//button is pressed when digitalRead(btnReset) == LOW
const byte btnReset = 0;

const int  ROWS = 6;
const int COLS = 8;
const byte rows[ROWS] = {A5,A4,A3,A2,A1,A0};
const byte cols[COLS] = {6, 8, 7, 9, 10, 11, 13, 12};

const int keyboardMap[ROWS][COLS] = {
  {KEY_0,KEY_2,KEY_1,KEY_3, KEY_4,KEY_5,KEY_7,KEY_6},
  {KEY_8,KEY_0,KEY_9,KEY_1,KEY_SPACE,KEY_QUESTION,KEY_P,KEY_L},
  {KEY_PLUS,KEY_E,KEY_W,KEY_R,KEY_T,KEY_U,KEY_O,KEY_I},
  {KEY_Q,KEY_D,KEY_S,KEY_F,KEY_G,KEY_H,KEY_K,KEY_J},
  {KEY_A,KEY_X,KEY_Y,KEY_C,KEY_V,KEY_B,KEY_DOT,KEY_M},
  {KEY_MINUS,KEY_SLASH,KEY_STAR,KEY_EQUALS,KEY_Z,KEY_N,KEY_ENTER,KEY_DEL}
};
const int SPECIALSHIFTKEYS = 5;
const int specialShiftKeys[SPECIALSHIFTKEYS][2] = {
   {KEY_PLUS,KEY_LEFT_ARROW},
   {KEY_MINUS,KEY_RIGHT_ARROW},
   {KEY_STAR, KEY_UP_ARROW},
   {KEY_SLASH, KEY_DOWN_ARROW},
   {KEY_0, KEY_ESCAPE}
};
   
   
boolean shift = false;

//set input pins with pullup resistor and output pins
void initPins(){
 
  for(int i = 0; i < ROWS; i++) {
    pinMode(rows[i],INPUT_PULLUP);
  }
  for(int i = 0; i < COLS; i++) {
    pinMode(cols[i],OUTPUT);
    digitalWrite(cols[i], HIGH);
  }
  pinMode(btnReset, INPUT_PULLUP); //reset button is not part of the keypad
}

//returns value of pressed key. If no key was pressed, NOKEY gets returned
int getPressedKey() {
 for(int c = 0; c< COLS; c++) {
    digitalWrite(cols[c],LOW);
    for(int r = 0; r < ROWS; r++) {
      if(digitalRead(rows[r]) == LOW) {
          delayMs(DEBOUNCE);
          if(digitalRead(rows[r]) == LOW) {
            digitalWrite(cols[c], HIGH);
            return(keyboardMap[r][c]);
          }
      }
      
    }
    digitalWrite(cols[c], HIGH);
  } 
 
 return NOKEY; 
}

//handles behaviour of the deticated reset (shift) button of the videopac keyboard
//sends special key commands via usb
//push reset button: hold shift
//hold reset button: reset game
//hold reset button long: back to menu
void handleResetButton(){
  boolean toggleShift = false;
  
  if(digitalRead(btnReset) == LOW) {
    delayMs(DEBOUNCE);
    if(digitalRead(btnReset) == LOW) {
      toggleShift = true;
      delayMs(2*DEBOUNCE);
      if(digitalRead(btnReset) == LOW) {
        toggleShift = false;
        delayMs(DEBOUNCE);
        if(digitalRead(btnReset) == LOW) {
          UsbKeyboard.sendKeyStroke(RESET_GAME_COMB[0], RESET_GAME_COMB[1]);//reset game
          delayMs(4*DEBOUNCE);
          if(digitalRead(btnReset) == LOW) {
            delayMs(DEBOUNCE);
            if(digitalRead(btnReset) == LOW) {
              UsbKeyboard.sendKeyStroke(RESET_CONSOLE_COMB[0], RESET_CONSOLE_COMB[1]);//back to menu
            }
          }
        }
      }
    }
  }
  if(toggleShift){
    shift = !shift;
  }
}

//if shift key is pressed or a special key should be send, where I did not find out the key value
//sends key combination via usb
boolean isSpecialKey(char key){
  boolean special = false;
  if(key == KEY_QUESTION) {//workaround for key_?
      UsbKeyboard.sendKeyStroke(KEY_QUESTION, MOD_SHIFT_LEFT);
      special = true;
  }
  else if(key == KEY_EQUALS) {//workaround for key_=
    UsbKeyboard.sendKeyStroke(KEY_0, MOD_SHIFT_LEFT);
    special = true;
  }
  else if(shift) {
    int i = 0;
    while( !special && i < SPECIALSHIFTKEYS) {
      if(key == specialShiftKeys[i][0]) {
        UsbKeyboard.sendKeyStroke(specialShiftKeys[i][1]);
        special = true;
      }
      i++;
    }
  }
  return special;
}

void setup() {
  initPins();
  
  // Disable timer0 since it can mess with the USB timing. Note that
  // this means some functions such as delay() will no longer work.
  TIMSK0&=!(1<<TOIE0);

  // Clear interrupts while performing time-critical operations
  cli();

  // Force re-enumeration so the host will detect us
  usbDeviceDisconnect();
  delayMs(250);
  usbDeviceConnect();

  // Set interrupts again
  sei();
}

void loop() {
  UsbKeyboard.update();
  
  handleResetButton();
  int key = getPressedKey();

  if(key != NOKEY) {	//if key was pressed
    if(!isSpecialKey(key)) {	//if a normal key was pressed
      if(shift) {
        UsbKeyboard.sendKeyStroke(key, MOD_SHIFT_LEFT);
      } else {
        UsbKeyboard.sendKeyStroke(key);
      }
    }
  }
  
}
//special delay function is needed, because TIMER0 is deactivated (millis(), delay()... do not work)
void delayMs(unsigned int ms) {
  for (int i = 0; i < ms; i++) {
    delayMicroseconds(1000);
  }
}
