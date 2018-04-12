/*
[step edit]
knobs: octave, note, sequence length, tempo

buttons: prev step, next step, rest, glide, accent, play/stop
*/
// User Interface data
uint16_t _step_edit = 0;
uint8_t _last_octave = 3;
uint8_t _last_note = 0;

void sendPreviewNote(uint16_t step)
{
  unsigned long milliTime, preMilliTime;
  
  sendMidiMessage(NOTE_ON, _sequencer[_selected_track].step[step].note, _sequencer[_selected_track].step[step].accent ? ACCENT_VELOCITY : NOTE_VELOCITY, _sequencer[_selected_track].channel);

  // avoid delay() call because of uClock timmer1 usage
  //delay(200);
  preMilliTime = millis();
  while ( true ) {
    milliTime = millis();
    if (abs(milliTime - preMilliTime) >= 200) {
      break;
    }
  }
  
  sendMidiMessage(NOTE_OFF, _sequencer[_selected_track].step[step].note, 0, _sequencer[_selected_track].channel);
}

void processSequencerPots()
{
  static int8_t octave, note, step_note;
  static int16_t step_length;

  // GENERIC_POT_1: Note Octave Selector
  octave = getPotChanges(GENERIC_POT_1, 0, 10);
  if ( octave != -1 ) {  
    _last_octave = octave;
  }

  // GENERIC_POT_2: Note Selector (generic C to B, no octave)
  note = getPotChanges(GENERIC_POT_2, 0, 11);
  if ( note != -1 ) { 
    _last_note = note;
  }

  // changes on octave or note pot?
  if ( octave != -1 || note != -1 ) {
    //ATOMIC(_sequencer[_selected_track].step[_step_edit].note = (_last_octave * 8) + _last_note);
    note = harmonizer((_last_octave * 8) + _last_note);
    ATOMIC(_sequencer[_selected_track].step[_step_edit].note = note);
    if ( _playing == false && _sequencer[_selected_track].step[_step_edit].rest == false ) {
      sendPreviewNote(_step_edit);
    }
  }

  // GENERIC_POT_3: Sequencer step length
  step_length = getPotChanges(GENERIC_POT_3, 1, STEP_MAX_SIZE);
  if ( step_length != -1 ) {  
    ATOMIC(_sequencer[_selected_track].step_length = step_length);
    if ( _step_edit >= _sequencer[_selected_track].step_length ) {
      _step_edit = _sequencer[_selected_track].step_length-1;
    }
  }
}

void processSequencerButtons()
{

  // previous step edit
  if ( released(GENERIC_BUTTON_1) ) {
    if ( _step_edit != 0 ) {
      // add a lock here for octave and note to not mess with edit mode when moving steps around 
      lockPotsState(true);   
      --_step_edit;
    }
    if ( _playing == false && _sequencer[_selected_track].step[_step_edit].rest == false ) {
      sendPreviewNote(_step_edit);
    }
  }

  // next step edit
  if ( released(GENERIC_BUTTON_2) ) {
    if ( _step_edit < _sequencer[_selected_track].step_length-1 ) {
      // add a lock here for octave and note to not mess with edit mode when moving steps around
      lockPotsState(true);     
      ++_step_edit;
    }
    if ( _playing == false && _sequencer[_selected_track].step[_step_edit].rest == false ) {
      sendPreviewNote(_step_edit);
    }    
  }

  // step rest
  if ( pressed(GENERIC_BUTTON_3) ) {
    ATOMIC(_sequencer[_selected_track].step[_step_edit].rest = !_sequencer[_selected_track].step[_step_edit].rest);
    if ( _playing == false && _sequencer[_selected_track].step[_step_edit].rest == false ) {
      sendPreviewNote(_step_edit);
    }
  }

  // step glide
  if ( pressed(GENERIC_BUTTON_4) ) {
    ATOMIC(_sequencer[_selected_track].step[_step_edit].glide = !_sequencer[_selected_track].step[_step_edit].glide);
  }

  // step accent
  if ( pressed(GENERIC_BUTTON_5) ) {
    ATOMIC(_sequencer[_selected_track].step[_step_edit].accent = !_sequencer[_selected_track].step[_step_edit].accent);
    if ( _playing == false && _sequencer[_selected_track].step[_step_edit].rest == false ) {
      sendPreviewNote(_step_edit);
    }       
  }     
}
  
void processSequencerLeds()
{   
  // Editing First Step? 
  if ( _step_edit == 0 ) {
    digitalWrite(GENERIC_LED_1 , HIGH);
  } else {
    digitalWrite(GENERIC_LED_1 , LOW);
  }  

  // Editing Last Step? 
  if ( _step_edit == _sequencer[_selected_track].step_length-1 ) {
    digitalWrite(GENERIC_LED_2 , HIGH);
  } else {
    digitalWrite(GENERIC_LED_2 , LOW);
  }  
  
  // Rest 
  if ( _sequencer[_selected_track].step[_step_edit].rest == true ) {
    digitalWrite(GENERIC_LED_3 , HIGH);
  } else {
    digitalWrite(GENERIC_LED_3 , LOW);
  }

  // Glide 
  if ( _sequencer[_selected_track].step[_step_edit].glide == true ) {
    digitalWrite(GENERIC_LED_4 , HIGH);
  } else {
    digitalWrite(GENERIC_LED_4 , LOW);
  }  

  // Accent 
  if ( _sequencer[_selected_track].step[_step_edit].accent == true ) {
    digitalWrite(GENERIC_LED_5 , HIGH);
  } else {
    digitalWrite(GENERIC_LED_5 , LOW);
  } 
}
