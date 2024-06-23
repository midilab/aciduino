#include "../../uCtrl/uCtrl.h"

#include "../../aciduino.hpp"

//============================================
// Aciduino Features Setup
//============================================

// make use of bpm led?
// change for another pin if use any spi module
//#define USE_BPM_LED ??

// main navigation
// choose you changer! only one please...
//#define USE_CHANGER_POT
#define USE_CHANGER_ENCODER
//#define USE_TRANSPORT_BUTTON

#define USE_MIDI1
#define USE_SERIAL_MIDI_115200
#define USE_MIDI2
//#define USE_MIDI3
//#define USE_MIDI4

// to extend your aciduino controls you can make use of the following uMODULAR modules:
// PUSH and LED modules require booth PUSH_SPI and LED_SPI to point into some spi device
#define USE_PUSH_8      // uses 165 shiftregister (buttons)
//#define USE_PUSH_24     // uses 3x 165 shiftregister
//#define USE_PUSH_32     // uses 4x 165 shiftregister
#define USE_LED_8       // uses 595 shiftregister
//#define USE_LED_24      // uses 3x 595 shiftregister
//#define USE_POT_8       // uses 4051 multiplexer
#define USE_POT_16      // uses 2x 4051 multiplexer
//#define USE_TOUCH_32    // uses 2x 4067 multiplexer


//============================================
// PINOUT Setup
//============================================

// going to use changer encoder?
#define NAV_ENCODER_DEC_PIN       28
#define NAV_ENCODER_INC_PIN       26

// or use changer pot?
//#define CHANGER_POT_PIN           A8

#define NAV_SHIFT_PIN             13

#define NAV_FUNCTION1_PIN         12
#define NAV_FUNCTION2_PIN         11
#define NAV_GENERAL1_PIN          10
#define NAV_GENERAL2_PIN          6
#define NAV_RIGHT_PIN             5
#define NAV_UP_PIN                4
#define NAV_DOWN_PIN              3
#define NAV_LEFT_PIN              2

//#define TRANSPORT_BUTTON_1_PIN  7

// Extension modules
// PUSH 
#define PUSH_LATCH_PIN            9
// LED
#define LED_LATCH_PIN             8
// POT
#define POT_CTRL_PIN1             45
#define POT_CTRL_PIN2             47
#define POT_CTRL_PIN3             49
#define POT_MUX_COMM1             A0
#define POT_MUX_COMM2             A1


//============================================
// Managed Devices Setup
//============================================

// Display device
//U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

// Midi device
// initing midi devices
MIDI_CREATE_INSTANCE(HardwareSerial, Serial, MIDI1);
MIDI_CREATE_INSTANCE(HardwareSerial, Serial3, MIDI2);
//MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI3);
//MIDI_CREATE_INSTANCE(HardwareSerial, Serial2, MIDI4);

// SPI devices
#define PUSH_SPI          SPI
#define LED_SPI           SPI

// the main interface choice
#include "../../interface/midilab/main.h"

void initPort()
{

  //
  // OLED setup
  //
  uCtrl.initOled(&u8g2);
  uCtrl.oled->flipDisplay(1);

  //
  // Storage setup
  //
  uCtrl.initStorage();

  //
  // DIN Module
  //
  // inits with shift register support
  uCtrl.initDin(&PUSH_SPI, PUSH_LATCH_PIN);
  // dec
  uCtrl.din->plug(NAV_ENCODER_DEC_PIN);
  // inc
  uCtrl.din->plug(NAV_ENCODER_INC_PIN);
  // shift button
  uCtrl.din->plug(NAV_SHIFT_PIN);
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
  // any shiftregister to plug?
  uCtrl.din->plugSR(1);

  // encoders setup
  // in pair order always! and pairs starting with odd ids
  uCtrl.din->encoder(ENCODER_DEC, ENCODER_INC);

  //
  // DOUT Module
  //
  uCtrl.initDout(&LED_SPI, LED_LATCH_PIN);

/* #if defined(USE_BPM_LED)
  // builtin led for BPM led
  uCtrl.dout->plug(USE_BPM_LED);
#endif */

  uCtrl.dout->plugSR(1);

  //
  // AIN Module
  //
  uCtrl.initAin(POT_CTRL_PIN1, POT_CTRL_PIN2, POT_CTRL_PIN3);
  uCtrl.ain->plugMux(POT_MUX_COMM1);
  uCtrl.ain->plugMux(POT_MUX_COMM2);
  // get a global entry point for our midi pot controllers
  uCtrl.ain->setCallback(midiControllerHandle);

  //
  // MIDI Module
  //
  uCtrl.initMidi();
  uCtrl.midi->plug(&MIDI1);
  #if defined(USE_SERIAL_MIDI_115200)
  // forces MidiInterface back to 115200
  Serial.begin(115200);
  #endif
  uCtrl.midi->plug(&MIDI2);
  //uCtrl.midi->plug(&MIDI3);
  //uCtrl.midi->plug(&MIDI4);
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
  uCtrl.page->setNavComponentCtrl(SHIFT_BUTTON, UP_BUTTON, DOWN_BUTTON, PREVIOUS_BUTTON, NEXT_BUTTON, PAGE_BUTTON_1, PAGE_BUTTON_2, GENERIC_BUTTON_1, GENERIC_BUTTON_2, ENCODER_DEC, ENCODER_INC);
  // hook button callback setup
  // previous track
  uCtrl.page->setShiftCtrlAction(GENERIC_BUTTON_1, Aciduino::previousTrack);
  // next track
  uCtrl.page->setShiftCtrlAction(GENERIC_BUTTON_2, Aciduino::nextTrack);
  // transport play/stop and shifted rec on/off
  //uCtrl.page->setCtrlAction(TRANSPORT_BUTTON_1, playStop);
  //uCtrl.page->setShiftCtrlAction(TRANSPORT_BUTTON_1, recToggle);
  uCtrl.page->setCtrlAction(SELECTOR_BUTTON_8, Aciduino::playStop);
  uCtrl.page->setShiftCtrlAction(SELECTOR_BUTTON_8, Aciduino::recToggle);
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

