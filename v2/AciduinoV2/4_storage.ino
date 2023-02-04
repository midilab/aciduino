
//
// epprom memory layout setup
//
#define EPPROM_SIZE                 EEPROM.length()
#define EPPROM_CHECK_DATA_ADDRESS   0
#define EPPROM_SESSION_ADDRESS      2
#define EPPROM_SESSION_SIZE         (sizeof(_generative_303) + sizeof(_generative_808) + sizeof(_control_map_global) + sizeof(_track_output_setup))
//
// common pattern data definitions
//
#define PATTERN_303_TRACK_SIZE      AcidSequencer.get303PatternTrackSize()
#define PATTERN_808_TRACK_SIZE      AcidSequencer.get808PatternTrackSize()
#define PATTERN_303_MEM_SIZE        AcidSequencer.get303PatternMemorySize()
#define PATTERN_808_MEM_SIZE        AcidSequencer.get808PatternMemorySize()
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

GENERATIVE_303_DATA _generative_303[TRACK_NUMBER_303];
GENERATIVE_808_DATA _generative_808[TRACK_NUMBER_808];

// for global learn feature to keep track of controls setup by user
typedef struct
{
    int8_t ctrl = -1;
    uint8_t track;
} MIDI_CTRL_GLOBAL_MAP;

volatile MIDI_CTRL_GLOBAL_MAP _control_map_global[16];

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

volatile TRACK_OUTPUT_DATA _track_output_setup[TRACK_NUMBER_303+TRACK_NUMBER_808];

// pattern and mute automation grid
typedef struct
{
  uint16_t map_303 = 0xFFFF;
  uint16_t map_808[TRACK_NUMBER_808] = {0xFFFF};
} MUTE_PATTERN;

MUTE_PATTERN _mute_pattern[4];

// TODO: should goes to session data?
uint8_t _pattern_grid[TRACK_NUMBER_303 + TRACK_NUMBER_808];

// helpers for copy and paste pattern
uint8_t _copy_pattern = 0;
int8_t _copy_track = -1;
    
bool loadSession()
{
  // check if there is a session saved.. otherwise init one based on the defaults
  // we can use the last byte as version control
  // load session data
  uCtrl.storage->load(_generative_303, sizeof(_generative_303), EPPROM_SESSION_ADDRESS);
  uCtrl.storage->load(_generative_808, sizeof(_generative_808));
  uCtrl.storage->load(_control_map_global, sizeof(_control_map_global));
  uCtrl.storage->load(_track_output_setup, sizeof(_track_output_setup));
}

bool saveSession()
{
  // save session data
  uCtrl.storage->save(_generative_303, sizeof(_generative_303), EPPROM_SESSION_ADDRESS);
  uCtrl.storage->save(_generative_808, sizeof(_generative_808));
  uCtrl.storage->save(_control_map_global, sizeof(_control_map_global));
  uCtrl.storage->save(_track_output_setup, sizeof(_track_output_setup));
}

uint16_t getPatternEppromAddress(uint8_t pattern, int8_t track = -1)
{
  uint16_t pattern_address = EPRROM_PATTERN_ADDRESS + (pattern * PATTERN_TOTAL_MEM_SIZE);
  if (track != -1) {
    pattern_address += AcidSequencer.is303(track) ? PATTERN_303_TRACK_SIZE * track : PATTERN_303_MEM_SIZE + (PATTERN_808_TRACK_SIZE * (track-TRACK_NUMBER_303));
  }
  return pattern_address;
}

bool loadPattern(uint8_t pattern, int8_t track = -1, int8_t to_track = -1)
{
  uint16_t pattern_address = 0;
  
  if (track != -1) {
    // load only a track patttern
    pattern_address = getPatternEppromAddress(pattern, track);
    
    // to_track request?
    if (to_track != -1)
      track = to_track;
      
    AcidSequencer.setMute(track, true);
    AcidSequencer.clearStackNote(track);
    uCtrl.storage->load(AcidSequencer.getPatternData(track), (AcidSequencer.is303(track) ? PATTERN_303_TRACK_SIZE : PATTERN_808_TRACK_SIZE), pattern_address);
    AcidSequencer.setMute(track, false);

    // load _mute_pattern only for track?
    //...
  } else {
    // load the whole pattern for all tracks
    pattern_address = getPatternEppromAddress(pattern);
    
    // mute all before load into memory(its volatile! here a mute is enongth)
    for (uint8_t i=0; i < (TRACK_NUMBER_303+TRACK_NUMBER_808); i++) {
      AcidSequencer.setMute(i, true);
    }
    // clear all floating notes around
    AcidSequencer.clearStackNote();
    
    // get 303 whole pattern data first
    uCtrl.storage->load(AcidSequencer.getPatternData(0), PATTERN_303_MEM_SIZE, pattern_address);
    // then 808 whole pattern data last
    uCtrl.storage->load(AcidSequencer.getPatternData(TRACK_NUMBER_303), PATTERN_808_MEM_SIZE, pattern_address+PATTERN_303_MEM_SIZE);

    // load _mute_pattern
    uCtrl.storage->load((void *)&_mute_pattern, sizeof(_mute_pattern), pattern_address+PATTERN_303_MEM_SIZE+PATTERN_808_MEM_SIZE);
    
    // unmute all loaded fresh and new pattern
    for (uint8_t i=0; i < (TRACK_NUMBER_303+TRACK_NUMBER_808); i++) {
      AcidSequencer.setMute(i, false);
    }
  }
}

bool savePattern(uint8_t pattern, int8_t track = -1, int8_t from_track = -1)
{
  uint16_t pattern_address = 0;
  if (track != -1) {
    // save only a track patttern
    pattern_address = getPatternEppromAddress(pattern, track);
    
    // to_track request?
    if (from_track != -1)
      track = from_track;
      
    uCtrl.storage->save(AcidSequencer.getPatternData(track), (AcidSequencer.is303(track) ? PATTERN_303_TRACK_SIZE : PATTERN_808_TRACK_SIZE), pattern_address);

    // save _mute_pattern only for track?
    //...
  } else {
    // saves the whole pattern for all tracks
    pattern_address = getPatternEppromAddress(pattern);
    // save 303 whole pattern data first
    uCtrl.storage->save(AcidSequencer.getPatternData(0), PATTERN_303_MEM_SIZE, pattern_address);
    // then 808 whole pattern data last
    uCtrl.storage->save(AcidSequencer.getPatternData(TRACK_NUMBER_303), PATTERN_808_MEM_SIZE, pattern_address+PATTERN_303_MEM_SIZE);
    // _mute_grid data
    uCtrl.storage->save((void *)&_mute_pattern, sizeof(_mute_pattern), pattern_address+PATTERN_303_MEM_SIZE+PATTERN_808_MEM_SIZE);
  }
}

// copy current track data into same or another track pattern memory
bool copyPattern(uint8_t pattern, int8_t track = -1)
{
    _copy_pattern = pattern;
    _copy_track = track;
}

bool pastePattern(uint8_t pattern, int8_t track = -1)
{
  bool copy_memory = true;
  
  if (track == -1 || _copy_track == - 1) {
    // paste whole pattern track per track
    for (uint8_t i=0; i < AcidSequencer.getTrackNumber(); i++) {
      // update _copy_track with current track
      _copy_track = i;
      pastePattern(pattern, i);
    }
    // paste _mute_grid too!
    uCtrl.storage->save((void *)&_mute_pattern, sizeof(_mute_pattern), getPatternEppromAddress(pattern)+PATTERN_303_MEM_SIZE+PATTERN_808_MEM_SIZE);
    return true;
  }

  // check if track types are different
  if (AcidSequencer.is303(_copy_track) != AcidSequencer.is303(track)) {
    // cant copy.. handle UI
    return false;
  }
  
  // copy from memory or from storage?
  if (_pattern_grid[_copy_track] == _copy_pattern) {
    // we copy from current memory data
    copy_memory = true;
  } else {
    // we copy from storage data
    copy_memory = false;
  }

  // paste to memory or to storage?
  if (_pattern_grid[track] == pattern) {
    // TODO: should we save stuffs copied to memory?
    // we paste to current memory
    if (copy_memory) {
      // if paste is from memory to memory
      AcidSequencer.setMute(track, true);
      AcidSequencer.clearStackNote(track);
      memcpy(AcidSequencer.getPatternData(track), AcidSequencer.getPatternData(_copy_track), (AcidSequencer.is303(track) ? PATTERN_303_TRACK_SIZE : PATTERN_808_TRACK_SIZE));
      AcidSequencer.setMute(track, false);
    } else {
      // if paste is from storage to memory
      loadPattern(_copy_pattern, _copy_track, track);
    }
  } else {
    // we paste to storage data
    if (copy_memory) {
      // if paste is from memory to storage
      savePattern(pattern, track, _copy_track);
    } else {
      // if paste is from storage to storage
      uCtrl.storage->copy(getPatternEppromAddress(_copy_pattern, _copy_track), getPatternEppromAddress(pattern, track), (AcidSequencer.is303(track) ? PATTERN_303_TRACK_SIZE : PATTERN_808_TRACK_SIZE));
    }
  }

  return true;

}

/*
void Engine808::setBufferTrack(uint8_t track)
{
  // copy track into buffer
  memcpy(_track_helper_buffer, _sequencer[track], sizeof(_track_helper_buffer));
}
 */

bool checkEppromDataLayoutChange()
{
  uint16_t pattern_size = 0;
  uCtrl.storage->load(&pattern_size, sizeof(pattern_size), EPPROM_CHECK_DATA_ADDRESS);
  if (pattern_size == PATTERN_TOTAL_MEM_SIZE) {
    return false;
  } else {
    return true;
  }
}

void eppomInit()
{
    uint16_t pattern_size = PATTERN_TOTAL_MEM_SIZE;
    // init epprom session/pattern memory
    // init all epprom slots to defaults
    saveSession();
    for (uint8_t pattern=0; pattern < EPRROM_PATTERN_AVAILABLE; pattern++) {
      savePattern(pattern);
    }
    // mark epprom first bytes with pattern size to use as a checker for layout changes
    uCtrl.storage->save(&pattern_size, sizeof(pattern_size), EPPROM_CHECK_DATA_ADDRESS);
}

#if defined(TEENSYDUINO)
extern "C" char* sbrk(int incr);
int freeRam() {
  char top;
  return &top - reinterpret_cast<char*>(sbrk(0));
}
#elif defined(ARDUINO_ARCH_ESP32) || defined(ESP32)
int freeRam () 
{
  return ESP.getFreeHeap(); 
}
#elif defined(ARDUINO_ARCH_AVR)
int freeRam () 
{
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}
#endif
