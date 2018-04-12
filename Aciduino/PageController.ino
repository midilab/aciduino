/*
[midi controller]
knobs: cutoff freq./decay, resonance/accent, env mod/tunning, tempo

buttons: track 1, track 2, ctrl A, ctrl B, wave form, play/stop
*/
// TODO: implement pickup by value for controllers

// MIDI CC definitions
#define MIDI_CTRL_TUNNING     79
#define MIDI_CTRL_CUTOFF      80
#define MIDI_CTRL_RESONANCE   81
#define MIDI_CTRL_ENVMOD      82
#define MIDI_CTRL_DECAY       83
#define MIDI_CTRL_ACCENT      84
#define MIDI_CTRL_WAVE        85 

uint8_t _selected_ctrl = 0;
uint8_t _selected_waveform[2] = { 0 };

void processControllerButtons()
{
  if ( released(GENERIC_BUTTON_1) ) {
    lockPotsState(true);
    _selected_track = 0;
  }

  if ( released(GENERIC_BUTTON_2) ) {
    lockPotsState(true);
    _selected_track = 1;
  }

  if ( pressed(GENERIC_BUTTON_3) ) {
    lockPotsState(true);
    _selected_ctrl = 0;
  }

  if ( pressed(GENERIC_BUTTON_4) ) {
    lockPotsState(true);
    _selected_ctrl = 1;
  }

  if ( pressed(GENERIC_BUTTON_5) ) {
    _selected_waveform[_selected_track] = !_selected_waveform[_selected_track];
    // send wave form cc change
    if ( _selected_waveform[_selected_track] == 0 ) {
      sendMidiMessage(MIDI_CC, MIDI_CTRL_WAVE, 0, _sequencer[_selected_track].channel, true); 
    } else if ( _selected_waveform[_selected_track] == 1 ) {
      sendMidiMessage(MIDI_CC, MIDI_CTRL_WAVE, 127, _sequencer[_selected_track].channel, true); 
    }
  }
}

void processControllerLeds()
{
  if ( _selected_track == 0 ) {
    digitalWrite(GENERIC_LED_1, HIGH);
    digitalWrite(GENERIC_LED_2, LOW);
  } else if ( _selected_track == 1 ) {
    digitalWrite(GENERIC_LED_1, LOW);
    digitalWrite(GENERIC_LED_2, HIGH);
  } 
    
  if ( _selected_ctrl == 0 ) {
    digitalWrite(GENERIC_LED_3, HIGH);
    digitalWrite(GENERIC_LED_4, LOW);
  } else if ( _selected_ctrl == 1 ) {
    digitalWrite(GENERIC_LED_3, LOW);
    digitalWrite(GENERIC_LED_4, HIGH);
  } 

  digitalWrite(GENERIC_LED_5, _selected_waveform[_selected_track]);
}

void processControllerPots()
{
  uint16_t value;
  uint8_t ctrl;
  
  // GENERIC_POT_1: cutoff freq./decay
  value = getPotChanges(GENERIC_POT_1, 0, 127);
  if ( value != -1 ) {  
    // send cc
    if ( _selected_ctrl == 0 ) {
      ctrl = MIDI_CTRL_CUTOFF;
    } else if ( _selected_ctrl == 1 ) {
      ctrl = MIDI_CTRL_DECAY;
    }
    sendMidiMessage(MIDI_CC, ctrl, value, _sequencer[_selected_track].channel, true);    
  }  

  // GENERIC_POT_2: resonance/accent
  value = getPotChanges(GENERIC_POT_2, 0, 127);
  if ( value != -1 ) {  
    // send cc
    if ( _selected_ctrl == 0 ) {
      ctrl = MIDI_CTRL_RESONANCE;
    } else if ( _selected_ctrl == 1 ) {
      ctrl = MIDI_CTRL_ACCENT;
    }
    sendMidiMessage(MIDI_CC, ctrl, value, _sequencer[_selected_track].channel, true);      
  }  

  // GENERIC_POT_3: env mod/tunning
  value = getPotChanges(GENERIC_POT_3, 0, 127);
  if ( value != -1 ) {  
    // send cc
    if ( _selected_ctrl == 0 ) {
      ctrl = MIDI_CTRL_ENVMOD;
    } else if ( _selected_ctrl == 1 ) {
      ctrl = MIDI_CTRL_TUNNING;
    }
    sendMidiMessage(MIDI_CC, ctrl, value, _sequencer[_selected_track].channel, true);     
  }      
}

