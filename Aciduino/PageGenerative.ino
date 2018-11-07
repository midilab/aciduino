/*
[generative]
knobs: ramdon low range note, ramdon high range note, number of notes to use(1 to 12 or 1 to 7 if harmonized), ramdomizer signatures

buttons: shift left sequence, shift rigth sequence, ramdomize it, harmonizer scale-, harmonizer scale+, play/stop
*/

uint8_t _lower_note = 36;
uint8_t _range_note = 34;
uint8_t _accent_probability = ACCENT_PROBABILITY_GENERATION;
uint8_t _glide_probability = GLIDE_PROBABILITY_GENERATION;
uint8_t _rest_probability = REST_PROBABILITY_GENERATION;
uint8_t _number_of_tones = 3;

uint8_t _allowed_tones[12] = {0};

void shiftSequence(int8_t offset)
{
  // clear stack note(also send any note on to off) before shift sequence init point
  clearStackNote(_selected_track);
  ATOMIC(_sequencer[_selected_track].step_init_point = _sequencer[_selected_track].step_init_point+offset); 
}

void processGenerativeButtons()
{
  if ( released(GENERIC_BUTTON_1) ) {
    shiftSequence(-1);
  }

  if ( released(GENERIC_BUTTON_2) ) {
    shiftSequence(1);
  }

  if ( pressed(GENERIC_BUTTON_3) ) {
    // ramdomize it!
    acidRandomize();
  }

  if ( pressed(GENERIC_BUTTON_4) ) {
    // previous harmonic mode
    if ( _selected_mode > 0 ) {
      ATOMIC(--_selected_mode);
    } else if ( _selected_mode == 0 ) {
      ATOMIC(_harmonize = false);
    }
  }

  if ( pressed(GENERIC_BUTTON_5) ) {
    // next harmonic mode
    if ( _selected_mode < MODES_NUMBER-1 ) {
      if ( _harmonize == false ) {
        ATOMIC(_harmonize = true);
      } else {
        ATOMIC(++_selected_mode);
      }
    }
  }
}

void processGenerativeLeds()
{
  digitalWrite(GENERIC_LED_1, LOW);
  digitalWrite(GENERIC_LED_2, LOW); 
  digitalWrite(GENERIC_LED_3, LOW);
    
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
  
  // GENERIC_POT_1: lower range note to be generated 
  value = getPotChanges(GENERIC_POT_1, 0, 127);
  if ( value != -1 ) {  
    _lower_note = value;
  }

  // GENERIC_POT_2: high range note to be generated 
  value = getPotChanges(GENERIC_POT_2, 0, 127);
  if ( value != -1 ) {  
    _range_note = value;
  }  

  // GENERIC_POT_3: number of notes to use on sequence(1 to 12 or 1 to 7 if harmonized)
  if ( _harmonize == true ) {
    value = getPotChanges(GENERIC_POT_3, 1, 7);
  } else if ( _harmonize == false ) {
    value = getPotChanges(GENERIC_POT_3, 1, 12);
  }
  if ( value != -1 ) {  
    _number_of_tones = value;
    uint8_t note = 0;
    for ( uint8_t i=0; i < 12; i++ ) {
      if ( i%(12/_number_of_tones) == 0 && i != 0 ) {
        note += (12/_number_of_tones); 
      }
      _allowed_tones[i] = note;
    }
  }  
  
  // GENERIC_POT_4: ramdomizer signatures
  value = getPotChanges(GENERIC_POT_4, 0, 70);
  if ( value != -1 ) {  
    //_accent_probability = value???;
    //_glide_probability  value???;
    _rest_probability = 70-value;
  }    

}

uint8_t getNoteByMaxNumOfTones(uint8_t note)
{
  uint8_t octave, relative_note;

  octave = note/12;
  relative_note = note%12;
  return _allowed_tones[relative_note] + _lower_note + (octave*12);
}

void acidRandomize() 
{
  uint8_t note, high_note, accent, glide, rest;

  // clear track before random data or only clear stack note?
  // probably clear stack note is a better idea
  
  // ramdom it all
  for ( uint16_t i = 0; i < STEP_MAX_SIZE; i++ ) {
    high_note = _lower_note+_range_note;
    if ( high_note > 127 ) {
      high_note = 127;
    }

    note = getNoteByMaxNumOfTones(random(_lower_note, high_note));
    accent = random(0, 100) < _accent_probability ? 1 : 0;
    glide = random(0, 100) < _glide_probability ? 1 : 0;
    rest = random(0, 100) < _rest_probability ? 1 : 0;
    
    ATOMIC(_sequencer[_selected_track].step[i].note = note);
    ATOMIC(_sequencer[_selected_track].step[i].accent = accent);
    ATOMIC(_sequencer[_selected_track].step[i].glide = glide);
    ATOMIC(_sequencer[_selected_track].step[i].rest = rest);
  }
}
