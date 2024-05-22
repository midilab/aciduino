
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
// storage schema
//

//
// epprom memory layout setup
//
#if defined(ARDUINO_ARCH_ESP32) || defined(ESP32)
#define EPPROM_SIZE                 4096
#else
#define EPPROM_SIZE                 EEPROM.length()
#endif
#define EPPROM_CHECK_DATA_ADDRESS   0
#define EPPROM_SESSION_ADDRESS      2
#define EPPROM_SESSION_SIZE         (sizeof(_generative_303) + sizeof(_generative_808) + sizeof(_control_map_global) + sizeof(_track_output_setup))
//
// common pattern data definitions
//
#define PATTERN_303_TRACK_SIZE      aciduino.seq.get303PatternTrackSize()
#define PATTERN_808_TRACK_SIZE      aciduino.seq.get808PatternTrackSize()
#define PATTERN_303_MEM_SIZE        aciduino.seq.get303PatternMemorySize()
#define PATTERN_808_MEM_SIZE        aciduino.seq.get808PatternMemorySize()
#define PATTERN_TOTAL_MEM_SIZE      (PATTERN_303_MEM_SIZE + PATTERN_808_MEM_SIZE + sizeof(_mute_pattern))
#define EPRROM_PATTERN_ADDRESS      (EPPROM_SESSION_ADDRESS + EPPROM_SESSION_SIZE)
#define EPRROM_PATTERN_AVAILABLE    (EPPROM_SIZE-EPPROM_SESSION_SIZE-EPPROM_SESSION_ADDRESS) / (PATTERN_TOTAL_MEM_SIZE)

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

// for global learn feature to keep track of controls setup by user
typedef struct
{
    int8_t ctrl = -1;
    uint8_t track;
} MIDI_CTRL_GLOBAL_MAP;

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
  void recToggle();
  void playStop();
  void previousTrack();
  void nextTrack();

  //AcidSequencerClass * seq() { return &_seq; };

  uint8_t getSelectedTrack() { return _selected_track; };
  uint8_t setSelectedTrack(uint8_t selected_track) { _selected_track = selected_track; };
  uint8_t isPlaying() { return _playing; };

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

  int freeRam();
  
  AcidSequencerClass seq;

  // create acessors later.. TODO
  // globals
  bool _playing = false;
  uint8_t _selected_track = 0;

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

private:
  void initSequencer();
  void uClockSetup();

  // session and storage
  uint16_t getPatternEppromAddress(uint8_t pattern, int8_t track = -1);
  bool checkEppromDataLayoutChange();
  void eppromInit();

  // todo: create acessors and modify usage 
  GENERATIVE_303_DATA _generative_303[TRACK_NUMBER_303];
  GENERATIVE_808_DATA _generative_808[TRACK_NUMBER_808];

  volatile MIDI_CTRL_GLOBAL_MAP _control_map_global[16];
  volatile TRACK_OUTPUT_DATA _track_output_setup[TRACK_NUMBER_303+TRACK_NUMBER_808];
  MUTE_PATTERN _mute_pattern[4];
  // TODO: should goes to session data?
  uint8_t _pattern_grid[TRACK_NUMBER_303 + TRACK_NUMBER_808];

  // helpers for copy and paste pattern
  uint8_t _copy_pattern = 0;
  int8_t _copy_track = -1;
    
};


extern Aciduino aciduino;

#endif // __ACIDUINO_H__