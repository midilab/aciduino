#include "../../uCtrl/uCtrl.h"

//uONE!

//============================================
// Aciduino Features Setup
//============================================

// make use of bpm led?
// change for another pin if use any spi module
//#define USE_BPM_LED       LED_BUILTIN
// main navigation
//#define USE_CHANGER_POT
#define USE_CHANGER_ENCODER
//#define USE_TRANSPORT_BUTTON

//#define USE_POT_MICRO

//#define FLIP_DISPLAY
//#define INVERT_POT_READ

#define USE_MIDI1
#define USE_MIDI2

// wich modules you need acidman?
// PUSH and LED modules require booth PUSH_SPI and LED_SPI to point into some spi device
//#define USE_PUSH_8      // uses 165 shiftregister (buttons)
//#define USE_PUSH_24     // uses 3x 165 shiftregister
//#define USE_PUSH_32     // uses 4x 165 shiftregister
//#define USE_LED_8       // uses 595 shiftregister
#define USE_LED_24      // uses 3x 595 shiftregister
//#define USE_POT_8       // uses 4051 multiplexer
//#define USE_POT_16      // uses 2x 4051 multiplexer
#define USE_TOUCH_32    // uses 2x 4067 multiplexer
//#define TOUCH_TRESHOLD    90
#define TOUCH_TRESHOLD    100
//#define TOUCH_TRESHOLD    110


//============================================
// PINOUT Setup
//============================================

// going to use changer encoder?
#define NAV_ENCODER_DEC_PIN       24
#define NAV_ENCODER_INC_PIN       25
// teensy 3.2+
// on 3.2+ there are no digital port only analog at 24 and 25 pins :(
//#define NAV_ENCODER_DEC_PIN       A10
//#define NAV_ENCODER_INC_PIN       A11
// swap per A6, A7? make nav encoder dec=20(A6) inc=21(A7) on rev 0.2 uone

#define NAV_SHIFT_PIN             23

// the other controls are handled
// by touch driver

// Extension modules
// LED
#define LED_LATCH_PIN             17
// TOUCH
// 4 and 3 needs a swap, fix it on rev 0.2
#define TOUCH_CTRL_PIN1           6
#define TOUCH_CTRL_PIN2           5
#define TOUCH_CTRL_PIN3           3
#define TOUCH_CTRL_PIN4           4
#define TOUCH_MUX_COMM1           A6
#define TOUCH_MUX_COMM2           A7

//============================================
// Managed Devices Setup
//============================================

// Display device
//U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// Midi device
// initing midi devices
#define MIDI1         usbMIDI
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI2);

// SPI devices
#define PUSH_SPI          SPI
#define LED_SPI           SPI
