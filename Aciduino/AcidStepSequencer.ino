// Acid StepSequencer, a Roland TB303 step sequencer engine clone
// author: midilab contact@midilab.co
// under MIT license
#include "uClock.h"

#define NOTE_STACK_SIZE    3

// MIDI clock, start, stop, note on and note off byte definitions - based on MIDI 1.0 Standards.
#define MIDI_CLOCK 0xF8
#define MIDI_START 0xFA
#define MIDI_STOP  0xFC
#define NOTE_ON    0x90
#define NOTE_OFF   0x80
#define MIDI_CC    0xB0

// sequencer data
typedef struct
{
  uint8_t note;
  int16_t length;
} STACK_NOTE_DATA;

typedef struct
{
  uint8_t note:7;
  uint8_t accent:1;
  uint8_t glide:1;
  uint8_t rest:1;
  uint8_t reserved:6;
} SEQUENCER_STEP_DATA;
// 2 bytes per step

typedef struct
{
  SEQUENCER_STEP_DATA step[STEP_MAX_SIZE];
  int8_t step_init_point;
  uint8_t step_length;
} SEQUENCER_TRACK_DATA;
// 32 bytes per 16 step + 2 bytes config = 34 bytes [STEP_MAX_SIZE=16]

typedef struct
{
  SEQUENCER_TRACK_DATA data;
  uint8_t step_location;
  uint8_t channel;
  bool mute;
  STACK_NOTE_DATA stack[NOTE_STACK_SIZE];  
} SEQUENCER_TRACK;

// main sequencer data is constantly change inside uClock 16PPQN and 96PPQN ISR callbacks, so volatile him!
SEQUENCER_TRACK volatile _sequencer[TRACK_NUMBER];

uint8_t _selected_track = 0;
uint8_t _selected_pattern = 0;

// make sure all above sequencer data are modified atomicly only
// eg. ATOMIC(_sequencer[track]data.step[0].accent = 1); ATOMIC(_sequencer[track].data.step_length = 7);
uint8_t _tmpSREG;
#define ATOMIC(X) _tmpSREG = SREG; cli(); X; SREG = _tmpSREG;

// shared data to be used for user interface interaction and feedback
bool _playing = false;
uint8_t _harmonize = 0;
uint16_t _step_edit = 0;
uint8_t _last_octave = 3;
uint8_t _last_note = 0;
int8_t _transpose = 0; // zero is centered C
uint8_t _selected_mode = 0;

void sendMidiMessage(uint8_t command, uint8_t byte1, uint8_t byte2, uint8_t channel, bool atomicly = false)
{   
  // send midi message
  command = command | (uint8_t)channel; 
  
  // if we want to use it with MIDI_CC from non timmer compare ISR code then set it to true
  if ( atomicly == true ) {
    _tmpSREG = SREG;
    cli();
  }
 
  Serial.write(command);
  Serial.write(byte1);
  Serial.write(byte2);
  
  if ( atomicly == true ) {
    SREG = _tmpSREG;
  }
}

// The callback function wich will be called by uClock each Pulse of 16PPQN clock resolution. Each call represents exactly one step.
void ClockOut16PPQN(uint32_t * tick) 
{
  uint8_t step;
  uint16_t length;
  int8_t note;

  for ( uint8_t track = 0; track < TRACK_NUMBER; track++ ) {

    if ( _sequencer[track].mute == true ) {
      continue;
    }
    
    length = NOTE_LENGTH;
    
    // get actual step location.
    _sequencer[track].step_location = uint32_t(*tick + _sequencer[track].data.step_init_point) % _sequencer[track].data.step_length;
    
    // send note on only if this step are not in rest mode
    if ( _sequencer[track].data.step[_sequencer[track].step_location].rest == 0 ) {
  
      // check for glide event ahead of _sequencer[track].step_location
      step = _sequencer[track].step_location;
      for ( uint8_t i = 1; i < _sequencer[track].data.step_length; i++  ) {
        ++step;
        step = step % _sequencer[track].data.step_length;
        if ( _sequencer[track].data.step[step].glide == 1 && _sequencer[track].data.step[step].rest == 1 ) {
          length = NOTE_LENGTH + (i * 6);
          break;
        } else if ( _sequencer[track].data.step[step].rest == 0 ) {
          break;
        }
      }
  
      // find a free note stack to fit in
      for ( uint8_t i = 0; i < NOTE_STACK_SIZE; i++ ) {
        if ( _sequencer[track].stack[i].length == -1 ) {
          if ( _harmonize == 1 ) {
            note = harmonizer(_sequencer[track].data.step[_sequencer[track].step_location].note);
          } else {
            note = _sequencer[track].data.step[_sequencer[track].step_location].note;
          }
          note += _transpose;
          // in case transpose push note away from the lower or higher midi note range barrier do not play it
          if ( note < 0 || note > 127 ) {
            break;
          }
          _sequencer[track].stack[i].note = note;
          _sequencer[track].stack[i].length = length;
          // send note on
          sendMidiMessage(NOTE_ON, note, _sequencer[track].data.step[_sequencer[track].step_location].accent ? ACCENT_VELOCITY : NOTE_VELOCITY, _sequencer[track].channel);    
          break;
        }
      }
      
    } 
    
  } 
  
}

void clearStackNote(int8_t track = -1)
{
  if ( track <= -1 ) {
    // clear all tracks stack note
    for ( uint8_t i = 0; i < TRACK_NUMBER; i++ ) {
      // clear and send any note off 
      for ( uint8_t j = 0; j < NOTE_STACK_SIZE; j++ ) {
        sendMidiMessage(NOTE_OFF, _sequencer[i].stack[j].note, 0, _sequencer[i].channel, true);
        _sequencer[i].stack[j].length = -1;
      } 
    }
  } else {
    // clear and send any note off 
    for ( uint8_t i = 0; i < NOTE_STACK_SIZE; i++ ) {
      sendMidiMessage(NOTE_OFF, _sequencer[track].stack[i].note, 0, _sequencer[track].channel, true);
      _sequencer[track].stack[i].length = -1;
    }     
  }

}

// The callback function wich will be called by uClock each Pulse of 96PPQN clock resolution.
void ClockOut96PPQN(uint32_t * tick) 
{
  uint8_t track;
    
  // Send MIDI_CLOCK to external hardware
  Serial.write(MIDI_CLOCK);

  for ( track = 0; track < TRACK_NUMBER; track++ ) {

    // handle note on stack
    for ( uint8_t i = 0; i < NOTE_STACK_SIZE; i++ ) {
      if ( _sequencer[track].stack[i].length != -1 ) {
        --_sequencer[track].stack[i].length;
        if ( _sequencer[track].stack[i].length == 0 ) {
          sendMidiMessage(NOTE_OFF, _sequencer[track].stack[i].note, 0, _sequencer[track].channel);
          _sequencer[track].stack[i].length = -1;
        }
      }  
    }
  
  }
  
  // user feedback about sequence time events
  tempoInterface(tick);
}

// The callback function wich will be called when clock starts by using Clock.start() method.
void onClockStart() 
{
  Serial.write(MIDI_START);
  _playing = 1;
}

// The callback function wich will be called when clock stops by using Clock.stop() method.
void onClockStop() 
{
  Serial.write(MIDI_STOP);

  // clear all tracks stack note
  clearStackNote();
    
  _playing = 0;
}

void setTrackChannel(uint8_t track, uint8_t channel) 
{
  --track;
  --channel;
  ATOMIC(_sequencer[track].channel = channel);
}

void initAcidStepSequencer(uint8_t mode)
{
  uint8_t track;
  
  // Initialize serial communication
  if ( mode == 0 ) {
    // the default MIDI serial speed communication at 31250 bits per second
    Serial.begin(31250);
  } else if ( mode == 1 ) {
    // for usage with a PC with a serial to MIDI bridge
    Serial.begin(115200);
  }

  // Inits the clock
  uClock.init();
  
  // Set the callback function for the clock output to send MIDI Sync message.
  uClock.setClock96PPQNOutput(ClockOut96PPQN);
  
  // Set the callback function for the step sequencer on 16ppqn
  uClock.setClock16PPQNOutput(ClockOut16PPQN);  
  
  // Set the callback function for MIDI Start and Stop messages.
  uClock.setOnClockStartOutput(onClockStart);  
  uClock.setOnClockStopOutput(onClockStop);
  
  // Set the clock BPM to 126 BPM
  uClock.setTempo(126);

  // initing sequencer memory data
  for ( track = 0; track < TRACK_NUMBER; track++ ) {

    _sequencer[track].channel = track;
    _sequencer[track].data.step_init_point = 0;
    _sequencer[track].data.step_length = STEP_MAX_SIZE;
    _sequencer[track].step_location = 0;
    _sequencer[track].mute = false;

    // initing note data
    for ( uint16_t i = 0; i < STEP_MAX_SIZE; i++ ) {
      _sequencer[track].data.step[i].note = 48;
      _sequencer[track].data.step[i].accent = 0;
      _sequencer[track].data.step[i].glide = 0;
      _sequencer[track].data.step[i].rest = 0;
    }
  
    // initing note stack data
    for ( uint8_t i = 0; i < NOTE_STACK_SIZE; i++ ) {
      _sequencer[track].stack[i].note = 0;
      _sequencer[track].stack[i].length = -1;
    }
    
  }
  
}

