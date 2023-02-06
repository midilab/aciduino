#ifndef __U_CTRL_MODULES_H__
#define __U_CTRL_MODULES_H__

// wichg hardware version you're using?
// using lite protoboard?
#define USE_PROTOBOARD
//#define USE_UONE
//#define USE_MEGA_SHIELD

// wich modules you need acidman?
//#define USE_PUSH_8      // uses 165 shiftregister (buttons)
//#define USE_PUSH_24     // uses 3x 165 shiftregister
//#define USE_PUSH_32     // uses 4x 165 shiftregister
//#define USE_LED_8       // uses 595 shiftregister
//#define USE_LED_24      // uses 3x 595 shiftregister
//#define USE_POT_8       // uses 4051 multiplexer
//#define USE_POT_16      // uses 2x 4051 multiplexer
//#define USE_TOUCH_32    // uses 2x 4067 multiplexer

// pre-defined modules per hardware released
#if defined(USE_PROTOBOARD)
  // make use of bpm led?
  // change for another pin if use any spi module
  #define USE_BPM_LED       LED_BUILTIN
  // main navigation
  // choose you changer! only one please...
  #define USE_CHANGER_POT
  //#define USE_CHANGER_ENCODER
#elif defined(USE_UONE)
  #define USE_CHANGER_ENCODER
  #define USE_LED_24       
  #define USE_TOUCH_32
#elif defined(USE_MEGA_SHIELD)
  #define USE_CHANGER_ENCODER
  #define USE_PUSH_8
  #define USE_LED_8
  #define USE_POT_16
#endif

// DO NOT EDIT ANYTHING BELLOW!

//
// MIDI
//
#define USE_MIDI
#if defined(TEENSYDUINO) && defined(USB_MIDI)
  #define USE_USB_MIDI
#endif

//
// setup Oled
//
#define USE_OLED
#define USE_OLED_U8G2

//
// setup Page system
//
#define USE_PAGE
#define USE_PAGE_MAX_PAGES  5
#define USE_PAGE_MAX_SUB_PAGES  2
#define USE_PAGE_COMPONENT

//
// setup Storage
//
#define USE_STORAGE

//
// setup DIN
//
#define USE_DIN
// needs 165 shiftregister spi driver support?
#if defined(USE_PUSH_8) || defined(USE_PUSH_24)
  #define USE_DIN_SPI_DRIVER
#endif
#if defined(USE_PROTOBOARD) || defined(USE_CHANGER_ENCODER)
  #define USE_DIN_PORT_PIN
#endif

//
// setup DOUT
//
#define USE_DOUT
#if defined(USE_BPM_LED)
  #define USE_DOUT_PORT_PIN
#endif
// needs 165 shiftregister spi driver support?
#if defined(USE_LED_8) || defined(USE_LED_24)
  #define USE_DOUT_SPI_DRIVER
#endif

//
// setup AIN?
//
#if defined(USE_CHANGER_POT) || defined (USE_POT_8) || defined (USE_POT_16)
  #define USE_AIN
  #if defined (USE_POT_8) || defined (USE_POT_16)
    #define USE_AIN_4051
    #define LEARN_ENABLED
  #endif
#endif

//
// setup Touch?
//
#if defined (USE_TOUCH_32)
  #define USE_CAP_TOUCH
#endif

// setup Cv/Gate
//
//#define USE_CV_GATE

//
// setup Sdcard
//
//#define USE_SDCARD

#endif
