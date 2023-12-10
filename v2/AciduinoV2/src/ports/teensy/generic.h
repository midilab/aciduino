#include "../../uCtrl/uCtrl.h"

// teensy 3.2+ on protoboard

//============================================
// Managed Devices Setup
//============================================

// Display device
//U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
#define FLIP_DISPLAY

// Midi device
// initing midi devices
#if defined(USB_MIDI)
  #define MIDI1         usbMIDI
  MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI2);
  #define USE_MIDI1
  #define USE_MIDI2
#else
  MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI1);
  #define USE_MIDI1
#endif

//============================================
// Aciduino Features Setup
//============================================

// make use of bpm led?
// change for another pin if use any spi module
#define USE_BPM_LED       LED_BUILTIN
// main navigation
// choose you changer! only one please...
#define USE_CHANGER_POT
//#define USE_CHANGER_ENCODER

// for protoboard hacks
#define USE_TEENSY_PROTOBOARD_HACKS

// wich modules you need acidman?
//#define USE_PUSH_8      // uses 165 shiftregister (buttons)
//#define USE_PUSH_24     // uses 3x 165 shiftregister
//#define USE_PUSH_32     // uses 4x 165 shiftregister
//#define USE_LED_8       // uses 595 shiftregister
//#define USE_LED_24      // uses 3x 595 shiftregister
//#define USE_POT_8       // uses 4051 multiplexer
//#define USE_POT_16      // uses 2x 4051 multiplexer
//#define USE_TOUCH_32    // uses 2x 4067 multiplexer

// learn support for pots?
#if defined (USE_POT_8) || defined (USE_POT_16)
  #define LEARN_ENABLED
#endif

//============================================
// PINOUT Setup
//============================================

// going to use changer encoder?
//#define NAV_ENCODER_DEC_PIN       24
//#define NAV_ENCODER_INC_PIN       25

#define NAV_SHIFT_PIN             4

#define NAV_FUNCTION1_PIN         6
#define NAV_FUNCTION2_PIN         7
#define NAV_GENERAL1_PIN          3
#define NAV_GENERAL2_PIN          5
#define NAV_RIGHT_PIN             15
#define NAV_UP_PIN                14
#define NAV_DOWN_PIN              9
#define NAV_LEFT_PIN              16

#define TRANSPORT_BUTTON_1_PIN    10

// going to use changer pot?
#define CHANGER_POT_PIN           A9