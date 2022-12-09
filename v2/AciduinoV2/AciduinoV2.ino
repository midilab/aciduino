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

// generative engine ui data
typedef struct
{
  uint8_t generative_fill = 80;
  uint8_t accent_probability = 50;
  uint8_t slide_probability = 30;
  uint8_t tie_probability = 100;
  uint8_t lower_octave = 2;
  uint8_t range_octave = 3;
  uint8_t number_of_tones = 5;
} GENERATIVE_303_DATA; 

typedef struct
{
  uint8_t generative_fill = 80;
  uint8_t accent_probability = 50;
  uint8_t roll_probability = 0;
} GENERATIVE_808_DATA; 

GENERATIVE_303_DATA generative_303[TRACK_NUMBER_303];
GENERATIVE_808_DATA generative_808[TRACK_NUMBER_808];

#if defined(USE_LITE_BOARD)

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

#elif defined(USE_UONE_BOARD)

typedef enum {
  ENCODER_DEC = 1,
  ENCODER_INC,
  SHIFT_BUTTON, 
  PAGE_BUTTON_1,
  PAGE_BUTTON_2,
  GENERIC_BUTTON_1,
  GENERIC_BUTTON_2,  
  NEXT_BUTTON,
  UP_BUTTON,
  DOWN_BUTTON,
  PREVIOUS_BUTTON,
  SELECTOR_BUTTON_1,
  SELECTOR_BUTTON_2,
  SELECTOR_BUTTON_3,
  SELECTOR_BUTTON_4,
  SELECTOR_BUTTON_5,
  SELECTOR_BUTTON_6,
  SELECTOR_BUTTON_7,
  SELECTOR_BUTTON_8,
  STEP_BUTTON_1,
  STEP_BUTTON_2,
  STEP_BUTTON_3,
  STEP_BUTTON_4,  
  STEP_BUTTON_5,
  STEP_BUTTON_6,
  STEP_BUTTON_7,
  STEP_BUTTON_8,  
  STEP_BUTTON_9,
  STEP_BUTTON_10,
  STEP_BUTTON_11,
  STEP_BUTTON_12,  
  STEP_BUTTON_13,
  STEP_BUTTON_14,
  STEP_BUTTON_15,
  STEP_BUTTON_16,  
} BUTTONS_INTERFACE_CONTROLS;
#endif

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
