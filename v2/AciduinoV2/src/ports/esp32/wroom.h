#include "../../uCtrl/uCtrl.h"

#include "../../aciduino.hpp"

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

#define USE_MIDI1 // USB MIDI, if no TINY_USB in use it needs a serial-to-midi bridge on the other side: hariless midi or ttymidi
#define USE_MIDI2 // real MIDI serial port: connect to your hardware synths via MIDI cable(check midi out schematics for 3.3v)
#if defined(USE_BT_MIDI_ESP32)
#define USE_MIDI3 // BLE MIDI: bluetooth is not a good option to run realtime application like midi, so be aware of shit timming and issues(good for a midi controller only)
#endif

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

#if defined(USE_MIDI2) // Hardware midi 
MIDI_CREATE_INSTANCE(HardwareSerial, Serial2, MIDI2); 
#endif

#if defined(USE_MIDI3) // Bluetooth
#if defined(USE_BT_MIDI_ESP32) && defined(CONFIG_BT_ENABLED) 
BLEMIDI_CREATE_INSTANCE("Aciduino", MIDI3);
#endif
#endif

// SPI devices
//#define PUSH_SPI          SPI
//#define LED_SPI           SPI

// the main interface choice
#include "../../interface/midilab/main.h"

void initPort() {
  //
  // OLED setup
  //
  uCtrl.initOled(&u8g2);
#if defined(FLIP_DISPLAY)
  uCtrl.oled->flipDisplay(1);
#endif

  //
  // Storage setup
  //
  uCtrl.initStorage();
  //uCtrl.initStorage(&STORAGE_SPI, 7);

  //
  // DIN Module
  //
#if defined(USE_PUSH_8) || defined(USE_PUSH_24) || defined(USE_PUSH_32)
  // going with shiftregister and SPI?
  uCtrl.initDin(&PUSH_SPI, PUSH_LATCH_PIN);
#else
  uCtrl.initDin();
#endif

#if defined(USE_CHANGER_ENCODER)
  // dec
  uCtrl.din->plug(NAV_ENCODER_DEC_PIN);
  // inc
  uCtrl.din->plug(NAV_ENCODER_INC_PIN);
#endif

  // shift button
  uCtrl.din->plug(NAV_SHIFT_PIN);

  // the default aciduino schema implements all the
  // basic navigation for full usage without any CI
  // only the usage of 32 buttons will disables
  // the direct microcontroler port connect to main interface
#if !defined(USE_PUSH_32) && !defined(USE_TOUCH_32)
  // follow the register order of BUTTONS_INTERFACE_CONTROLS enum
  // page button 1
  uCtrl.din->plug(NAV_FUNCTION1_PIN);
  // page button 2
  uCtrl.din->plug(NAV_FUNCTION2_PIN);
  // decrementer button
  uCtrl.din->plug(NAV_GENERAL1_PIN);
  // incrementer button
  uCtrl.din->plug(NAV_GENERAL2_PIN);
  // next
  uCtrl.din->plug(NAV_RIGHT_PIN);
  // up
  uCtrl.din->plug(NAV_UP_PIN);
  // down
  uCtrl.din->plug(NAV_DOWN_PIN);
  // previous
  uCtrl.din->plug(NAV_LEFT_PIN);
#if defined(USE_TRANSPORT_BUTTON)
  // transport
  uCtrl.din->plug(TRANSPORT_BUTTON_1_PIN);
#endif

#endif

// any shiftregister to plug?
#if defined(USE_PUSH_8)
  uCtrl.din->plugSR(1);
#elif defined(USE_PUSH_24)
  uCtrl.din->plugSR(3);
#elif defined(USE_PUSH_32)
  uCtrl.din->plugSR(4);
#endif

#if defined(USE_CHANGER_ENCODER)
  // encoders setup
  // in pair order always! and pairs starting with odd ids
  uCtrl.din->encoder(ENCODER_DEC, ENCODER_INC);
#endif

  //
  // DOUT Module
  //
#if defined(USE_LED_8) || defined(USE_LED_24)
  uCtrl.initDout(&LED_SPI, LED_LATCH_PIN);
#else
  uCtrl.initDout();
#endif

#if defined(USE_BPM_LED)
  // builtin led for BPM led
  uCtrl.dout->plug(USE_BPM_LED);
#endif

#if defined(USE_LED_8)
  uCtrl.dout->plugSR(1);
#elif defined(USE_LED_24)
  uCtrl.dout->plugSR(3);
#endif

  //
  // AIN Module
  //
#if defined(USE_POT_8) || defined(USE_POT_16)
  uCtrl.initAin(POT_CTRL_PIN1, POT_CTRL_PIN2, POT_CTRL_PIN3);

#if defined(USE_CHANGER_POT)
  uCtrl.ain->plug(CHANGER_POT_PIN);
#endif

  uCtrl.ain->plugMux(POT_MUX_COMM1);

#if defined(USE_POT_16)
  uCtrl.ain->plugMux(POT_MUX_COMM2);
#endif

  // get a global entry point for our midi pot controllers
  uCtrl.ain->setCallback(midiControllerHandle);

// else > no POT on mux
#else
  uCtrl.initAin();

#if defined(USE_CHANGER_POT)
  uCtrl.ain->plug(CHANGER_POT_PIN);
#endif

#if defined(USE_POT_MICRO)

#if defined(POT_MICRO_1_PIN)
  uCtrl.ain->plug(POT_MICRO_1_PIN);
#endif
#if defined(POT_MICRO_2_PIN)
  uCtrl.ain->plug(POT_MICRO_2_PIN);
#endif
#if defined(POT_MICRO_3_PIN)
  uCtrl.ain->plug(POT_MICRO_3_PIN);
#endif
#if defined(POT_MICRO_4_PIN)
  uCtrl.ain->plug(POT_MICRO_4_PIN);
#endif
#if defined(POT_MICRO_5_PIN)
  uCtrl.ain->plug(POT_MICRO_5_PIN);
#endif
#if defined(POT_MICRO_6_PIN)
  uCtrl.ain->plug(POT_MICRO_6_PIN);
#endif
#if defined(POT_MICRO_7_PIN)
  uCtrl.ain->plug(POT_MICRO_7_PIN);
#endif
#if defined(POT_MICRO_8_PIN)
  uCtrl.ain->plug(POT_MICRO_8_PIN);
#endif

  // get a global entry point for our midi pot controllers
  uCtrl.ain->setCallback(midiControllerHandle);
#endif

#if defined(INVERT_POT_READ)
  // our aciduino v2 protoboard can only connect with vcc and gnd swaped, lets inform that to uctrl ain driver
  uCtrl.ain->invertRead(true);
#endif

  // raise the average reads for pot for better stability
  uCtrl.ain->setAvgReads(8);

#endif // if defined(USE_POT_8) || defined(USE_POT_16) > else

  //
  // MIDI Module
  //
  uCtrl.initMidi();
  // ESP32 related
#if defined(USE_BT_MIDI_ESP32) && defined(USE_MIDI3) && defined(CONFIG_BT_ENABLED) && (defined(ARDUINO_ARCH_ESP32) || defined(ESP32))
  BLEMIDI3.setHandleConnected([]() {
    //uCtrl.dout->write(BPM_LED, HIGH, 0);
    _ble_midi_connected = 1;
  });
  
  BLEMIDI3.setHandleDisconnected([]() {
    //uCtrl.dout->write(BPM_LED, LOW, 0);
    _ble_midi_connected = 0;
  });
#endif

  // Plugin MIDI interfaces to handle
#if defined(USE_MIDI1)
  uCtrl.midi->plug(&MIDI1);
  #if defined(USE_SERIAL_MIDI_115200)
  // forces MidiInterface back to 115200
  Serial.begin(115200);
  #endif
#endif
#if defined(USE_MIDI2)
  uCtrl.midi->plug(&MIDI2);
#endif
#if defined(USE_MIDI3)
  uCtrl.midi->plug(&MIDI3);
#endif
#if defined(USE_MIDI4)
  uCtrl.midi->plug(&MIDI4);
#endif
  uCtrl.midi->setMidiInputCallback(Aciduino::midiInputHandler);
  // uCtrl realtime deals
  // process midi at 250 microseconds speed
  uCtrl.setOn250usCallback(Aciduino::midiHandleSync);
  // process midi input at 1ms speed
  uCtrl.setOn1msCallback(Aciduino::midiHandle);

  //
  // SdCard Module
  //
  //uCtrl.initSdCard(&SPI, 7);

  //
  // Page Module for UI
  //
  uCtrl.initPage(5);
  // syst | seqr | gene | ptrn | midi
  system_page_init();
  step_sequencer_page_init();
  generative_page_init();
  pattern_page_init();
  midi_page_init();
  // component UI interface setup
#if defined(USE_CHANGER_ENCODER)
  uCtrl.page->setNavComponentCtrl(SHIFT_BUTTON, UP_BUTTON, DOWN_BUTTON, PREVIOUS_BUTTON, NEXT_BUTTON, PAGE_BUTTON_1, PAGE_BUTTON_2, GENERIC_BUTTON_1, GENERIC_BUTTON_2, ENCODER_DEC, ENCODER_INC);
#else
  uCtrl.page->setNavComponentCtrl(SHIFT_BUTTON, UP_BUTTON, DOWN_BUTTON, PREVIOUS_BUTTON, NEXT_BUTTON, PAGE_BUTTON_1, PAGE_BUTTON_2, GENERIC_BUTTON_1, GENERIC_BUTTON_2);
#endif
#if defined(USE_CHANGER_POT)
  uCtrl.page->setNavPot(0); // id=0, first registred ain pin via plug() call
#endif
  // hook button callback setup
  // previous track
  uCtrl.page->setShiftCtrlAction(GENERIC_BUTTON_1, Aciduino::previousTrack);
  // next track
  uCtrl.page->setShiftCtrlAction(GENERIC_BUTTON_2, Aciduino::nextTrack);

  // transport play/stop and shifted rec on/off
#if defined(USE_TRANSPORT_BUTTON)
  uCtrl.page->setCtrlAction(TRANSPORT_BUTTON_1, Aciduino::playStop);
  uCtrl.page->setShiftCtrlAction(TRANSPORT_BUTTON_1, Aciduino::recToggle);
#elif defined(USE_PUSH_8) || defined(USE_PUSH_16) || defined(USE_PUSH_32)
  uCtrl.page->setCtrlAction(SELECTOR_BUTTON_8, Aciduino::playStop);
  uCtrl.page->setShiftCtrlAction(SELECTOR_BUTTON_8, Aciduino::recToggle);
#endif

  // bottom bar for f1 and f2 functions draw function
  uCtrl.page->setFunctionDrawCallback(functionDrawCallback);

  // init uCtrl modules and memory
  uCtrl.init();

  // get all leds off
  uCtrl.dout->writeAll(LOW);

  // default page to call at init
  uCtrl.page->setPage(0);

  // sequencer parameters could be initialized here?
  // how many 303 tracks? 808 tracks? 
  aciduino.init();
}