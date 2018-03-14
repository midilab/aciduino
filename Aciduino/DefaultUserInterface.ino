
#define SEQUENCER_MIN_BPM  50
#define SEQUENCER_MAX_BPM  177

// Ui config
#define LOCK_POT_SENSTIVITY 3

// hardware setup to fit different kinda of setups and arduino models
#define OCTAVE_POT_PIN            A3
#define NOTE_POT_PIN              A2
#define STEP_LENGTH_POT_PIN       A1
#define TEMPO_POT_PIN             A0

#define PREVIOUS_STEP_BUTTON_PIN  2
#define NEXT_STEP_BUTTON_PIN      3
#define REST_BUTTON_PIN           4
#define GLIDE_BUTTON_PIN          5
#define ACCENT_BUTTON_PIN         6
#define PLAY_STOP_BUTTON_PIN      7

#define PREVIOUS_STEP_LED_PIN     8
#define NEXT_STEP_LED_PIN         9
#define REST_LED_PIN              10
#define GLIDE_LED_PIN             11
#define ACCENT_LED_PIN            12
#define PLAY_STOP_LED_PIN         13

// User Interface data
uint16_t _step_edit = 0;
uint8_t _last_octave = 3;
uint8_t _last_note = 0;

uint8_t _bpm_blink_timer = 1;

void configureInterface()
{
  // Buttons config
  // use internal pullup for buttons
  pinMode(PREVIOUS_STEP_BUTTON_PIN, INPUT_PULLUP);
  pinMode(NEXT_STEP_BUTTON_PIN, INPUT_PULLUP);
  pinMode(REST_BUTTON_PIN, INPUT_PULLUP);
  pinMode(GLIDE_BUTTON_PIN, INPUT_PULLUP);
  pinMode(ACCENT_BUTTON_PIN, INPUT_PULLUP);
  pinMode(PLAY_STOP_BUTTON_PIN, INPUT_PULLUP);

  // Leds config
  pinMode(PREVIOUS_STEP_LED_PIN, OUTPUT);
  pinMode(NEXT_STEP_LED_PIN, OUTPUT);
  pinMode(REST_LED_PIN, OUTPUT);
  pinMode(GLIDE_LED_PIN, OUTPUT);
  pinMode(ACCENT_LED_PIN, OUTPUT);
  pinMode(PLAY_STOP_LED_PIN, OUTPUT);

  digitalWrite(PREVIOUS_STEP_LED_PIN, LOW);
  digitalWrite(NEXT_STEP_LED_PIN, LOW);
  digitalWrite(REST_LED_PIN, LOW);
  digitalWrite(GLIDE_LED_PIN, LOW);
  digitalWrite(ACCENT_LED_PIN, LOW);
  digitalWrite(PLAY_STOP_LED_PIN, LOW);  

  // getting first value state
  pressed(PREVIOUS_STEP_BUTTON_PIN);
  pressed(NEXT_STEP_BUTTON_PIN);
  pressed(REST_BUTTON_PIN);
  pressed(GLIDE_BUTTON_PIN);
  pressed(ACCENT_BUTTON_PIN);
  pressed(PLAY_STOP_BUTTON_PIN);

  // getting first values
  getPotChanges(OCTAVE_POT_PIN, 0, 10);
  getPotChanges(NOTE_POT_PIN, 0, 11);
  getPotChanges(STEP_LENGTH_POT_PIN, 1, STEP_MAX_SIZE);
  getPotChanges(TEMPO_POT_PIN, SEQUENCER_MIN_BPM, SEQUENCER_MAX_BPM);
  
  lockPotsState(true);

  //acidRandomize();
}

void processInterface()
{
  processButtons();
  processLeds();
  processPots();  
}

void tempoInterface(uint32_t * tick) 
{
  // BPM led indicator
  if ( !(*tick % (96)) || (*tick == 0) ) {  // first compass step will flash longer
    _bpm_blink_timer = 8;
    digitalWrite(PLAY_STOP_LED_PIN , HIGH);
  } else if ( !(*tick % (24)) ) {   // each quarter led on
    digitalWrite(PLAY_STOP_LED_PIN , HIGH);
  } else if ( !(*tick % _bpm_blink_timer) ) { // get led off
    digitalWrite(PLAY_STOP_LED_PIN , LOW);
    _bpm_blink_timer = 1;
  }
}

void sendPreviewNote(uint16_t step)
{
  unsigned long milliTime, preMilliTime;
  
  sendMidiMessage(NOTE_ON, _sequencer[step].note, _sequencer[step].accent ? ACCENT_VELOCITY : NOTE_VELOCITY);

  // avoid delay() call because of uClock timmer1 usage
  //delay(200);
  preMilliTime = millis();
  while ( true ) {
    milliTime = millis();
    if (abs(milliTime - preMilliTime) >= 200) {
      break;
    }
  }
  
  sendMidiMessage(NOTE_OFF, _sequencer[step].note, 0);
}

void processPots()
{
  static int8_t octave, note, step_note;
  static int16_t tempo, step_length;

  octave = getPotChanges(OCTAVE_POT_PIN, 0, 10);
  if ( octave != -1 ) {  
    _last_octave = octave;
  }

  note = getPotChanges(NOTE_POT_PIN, 0, 11);
  if ( note != -1 ) { 
    _last_note = note;
  }

  // changes on octave or note pot?
  if ( octave != -1 || note != -1 ) {
    ATOMIC(_sequencer[_step_edit].note = (_last_octave * 8) + _last_note);
    if ( _playing == false && _sequencer[_step_edit].rest == false ) {
      sendPreviewNote(_step_edit);
    }
  }

  step_length = getPotChanges(STEP_LENGTH_POT_PIN, 1, STEP_MAX_SIZE);
  if ( step_length != -1 ) {  
    ATOMIC(_step_length = step_length);
    if ( _step_edit >= _step_length ) {
      _step_edit = _step_length-1;
    }
  }

  tempo = getPotChanges(TEMPO_POT_PIN, SEQUENCER_MIN_BPM, SEQUENCER_MAX_BPM);
  if ( tempo != -1 ) {   
    //uClock.setTempo(tempo);
  }
}
  
void processButtons()
{
  // play/stop
  if ( pressed(PLAY_STOP_BUTTON_PIN) ) {
    if ( _playing == false ) {
      // Starts the clock, tick-tac-tick-tac...
      uClock.start();
    } else {
      // stop the clock
      uClock.stop();
    }
  }

  // ramdom test
  //if ( pressed(PREVIOUS_STEP_BUTTON_PIN) && pressed(NEXT_STEP_BUTTON_PIN) ) {
    //acidRandomize();
    //return;
  //}

  // previous step edit
  if ( pressed(PREVIOUS_STEP_BUTTON_PIN) ) {
    if ( _step_edit != 0 ) {
      // add a lock here for octave and note to not mess with edit mode when moving steps around 
      lockPotsState(true);   
      --_step_edit;
    } else { // TODO: just for tests.. take this guy off here and put it on second page
      acidRandomize();
    }
    if ( _playing == false && _sequencer[_step_edit].rest == false ) {
      sendPreviewNote(_step_edit);
    }
  }

  // next step edit
  if ( pressed(NEXT_STEP_BUTTON_PIN) ) {
    if ( _step_edit < _step_length-1 ) {
      // add a lock here for octave and note to not mess with edit mode when moving steps around
      lockPotsState(true);     
      ++_step_edit;
    }
    if ( _playing == false && _sequencer[_step_edit].rest == false ) {
      sendPreviewNote(_step_edit);
    }    
  }

  // step rest
  if ( pressed(REST_BUTTON_PIN) ) {
    ATOMIC(_sequencer[_step_edit].rest = !_sequencer[_step_edit].rest);
    if ( _playing == false && _sequencer[_step_edit].rest == false ) {
      sendPreviewNote(_step_edit);
    }
  }

  // step glide
  if ( pressed(GLIDE_BUTTON_PIN) ) {
    ATOMIC(_sequencer[_step_edit].glide = !_sequencer[_step_edit].glide);
  }

  // step accent
  if ( pressed(ACCENT_BUTTON_PIN) ) {
    ATOMIC(_sequencer[_step_edit].accent = !_sequencer[_step_edit].accent);
    if ( _playing == false && _sequencer[_step_edit].rest == false ) {
      sendPreviewNote(_step_edit);
    }       
  }     
}
  
void processLeds()
{   
  // Editing First Step? 
  if ( _step_edit == 0 ) {
    digitalWrite(PREVIOUS_STEP_LED_PIN , HIGH);
  } else {
    digitalWrite(PREVIOUS_STEP_LED_PIN , LOW);
  }  

  // Editing Last Step? 
  if ( _step_edit == _step_length-1 ) {
    digitalWrite(NEXT_STEP_LED_PIN , HIGH);
  } else {
    digitalWrite(NEXT_STEP_LED_PIN , LOW);
  }  
  
  // Rest 
  if ( _sequencer[_step_edit].rest == true ) {
    digitalWrite(REST_LED_PIN , HIGH);
  } else {
    digitalWrite(REST_LED_PIN , LOW);
  }

  // Glide 
  if ( _sequencer[_step_edit].glide == true ) {
    digitalWrite(GLIDE_LED_PIN , HIGH);
  } else {
    digitalWrite(GLIDE_LED_PIN , LOW);
  }  

  // Accent 
  if ( _sequencer[_step_edit].accent == true ) {
    digitalWrite(ACCENT_LED_PIN , HIGH);
  } else {
    digitalWrite(ACCENT_LED_PIN , LOW);
  } 

  // shut down play led if we are stoped
  if ( _playing == false ) {
    digitalWrite(PLAY_STOP_LED_PIN , LOW);
  }
}

void acidRandomize() 
{
  // ramdom it all
  for ( uint16_t i = 0; i < STEP_MAX_SIZE; i++ ) {
    ATOMIC(_sequencer[i].note = random(36, 70)); // octave 2 to 4. octave 3 to 5 (40 - 83)
    ATOMIC(_sequencer[i].accent = random(0, 2));
    ATOMIC(_sequencer[i].glide = random(0, 2));
    ATOMIC(_sequencer[i].rest = random(0, 1));
  }
}
