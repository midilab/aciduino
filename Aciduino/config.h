#ifndef __CONFIG_H__
#define __CONFIG_H__

//
// MIDI Config
//
// MIDI_STANDARD(0) for 31250 standard compilant MIDI devices(use with MIDI 5 pin connector)
// MIDI_SERIAL(1) for 115200 compilant with serial devices(use with PC and a serial-to-midi converter)
#define MIDI_MODE           0
// MIDI Channel
#define TRACK1_CHANNEL      1
#define TRACK2_CHANNEL      2

//
// MIDI Controller config
//
#define USE_MIDI_CTRL         
#define MIDI_CTRL_TUNNING     79
#define MIDI_CTRL_CUTOFF      80
#define MIDI_CTRL_RESONANCE   81
#define MIDI_CTRL_ENVMOD      82
#define MIDI_CTRL_DECAY       83
#define MIDI_CTRL_ACCENT      84
#define MIDI_CTRL_WAVE        85 

//
// User interface config
//
#define SEQUENCER_MIN_BPM     50
#define SEQUENCER_MAX_BPM     177

//
// Generative config
//
#define ACCENT_PROBABILITY_GENERATION   50
#define GLIDE_PROBABILITY_GENERATION    30
#define REST_PROBABILITY_GENERATION     10

//
// Sequencer config
//
#define TRACK_NUMBER       2 // you can go up to 8 but no interface ready to control it
#define STEP_MAX_SIZE      16
#define NOTE_LENGTH        4 // min: 1 max: 5 DO NOT EDIT BEYOND!!!
#define NOTE_VELOCITY      90
#define ACCENT_VELOCITY    127

//
// Hardware config
//
#define POT_NUMBER    4
#define BUTTON_NUMBER 6

// Hardware config
#define ADC_RESOLUTION        1024
#define POT_SENSITIVITY       2
// Pin configuration(double check your schematic before configure those pins)
// Pots
#define GENERIC_POT_1_PIN     A3
#define GENERIC_POT_2_PIN     A2
#define GENERIC_POT_3_PIN     A1
#define GENERIC_POT_4_PIN     A0
// Buttons
#define GENERIC_BUTTON_1_PIN  2
#define GENERIC_BUTTON_2_PIN  3
#define GENERIC_BUTTON_3_PIN  4
#define GENERIC_BUTTON_4_PIN  5
#define GENERIC_BUTTON_5_PIN  6
#define GENERIC_BUTTON_6_PIN  7
// Leds
#define GENERIC_LED_1         8
#define GENERIC_LED_2         9
#define GENERIC_LED_3         10
#define GENERIC_LED_4         11
#define GENERIC_LED_5         12
#define GENERIC_LED_6         13

typedef enum {
  GENERIC_POT_1,
  GENERIC_POT_2,
  GENERIC_POT_3,
  GENERIC_POT_4  
} POT_HARDWARE_INTERFACE;

typedef enum {
  GENERIC_BUTTON_1,
  GENERIC_BUTTON_2,
  GENERIC_BUTTON_3,
  GENERIC_BUTTON_4,
  GENERIC_BUTTON_5,
  GENERIC_BUTTON_6
} BUTTON_HARDWARE_INTERFACE;

#endif
