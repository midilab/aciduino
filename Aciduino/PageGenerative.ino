/*
[generative]
knobs: ramdon low note, ramdon high note, harmonic mode, tempo

buttons: track 1, track 2, ramdomize it, transpose -, transpose +, play/stop
*/

#define ACCENT_PROBABILITY_GENERATION   50
#define GLIDE_PROBABILITY_GENERATION    30
#define REST_PROBABILITY_GENERATION     10

uint8_t _lower_note = 36;
uint8_t _range_note = 34;
uint8_t _harmonic_mode = 0;

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
    // transpose -
    --_transpose;
    //reHarmonize(); // force all track lines to be re-harmonize
  }

  if ( pressed(GENERIC_BUTTON_5) ) {
    // transpose +
    ++_transpose;
    //reHarmonize(); // force all track lines to be re-harmonize
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
    
  if ( _transpose > 0 ) {
    digitalWrite(GENERIC_LED_4, LOW);
    digitalWrite(GENERIC_LED_5, HIGH);
  } else if ( _transpose < 0 ) {
    digitalWrite(GENERIC_LED_4, HIGH);
    digitalWrite(GENERIC_LED_5, LOW);
  } else {
    digitalWrite(GENERIC_LED_4, LOW);
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
  value = getPotChanges(GENERIC_POT_3, 0, 12); // bug, if we set 13 as maximun it goes beyond to 14... 
  if ( value != -1 ) {  
    _selected_mode = value;
    //reHarmonize(); // force all track lines to be re-harmonize
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
