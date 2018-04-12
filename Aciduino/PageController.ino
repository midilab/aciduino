/*
[midi controller]
knobs: cutoff freq./decay, resonance/accent, env mod/tunning, tempo

buttons: track 1, track 2, ctrl A, ctrl B, wave form, play/stop
*/
// TODO: implement pickup by value for controllers

uint8_t _selected_ctrl = 0;
uint8_t _selected_waveform[2] = { 0 };

void processControllerButtons()
{
  if ( released(GENERIC_BUTTON_1) ) {
    _selected_track = 0;
  }

  if ( released(GENERIC_BUTTON_2) ) {
    _selected_track = 1;
  }

  if ( pressed(GENERIC_BUTTON_3) ) {
    _selected_ctrl = 0;
  }

  if ( pressed(GENERIC_BUTTON_4) ) {
    _selected_ctrl = 1;
  }

  if ( pressed(GENERIC_BUTTON_5) ) {
    // send wave form cc change
    _selected_waveform[_selected_track] = !_selected_waveform[_selected_track];
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
  
}

