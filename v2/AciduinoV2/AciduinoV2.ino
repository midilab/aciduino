/* 
 
  Aciduino v2
  
  4 tracks/2 engines: 303, 808

  track types:
  MIDI: 303, 808
  CV: 303(1cv 3gates), 808(4 gates)
  SAMPLER: 808
  SYNTH: 303
  
  load any engine on any track
  default: 303, 303, 808, 808

  shift+knob turn = last used midi control of the channel

  shift+ generic1/2 changes track...

  shift+ nav left/nav right changes subpage

  shift+ nav up/nav down changes page

  shift+ page button 2 play/stop

  shift+ page button 1 tempo setup

  compatible with uOne

  create a #define to allow uOne 32 buttons and nav to work with same code base
*/
//
// BPM Clock support
//
#include "src/uClock/src/uClock.h"

//
// Controlling interfaces support
//
#include "src/uCtrl/uCtrl.h"

//
// Acid step sequencer
//
#include "src/sequencer/acid_sequencer.h"

// want a acid midi controller?
#define MIDI_CONTROLER
// want a uart serial midi port?
//#define USE_UART_MIDI
// want a usb midi port?
#define USE_USB_MIDI

// globals
bool _playing = false;
uint8_t _selected_track = 0;

// generative engine
uint8_t _generative_fill = 80;
uint8_t _accent_probability = 50;
uint8_t _roll_probability = 0;
uint8_t _slide_probability = 30;
uint8_t _tie_probability = 100;
uint8_t _lower_note = 26;
uint8_t _range_note = 23;
uint8_t _number_of_tones = 5;

typedef enum {
  PREVIOUS_BUTTON = 1,
  NEXT_BUTTON,
  UP_BUTTON,
  DOWN_BUTTON,
  PAGE_BUTTON_1,
  PAGE_BUTTON_2,
  GENERIC_BUTTON_1,
  GENERIC_BUTTON_2,
  SHIFT_BUTTON,   
} BUTTONS_INTERFACE_CONTROLS;

void setup() {
  // setup uctrl hardware/interface
  uCtrlSetup();
  // clock system
  uClockSetup();
  // the acid sequencer
  AcidSequencer.setMidiOutputCallback(midiOutHandler);
  //AcidSequencer.setCVOutputCallback();
}

void loop() {
  uCtrl.run();
}
