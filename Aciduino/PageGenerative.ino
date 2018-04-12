/*
[generative]
knobs: ramdon low note, ramdon high note, harmonic mode, tempo

buttons: track 1, track 2, ramdomize it, transpose -, transpose +, play/stop
*/

uint8_t _lower_note = 36;
uint8_t _range_note = 34;

void processGenerativeButtons()
{
  if ( released(GENERIC_BUTTON_1) ) {
    _selected_track = 0;
  }

  if ( released(GENERIC_BUTTON_2) ) {
    _selected_track = 1;
  }

  if ( pressed(GENERIC_BUTTON_3) ) {
    // ramdomize it!
    acidRandomize();
  }

  if ( pressed(GENERIC_BUTTON_4) ) {
    // previous harmonic mode
    if ( _selected_mode > 0 ) {
      --_selected_mode;
    } else if ( _selected_mode == 0 ) {
      _harmonize = false;
    }
  }

  if ( pressed(GENERIC_BUTTON_5) ) {
    // next harmonic mode
    if ( _selected_mode < MODES_NUMBER-1 ) {
      if ( _harmonize == false ) {
        _harmonize = true;
      } else {
        ++_selected_mode;
      }
    }
  }
}

void processGenerativeLeds()
{
  if ( _selected_track == 0 ) {
    digitalWrite(GENERIC_LED_1, HIGH);
    digitalWrite(GENERIC_LED_2, LOW);
  } else if ( _selected_track == 1 ) {
    digitalWrite(GENERIC_LED_1, LOW);
    digitalWrite(GENERIC_LED_2, HIGH);
  } 

  if ( 1 ) {
    digitalWrite(GENERIC_LED_3, LOW);
  }
    
  if ( _harmonize == true ) {
    if ( _selected_mode  == MODES_NUMBER-1 ) {
      digitalWrite(GENERIC_LED_4, LOW);
      digitalWrite(GENERIC_LED_5, HIGH);
    } else {
      digitalWrite(GENERIC_LED_4, LOW);
      digitalWrite(GENERIC_LED_5, LOW);
    }
  } else {
    digitalWrite(GENERIC_LED_4, HIGH);
    digitalWrite(GENERIC_LED_5, LOW);
  }

}

void processGenerativePots()
{
  uint16_t value;
  
  // GENERIC_POT_1: Lower Note to be generated 
  value = getPotChanges(GENERIC_POT_1, 0, 127);
  if ( value != -1 ) {  
    _lower_note = value;
  }

  // GENERIC_POT_2: High Note to be generated 
  value = getPotChanges(GENERIC_POT_2, 0, 127);
  if ( value != -1 ) {  
    _range_note = value;
  }  

  // GENERIC_POT_3: Harmonic mode temperament 
  value = getPotChanges(GENERIC_POT_3, 0, 24);
  if ( value != -1 ) {  
    _transpose = value-12; // -12 (0) +12 
  }  

}

void acidRandomize() 
{
  uint8_t note, high_note, accent, glide, rest;
  
  // ramdom it all
  for ( uint16_t i = 0; i < STEP_MAX_SIZE; i++ ) {
    high_note = _lower_note+_range_note;
    if ( high_note > 127 ) {
      high_note = 127;
    }

    note = random(_lower_note, high_note);
    accent = random(0, 100);
    accent = accent < ACCENT_PROBABILITY_GENERATION ? 1 : 0;
    glide = random(0, 100);
    glide = glide < GLIDE_PROBABILITY_GENERATION ? 1 : 0;
    rest = random(0, 100);
    rest = rest < REST_PROBABILITY_GENERATION ? 1 : 0;
    
    ATOMIC(_sequencer[_selected_track].step[i].note = note);
    ATOMIC(_sequencer[_selected_track].step[i].accent = accent);
    ATOMIC(_sequencer[_selected_track].step[i].glide = glide);
    ATOMIC(_sequencer[_selected_track].step[i].rest = rest);
  }
}
