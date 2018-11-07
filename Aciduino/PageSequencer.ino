/*
[step edit]
knobs: octave, note, global tunning, sequence length

buttons: prev step, next step, rest, glide, accent, play/stop
*/
void sendPreviewNote(uint16_t step)
{
  unsigned long milliTime, preMilliTime;
  uint8_t note;

  // enable or disable harmonizer
  if ( _harmonize == true ) {
    note = harmonizer(_sequencer[_selected_track].step[step].note);
  } else {
    note = _sequencer[_selected_track].step[step].note;
  }
  sendMidiMessage(NOTE_ON, note, _sequencer[_selected_track].step[step].accent ? ACCENT_VELOCITY : NOTE_VELOCITY, _sequencer[_selected_track].channel);

  // avoid delay() call because of uClock timmer1 usage
  //delay(200);
  preMilliTime = millis();
  while ( true ) {
    milliTime = millis();
    if (abs(milliTime - preMilliTime) >= 200) {
      break;
    }
  }
  
  sendMidiMessage(NOTE_OFF, note, 0, _sequencer[_selected_track].channel);
}

void processSequencerPots()
{
  static int8_t octave, note, step_note;
  static int16_t value;
  uint8_t relative_step = uint8_t(_step_edit + _sequencer[_selected_track].step_init_point) % _sequencer[_selected_track].step_length;

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
    //ATOMIC(_sequencer[_selected_track].step[relative_step].note = (_last_octave * 8) + _last_note);
    note = (_last_octave * 8) + _last_note;
    ATOMIC(_sequencer[_selected_track].step[relative_step].note = note);
    if ( _playing == false && _sequencer[_selected_track].step[relative_step].rest == false ) {
      sendPreviewNote(relative_step);
    }
  }

  // GENERIC_POT_3: global tunning (afects booth tracks) or track tunning
  value = getPotChanges(GENERIC_POT_3, 0, 24);
  if ( value != -1 ) {  
    clearStackNote();
    // -12 (0) +12 
    ATOMIC(_transpose = value-12); 
  }  

  // GENERIC_POT_4: sequencer step length
  value = getPotChanges(GENERIC_POT_4, 1, STEP_MAX_SIZE);
  if ( value != -1 ) {  
    clearStackNote(_selected_track);
    ATOMIC(_sequencer[_selected_track].step_length = value);
    if ( relative_step >= _sequencer[_selected_track].step_length ) {
      _step_edit = _sequencer[_selected_track].step_length-1;
    }
  }
  
}

void processSequencerButtons()
{
  uint8_t relative_step = uint8_t(_step_edit + _sequencer[_selected_track].step_init_point) % _sequencer[_selected_track].step_length;
  
  // previous step edit
  if ( released(GENERIC_BUTTON_1) ) {
    if ( _step_edit != 0 ) {
      // add a lock here for octave and note to not mess with edit mode when moving steps around 
      lockPotsState(true);   
      --_step_edit;
    }
    if ( _playing == false && _sequencer[_selected_track].step[relative_step].rest == false ) {
      sendPreviewNote(relative_step-1);
    }
  }

  // next step edit
  if ( released(GENERIC_BUTTON_2) ) {
    if ( _step_edit < _sequencer[_selected_track].step_length-1 ) {
      // add a lock here for octave and note to not mess with edit mode when moving steps around
      lockPotsState(true);     
      ++_step_edit;
    }
    if ( _playing == false && _sequencer[_selected_track].step[relative_step].rest == false ) {
      sendPreviewNote(relative_step+1);
    }    
  }

  // step rest
  if ( pressed(GENERIC_BUTTON_3) ) {
    ATOMIC(_sequencer[_selected_track].step[relative_step].rest = !_sequencer[_selected_track].step[relative_step].rest);
    if ( _playing == false && _sequencer[_selected_track].step[relative_step].rest == false ) {
      sendPreviewNote(relative_step);
    }
  }

  // step glide
  if ( pressed(GENERIC_BUTTON_4) ) {
    ATOMIC(_sequencer[_selected_track].step[relative_step].glide = !_sequencer[_selected_track].step[relative_step].glide);
  }

  // step accent
  if ( pressed(GENERIC_BUTTON_5) ) {
    ATOMIC(_sequencer[_selected_track].step[relative_step].accent = !_sequencer[_selected_track].step[relative_step].accent);
    if ( _playing == false && _sequencer[_selected_track].step[relative_step].rest == false ) {
      sendPreviewNote(relative_step);
    }       
  }     
}
  
void processSequencerLeds()
{   
  uint8_t relative_step = uint8_t(_step_edit + _sequencer[_selected_track].step_init_point) % _sequencer[_selected_track].step_length;
  
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
  if ( _sequencer[_selected_track].step[relative_step].rest == true ) {
    digitalWrite(GENERIC_LED_3 , HIGH);
  } else {
    digitalWrite(GENERIC_LED_3 , LOW);
  }

  // Glide 
  if ( _sequencer[_selected_track].step[relative_step].glide == true ) {
    digitalWrite(GENERIC_LED_4 , HIGH);
  } else {
    digitalWrite(GENERIC_LED_4 , LOW);
  }  

  // Accent 
  if ( _sequencer[_selected_track].step[relative_step].accent == true ) {
    digitalWrite(GENERIC_LED_5 , HIGH);
  } else {
    digitalWrite(GENERIC_LED_5 , LOW);
  } 
}
