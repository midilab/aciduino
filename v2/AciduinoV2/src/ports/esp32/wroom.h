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

//#define USE_POT_MICRO

#define FLIP_DISPLAY
//#define INVERT_POT_READ

// enable ble-midi? needs USE_MIDI2
//#define USE_BT_MIDI_ESP32

// enable Serial to be able to use on serial-to-midi bridges on PCs
// like hairless midi or ttymidi
// does it needs USE_MIDI1
#define USE_SERIAL_MIDI_115200

#define USE_MIDI1 // USB MIDI, needs a serial-to-midi bridge on the other side: hariless midi or ttymidi
//#define USE_MIDI2 // BLE MIDI: bluetooth is not a good option to run realtime application like midi, so be aware of shit timming and issues(good for a midi controller only)
#define USE_MIDI3 // real MIDI serial port: connect to your hardware synths via MIDI cable(check midi out schematics for 3.3v)

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


//============================================
// PINOUT Setup
//============================================

// Some of the ADC2 pins are used as strapping pins (GPIO 0, 2, 15) thus cannot be used freely. Such is the case in the following official Development Kits:

// going to use changer encoder?
//#define NAV_ENCODER_DEC_PIN       13
//#define NAV_ENCODER_INC_PIN       4

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
#define CHANGER_POT_PIN             15

// 4 pot extension
//#define POT_MICRO_1_PIN           15
// GPIO 35 - ADC1_7
/* #define POT_MICRO_1_PIN   35
// GPIO 34 - ADC1_6
#define POT_MICRO_2_PIN   34
// GPIO 39 - ADC1_3
#define POT_MICRO_3_PIN   39
// GPIO 36 - ADC1_0
#define POT_MICRO_4_PIN   36 */
// + 4 pots extensions
/* // GPIO 35 - ADC1_7 ???
#define POT_MICRO_5_PIN   ??
// GPIO 34 - ADC1_6  ???
#define POT_MICRO_6_PIN   ??
// GPIO 39 - ADC1_3  ???
#define POT_MICRO_7_PIN   ??
// GPIO 36 - ADC1_0  ??
#define POT_MICRO_8_PIN   ?? */

//============================================
// Managed Devices Setup
//============================================

// Display device
//U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// Midi device
// initing midi devices
#if defined(USE_MIDI1) // USB
MIDI_CREATE_INSTANCE(HardwareSerial, Serial, MIDI1);
#endif

#if defined(USE_MIDI2) // Bluetooth
#if defined(USE_BT_MIDI_ESP32) && defined(CONFIG_BT_ENABLED) 
BLEMIDI_CREATE_INSTANCE("Aciduino", MIDI2);
#endif
#endif

#if defined(USE_MIDI3) // Hardware midi 
MIDI_CREATE_INSTANCE(HardwareSerial, Serial2, MIDI3); 
#endif

// in case we got USB native mode support builtin, use it! note the case of wroom, keep this setup for later other boards
//#if defined(CONFIG_TINYUSB_ENABLED)
//ESPNATIVEUSBMIDI espNativeUsbMidi;
//MIDI_CREATE_INSTANCE(ESPNATIVEUSBMIDI, espNativeUsbMidi, MIDI1);
//#endif

// SPI devices
//#define PUSH_SPI          SPI
//#define LED_SPI           SPI
