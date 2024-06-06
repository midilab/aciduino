#include "../../uCtrl/uCtrl.h"

#include "../../aciduino.hpp"

// teensy 3.2+ on protoboard

//============================================
// Aciduino Features Setup
//============================================

// make use of bpm led?
// change for another pin if use any spi module
#define USE_BPM_LED       LED_BUILTIN
// main navigation
#define USE_CHANGER_POT
//#define USE_CHANGER_ENCODER
#define USE_TRANSPORT_BUTTON

//#define USE_POT_MICRO

#define FLIP_DISPLAY
#define INVERT_POT_READ

#define USE_MIDI1
#define USE_MIDI2

// for protoboard hacks
#define USE_TEENSY_PROTOBOARD_HACKS

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

// you need USE_POT_MICRO to enable those pins
//#define POT_MICRO_1_PIN   ??
//#define POT_MICRO_2_PIN   ??
//#define POT_MICRO_3_PIN   ??
//#define POT_MICRO_4_PIN   ??
// + 4 pots extensions
//#define POT_MICRO_5_PIN   ??
//#define POT_MICRO_6_PIN   ??
//#define POT_MICRO_7_PIN   ??
//#define POT_MICRO_8_PIN   ??

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

// little hack to make the shift protoboard work, ground our gnd button pin -2 related to button pin to avoid floating noises around...
#if defined(USE_TEENSY_PROTOBOARD_HACKS)
  pinMode(NAV_SHIFT_PIN-2, OUTPUT);
  digitalWrite(NAV_SHIFT_PIN-2, LOW);
#if defined(USE_TRANSPORT_BUTTON)
  // in case transport button
  pinMode(TRANSPORT_BUTTON_1_PIN-2, OUTPUT);
  digitalWrite(TRANSPORT_BUTTON_1_PIN-2, LOW);
#endif
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

#if defined(USE_TEENSY_PROTOBOARD_HACKS)
  // little hack to make the pot on aciduino protoboard work, ground our gnd pot pin 22 to avoid floating noises around...
  pinMode(22, OUTPUT);
  digitalWrite(22, LOW);
#endif
  // raise the average reads for pot for better stability
  uCtrl.ain->setAvgReads(8);

#endif // if defined(USE_POT_8) || defined(USE_POT_16) > else

  //
  // MIDI Module
  //
  uCtrl.initMidi();
  // Plugin MIDI interfaces to handle
#if defined(USE_MIDI1)
  uCtrl.midi->plug(&MIDI1);
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