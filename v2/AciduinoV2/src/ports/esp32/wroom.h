#include "../../uCtrl/uCtrl.h"

// wroom on protoboard

//============================================
// Aciduino Features Setup
//============================================

// make use of bpm led?
// the blue led
#define LED_BUILTIN       2
#define USE_BPM_LED       LED_BUILTIN
// main navigation
#define USE_CHANGER_POT
//#define USE_CHANGER_ENCODER
//#define USE_TRANSPORT_BUTTON

#define FLIP_DISPLAY
//#define INVERT_POT_READ

#define USE_MIDI1
//#define USE_MIDI2

// wich modules you need acidman?
// PUSH and LED modules require booth PUSH_SPI and LED_SPI to point into some spi device
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
//#define NAV_ENCODER_DEC_PIN       5
//#define NAV_ENCODER_INC_PIN       18

#define NAV_SHIFT_PIN             12

#define NAV_FUNCTION1_PIN         14
#define NAV_FUNCTION2_PIN         27
#define NAV_GENERAL1_PIN          33
#define NAV_GENERAL2_PIN          32
#define NAV_RIGHT_PIN             5
#define NAV_UP_PIN                18
#define NAV_DOWN_PIN              19
#define NAV_LEFT_PIN              23

//#define TRANSPORT_BUTTON_1_PIN    ??

// going to use changer pot?
// esp32 30pins
// GPIO 15 - ADC2_3
//#define CHANGER_POT_PIN             15
// esp32 38pins
// GPIO 0 - ADC2_1
#define CHANGER_POT_PIN           0

//============================================
// Managed Devices Setup
//============================================

// Display device
//U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// Midi device
// initing midi devices
// in case we got USB native mode support builtin, use it!
#if defined(CONFIG_TINYUSB_ENABLED)
  ESPNATIVEUSBMIDI espNativeUsbMidi;
  MIDI_CREATE_INSTANCE(ESPNATIVEUSBMIDI, espNativeUsbMidi, MIDI1);
  MIDI_CREATE_INSTANCE(HardwareSerial, Serial2, MIDI2);
#else
  MIDI_CREATE_INSTANCE(HardwareSerial, Serial, MIDI1);
  //BLEMIDI_CREATE_INSTANCE("Aciduino", MIDI2);
#endif
// #if defined(CONFIG_BT_ENABLED)...

// SPI devices
//#define PUSH_SPI          SPI
//#define LED_SPI           SPI
