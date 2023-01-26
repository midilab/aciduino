
//
// epprom memory layout setup
//
#define EPPROM_SIZE                 EEPROM.length()
#define EPPROM_CHECK_DATA_ADDRESS   0
#define EPPROM_SESSION_ADDRESS      2
#define EPPROM_SESSION_SIZE         (sizeof(_generative_303) + sizeof(_generative_808) + sizeof(_control_map_global) + sizeof(_track_output_setup))
#define EPRROM_PATTERN_ADDRESS      (EPPROM_SESSION_ADDRESS + EPPROM_SESSION_SIZE)
#define EPRROM_PATTERN_AVAILABLE    (EPPROM_SIZE-EPPROM_SESSION_SIZE-EPPROM_SESSION_ADDRESS) / (AcidSequencer.get303PatternMemorySize() + AcidSequencer.get808PatternMemorySize())
//
// common data definitions
//
#define PATTERN_303_TRACK_SIZE      AcidSequencer.get303PatternTrackSize()
#define PATTERN_808_TRACK_SIZE      AcidSequencer.get808PatternTrackSize()
#define PATTERN_303_MEM_SIZE        AcidSequencer.get303PatternMemorySize()
#define PATTERN_808_MEM_SIZE        AcidSequencer.get808PatternMemorySize()
#define PATTERN_TOTAL_MEM_SIZE      (PATTERN_303_MEM_SIZE + PATTERN_808_MEM_SIZE)

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

// mute automation grid
typedef struct
{
  uint16_t map_303 = 0xFFFF;
  uint16_t map_808[TRACK_NUMBER_808] = {0xFFFF};
} MUTE_PATTERN;

MUTE_PATTERN _mute_pattern[4];

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

bool loadPattern(uint8_t pattern, int8_t track = -1)
{
  uint16_t pattern_address = EPRROM_PATTERN_ADDRESS + (pattern * PATTERN_TOTAL_MEM_SIZE);
  if (track != -1) {
    uint16_t pattern_track_address = pattern_address + (AcidSequencer.is303(track) ? PATTERN_303_TRACK_SIZE * track : PATTERN_303_MEM_SIZE + (PATTERN_808_TRACK_SIZE * (track-TRACK_NUMBER_303)));
    // load only a track patttern
    AcidSequencer.setMute(track, true);
    AcidSequencer.clearStackNote(track);
    uCtrl.storage->load(AcidSequencer.getPatternData(track), (AcidSequencer.is303(track) ? PATTERN_303_TRACK_SIZE : PATTERN_808_TRACK_SIZE), pattern_track_address);
    AcidSequencer.setMute(track, false);
  } else {
    // load the whole pattern for all tracks
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

    // unmute all loaded fresh and new pattern
    for (uint8_t i=0; i < (TRACK_NUMBER_303+TRACK_NUMBER_808); i++) {
      AcidSequencer.setMute(i, false);
    }
  }
}

bool savePattern(uint8_t pattern, int8_t track = -1)
{
  uint16_t pattern_address = EPRROM_PATTERN_ADDRESS + (pattern * PATTERN_TOTAL_MEM_SIZE);
  if (track != -1) {
    uint16_t pattern_track_address = pattern_address + (AcidSequencer.is303(track) ? PATTERN_303_TRACK_SIZE * track : PATTERN_303_MEM_SIZE + (PATTERN_808_TRACK_SIZE * (track-TRACK_NUMBER_303)));
    // save only a track patttern
    uCtrl.storage->save(AcidSequencer.getPatternData(track), (AcidSequencer.is303(track) ? PATTERN_303_TRACK_SIZE : PATTERN_808_TRACK_SIZE), pattern_track_address);
  } else {
    // saves the whole pattern for all tracks
    // save 303 whole pattern data first
    uCtrl.storage->save(AcidSequencer.getPatternData(0), PATTERN_303_MEM_SIZE, pattern_address);
    // then 808 whole pattern data last
    uCtrl.storage->save(AcidSequencer.getPatternData(TRACK_NUMBER_303), PATTERN_808_MEM_SIZE, pattern_address+PATTERN_303_MEM_SIZE);
  }
}

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
  return 0; //ESP.getFreeHeap(); 
}
#elif defined(ARDUINO_ARCH_AVR)
int freeRam () 
{
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}
#endif
