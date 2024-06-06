#include "../../uCtrl/uCtrl.h"

#include "../../aciduino.hpp"

//uONE!

//============================================
// Aciduino Features Setup
//============================================

#define USE_CHANGER_ENCODER

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

// the main interface choice
#include "../../interface/midilab/main.h"

void initPort() {
  //
  // OLED setup
  //
  uCtrl.initOled(&u8g2);

  //
  // Storage setup
  //
  uCtrl.initStorage();

  //
  // DIN Module
  //
  uCtrl.initDin();
  // dec
  uCtrl.din->plug(NAV_ENCODER_DEC_PIN);
  // inc
  uCtrl.din->plug(NAV_ENCODER_INC_PIN);
  // shift button
  uCtrl.din->plug(NAV_SHIFT_PIN);
  // encoders setup
  // in pair order always! and pairs starting with odd ids
  uCtrl.din->encoder(ENCODER_DEC, ENCODER_INC);

  //
  // DOUT Module
  //
  uCtrl.initDout(&LED_SPI, LED_LATCH_PIN);
  uCtrl.dout->plugSR(3);

  //
  // AIN Module
  //
  //uCtrl.initAin();
  //uCtrl.ain->plug(CHANGER_POT_PIN);
  // get a global entry point for our midi pot controllers
  //uCtrl.ain->setCallback(midiControllerHandle);
  // raise the average reads for pot for better stability
  //uCtrl.ain->setAvgReads(8);

  //
  // Capacitive Touch Module
  //
  uCtrl.initCapTouch(TOUCH_CTRL_PIN1, TOUCH_CTRL_PIN2, TOUCH_CTRL_PIN3, TOUCH_CTRL_PIN4);
  uCtrl.touch->setThreshold(TOUCH_TRESHOLD);
  uCtrl.touch->plug(TOUCH_MUX_COMM1);
  uCtrl.touch->plug(TOUCH_MUX_COMM2);

  //
  // MIDI Module
  //
  uCtrl.initMidi();

  // Plugin MIDI interfaces to handle
  uCtrl.midi->plug(&MIDI1);
  uCtrl.midi->plug(&MIDI2);
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