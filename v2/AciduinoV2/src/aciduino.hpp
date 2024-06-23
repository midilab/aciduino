
#ifndef __ACIDUINO_H__
#define __ACIDUINO_H__

#include <Arduino.h>

//
// BPM Clock support
//
#include "uClock/src/uClock.h"

//
// Controlling interfaces support
//
#include "uCtrl/uCtrl.h"

//
// Acid step sequencer
//
#include "sequencer/acid_sequencer.h"

//
// memory data structures for user data
//
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

// to be used with generativeParam accessors
typedef enum {
  GENERATIVE_FILL,
  GENERATIVE_ACCENT,
  GENERATIVE_SLIDE,
  GENERATIVE_TIE,
  GENERATIVE_LOWER_OCTAVE,
  GENERATIVE_RANGE_OCTAVE,
  GENERATIVE_NUM_TONES,
  GENERATIVE_ROLL,
} GENERATIVE_PARAM;

// for global learn feature to keep track of controls setup by user
typedef struct
{
  int8_t ctrl = -1;
  uint8_t track;
} MIDI_CTRL_GLOBAL_MAP;

typedef enum {
  MIDI_CTRL,
  MIDI_TRACK,
} MIDI_CTRL_PARAM;

// track output setup: midi/cv/gate/osc
typedef enum {
  MIDI_OUTPUT,
  CV_OUTPUT,
  GATE_OUTPUT, 
} SEQUENCER_OUTPUT;

typedef struct
{
  uint8_t output;
  uint8_t channel;
  uint8_t port;
} TRACK_OUTPUT_DATA;

typedef enum {
  TRACK_OUTPUT,
  TRACK_CHANNEL,
  TRACK_PORT,
} TRACK_OUTPUT_PARAM;

// pattern and mute automation grid
typedef struct
{
  uint16_t map_303 = 0xFFFF;
  uint16_t map_808[TRACK_NUMBER_808] = {0xFFFF};
} MUTE_PATTERN;


class Aciduino
{
public:

  void init();
  void run();

  // public interface
  static void recToggle();
  static void playStop();
  static void previousTrack();
  static void nextTrack();
  void play();
  void stop();
  uint8_t getSelectedTrack() { return _selected_track; };
  uint8_t setSelectedTrack(uint8_t selected_track) { _selected_track = selected_track; };
  uint8_t isPlaying() { return _playing; };
  void generatePattern(int8_t track = -1);

  // session
  void loadSession();
  void saveSession();
  void loadPattern(uint8_t pattern, int8_t track = -1, int8_t to_track = -1);
  void savePattern(uint8_t pattern, int8_t track = -1, int8_t from_track = -1);
  void saveMuteGrid(uint8_t pattern);
  void copyPattern(uint8_t pattern, int8_t track = -1);
  bool pastePattern(uint8_t pattern, int8_t track = -1);

  // midi
  void sendMidiCC(uint8_t cc, uint8_t value, uint8_t channel, uint8_t port, uint8_t interrupted = 0);
  void sendNote(uint8_t note, uint8_t channel, uint8_t port, uint8_t velocity);
  inline void sendMidiClock();
  void sendMidiStart();
  void sendMidiStop();

  uint8_t getMidiClockPort() { return _midi_clock_port; }
  void setMidiClockPort(uint8_t port) { ATOMIC(_midi_clock_port = port) }
  uint8_t getMidiRecPort() { return _midi_rec_port; }
  void setMidiRecPort(uint8_t port) { ATOMIC(_midi_rec_port = port) }

  // pattern grids
  // mute
  uint8_t getMuteGridState(uint8_t mute_pattern, uint8_t track, uint8_t voice = 0);
  void setMuteGridState(uint8_t mute_pattern, uint8_t state, uint8_t track, uint8_t voice = 0);
  // pattern
  uint8_t getPatternGrid(uint8_t track);
  void setPatternGrid(uint8_t track, uint8_t pattern);

  // generative data accessors
  uint8_t getGenerativeParam(uint8_t param, int8_t track = -1);
  void setGenerativeParam(uint8_t param, uint8_t data, int8_t track = -1);

  // midi controller data accessors
  int8_t getMidiControlParam(uint8_t param, uint8_t idx);
  void setMidiControlParam(uint8_t param, int8_t data, uint8_t idx);

  // track output setup data accessors
  uint8_t getTrackOutputParam(uint8_t param, int8_t track = -1);
  void setTrackOutputParam(uint8_t param, uint8_t data, int8_t track = -1);

  // system
  uint16_t getNumOfPatterns();
  uint16_t getSessionSize();
  uint16_t getStorageSize();

  // utils
  int freeRam();
  
  // midi input handles(mainly to be used as callback by external midi hanlders)
  static void midiInputHandler(uctrl::protocol::midi::MIDI_MESSAGE * msg, uint8_t port, uint8_t interrupted);
  static void midiHandle(); // sync handle
  static void midiHandleSync(); // note input handle

  // check sequencer/acid_sequencer.h for interface
  AcidSequencerClass seq;

private:
  void initSequencer();
  void uClockSetup();
  void storageSetup();

  // output handlers
  static void sequencerOutHandler(uint8_t msg_type, uint8_t note, uint8_t velocity, uint8_t track);
  void midiSequencerOutHandler(uint8_t msg_type, uint8_t byte1, uint8_t byte2, uint8_t channel, uint8_t port);

  // uclock callback handlers
  void handle_bpm_led(uint32_t tick);
  static void onSync24Callback(uint32_t tick);
  static void onPPQNCallback(uint32_t tick);
  static void onStepCallback(uint32_t step);
  static void onClockStart();
  static void onClockStop();

  // session and storage
  void eppromInit();
  uint16_t getPatternEppromAddress(uint8_t pattern, int8_t track = -1);
  bool checkEppromDataLayoutChange();

  GENERATIVE_303_DATA _generative_303[TRACK_NUMBER_303];
  GENERATIVE_808_DATA _generative_808[TRACK_NUMBER_808];

  volatile MIDI_CTRL_GLOBAL_MAP _control_map_global[16];
  volatile TRACK_OUTPUT_DATA _track_output_setup[TRACK_NUMBER_303+TRACK_NUMBER_808];
  MUTE_PATTERN _mute_grid[4];
  // TODO: should goes to session data?
  uint8_t _pattern_grid[TRACK_NUMBER_303 + TRACK_NUMBER_808];

  // helpers for copy and paste pattern
  uint8_t _copy_pattern = 0;
  int8_t _copy_track = -1;

  // state
  uint8_t _playing = 0;
  uint8_t _selected_track = 0;

  // uclock
  uint8_t _bpm_port = 0;
  uint8_t _bpm_blink_timer = 1;

  //
  // midi handling
  //
  // ussing 3 midi_messages data structure to
  // keep interrupted and non interrupted memory area safe
  uctrl::protocol::midi::MIDI_MESSAGE msg;
  uctrl::protocol::midi::MIDI_MESSAGE msg_interrupt;
  uctrl::protocol::midi::MIDI_MESSAGE msg_interrupt_pots;

  volatile uint8_t _midi_clock_port = 0; // 0 = internal
  volatile uint8_t _midi_rec_port = 1;

  // 
  // storage schema
  //

  //
  // epprom memory layout setup
  //
  uint16_t _epprom_size;
  uint16_t _epprom_check_data_address;
  uint16_t _epprom_session_address;
  uint16_t _epprom_session_size;
  //
  // common pattern data definitions
  //
  uint16_t _pattern_303_track_size;
  uint16_t _pattern_808_track_size;
  uint16_t _pattern_303_mem_size;
  uint16_t _pattern_808_mem_size;
  uint16_t _pattern_total_mem_size;
  uint16_t _eprrom_pattern_address;
  uint16_t _eprrom_pattern_available;

};


extern Aciduino aciduino;

#endif // __ACIDUINO_H__