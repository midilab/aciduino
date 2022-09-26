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
*/
//
// BPM Clock support
//
#include "src/uClock/uClock.h"

//
// Controlling interfaces support
//
#include "src/uCtrl/uCtrl.h"

// step sequencer
#include "src/sequencer/acid_sequencer.h"

// globals
bool _playing = false;
uint8_t _selected_track = 0;
uint8_t _generative_fill = 40;

// shared data to be used for user interface interaction and feedback
uctrl::protocol::midi::MIDI_MESSAGE msg;

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

extern "C" char* sbrk(int incr);
int freeram() {
  char top;
  return &top - reinterpret_cast<char*>(sbrk(0));
}
