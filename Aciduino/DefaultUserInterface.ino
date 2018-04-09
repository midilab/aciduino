// User interface config
#define SEQUENCER_MIN_BPM     50
#define SEQUENCER_MAX_BPM     177

// Hardware pin configuration(double check your schematic before configure those pins)
// Pots
#define GENERIC_POT_1_PIN     A3
#define GENERIC_POT_2_PIN     A2
#define GENERIC_POT_3_PIN     A1
#define GENERIC_POT_4_PIN     A0
// Buttons
#define GENERIC_BUTTON_1_PIN  2
#define GENERIC_BUTTON_2_PIN  3
#define GENERIC_BUTTON_3_PIN  4
#define GENERIC_BUTTON_4_PIN  5
#define GENERIC_BUTTON_5_PIN  6
#define GENERIC_BUTTON_6_PIN  7
// Leds
#define GENERIC_LED_1         8
#define GENERIC_LED_2         9
#define GENERIC_LED_3         10
#define GENERIC_LED_4         11
#define GENERIC_LED_5         12
#define GENERIC_LED_6         13

typedef enum {
  GENERIC_POT_1,
  GENERIC_POT_2,
  GENERIC_POT_3,
  GENERIC_POT_4  
} POT_HARDWARE_INTERFACE;

typedef enum {
  GENERIC_BUTTON_1,
  GENERIC_BUTTON_2,
  GENERIC_BUTTON_3,
  GENERIC_BUTTON_4,
  GENERIC_BUTTON_5,
  GENERIC_BUTTON_6
} BUTTON_HARDWARE_INTERFACE;

// User Interface data
uint16_t _step_edit = 0;
uint8_t _last_octave = 3;
uint8_t _last_note = 0;

uint8_t _bpm_blink_timer = 1;

void configureInterface()
{
  // Buttons config
  connectButton(GENERIC_BUTTON_1, GENERIC_BUTTON_1_PIN);
  connectButton(GENERIC_BUTTON_2, GENERIC_BUTTON_2_PIN);
  connectButton(GENERIC_BUTTON_3, GENERIC_BUTTON_3_PIN);
  connectButton(GENERIC_BUTTON_4, GENERIC_BUTTON_4_PIN);
  connectButton(GENERIC_BUTTON_5, GENERIC_BUTTON_5_PIN);
  connectButton(GENERIC_BUTTON_6, GENERIC_BUTTON_6_PIN);  

  // Pots config
  connectPot(GENERIC_POT_1, GENERIC_POT_1_PIN);
  connectPot(GENERIC_POT_2, GENERIC_POT_2_PIN);
  connectPot(GENERIC_POT_3, GENERIC_POT_3_PIN);
  connectPot(GENERIC_POT_4, GENERIC_POT_4_PIN);

  // Leds config
  pinMode(GENERIC_LED_1, OUTPUT);
  pinMode(GENERIC_LED_2, OUTPUT);
  pinMode(GENERIC_LED_3, OUTPUT);
  pinMode(GENERIC_LED_4, OUTPUT);
  pinMode(GENERIC_LED_5, OUTPUT);
  pinMode(GENERIC_LED_6, OUTPUT);
  digitalWrite(GENERIC_LED_1, LOW);
  digitalWrite(GENERIC_LED_2, LOW);
  digitalWrite(GENERIC_LED_3, LOW);
  digitalWrite(GENERIC_LED_4, LOW);
  digitalWrite(GENERIC_LED_5, LOW);
  digitalWrite(GENERIC_LED_6, LOW);  
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
    digitalWrite(GENERIC_LED_6 , HIGH);
  } else if ( !(*tick % (24)) ) {   // each quarter led on
    digitalWrite(GENERIC_LED_6 , HIGH);
  } else if ( !(*tick % _bpm_blink_timer) ) { // get led off
    digitalWrite(GENERIC_LED_6 , LOW);
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
    ATOMIC(_sequencer[_step_edit].note = (_last_octave * 8) + _last_note);
    //ATOMIC(_sequencer[_step_edit].note = harmonizer((_last_octave * 8) + _last_note));
    if ( _playing == false && _sequencer[_step_edit].rest == false ) {
      sendPreviewNote(_step_edit);
    }
  }

  // GENERIC_POT_3: Sequencer step length
  step_length = getPotChanges(GENERIC_POT_3, 1, STEP_MAX_SIZE);
  if ( step_length != -1 ) {  
    ATOMIC(_step_length = step_length);
    if ( _step_edit >= _step_length ) {
      _step_edit = _step_length-1;
    }
  }

  // GENERIC_POT_4: Sequencer BPM Tempo from SEQUENCER_MIN_BPM to SEQUENCER_MAX_BPM
  tempo = getPotChanges(GENERIC_POT_4, SEQUENCER_MIN_BPM, SEQUENCER_MAX_BPM);
  if ( tempo != -1 ) {   
    uClock.setTempo(tempo);
  }
}

void processButtons()
{
  // play/stop
  if ( pressed(GENERIC_BUTTON_6) ) {
    if ( _playing == false ) {
      // Starts the clock, tick-tac-tick-tac...
      uClock.start();
    } else {
      // stop the clock
      uClock.stop();
    }
  }

  // ramdom test
  //if ( pressed(GENERIC_BUTTON_1) && pressed(GENERIC_BUTTON_2) ) {
    //acidRandomize();
    //return;
  //}

  // previous step edit
  if ( pressed(GENERIC_BUTTON_1) ) {
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
  if ( pressed(GENERIC_BUTTON_2) ) {
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
  if ( pressed(GENERIC_BUTTON_3) ) {
    ATOMIC(_sequencer[_step_edit].rest = !_sequencer[_step_edit].rest);
    if ( _playing == false && _sequencer[_step_edit].rest == false ) {
      sendPreviewNote(_step_edit);
    }
  }

  // step glide
  if ( pressed(GENERIC_BUTTON_4) ) {
    ATOMIC(_sequencer[_step_edit].glide = !_sequencer[_step_edit].glide);
  }

  // step accent
  if ( pressed(GENERIC_BUTTON_5) ) {
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
    digitalWrite(GENERIC_LED_1 , HIGH);
  } else {
    digitalWrite(GENERIC_LED_1 , LOW);
  }  

  // Editing Last Step? 
  if ( _step_edit == _step_length-1 ) {
    digitalWrite(GENERIC_LED_2 , HIGH);
  } else {
    digitalWrite(GENERIC_LED_2 , LOW);
  }  
  
  // Rest 
  if ( _sequencer[_step_edit].rest == true ) {
    digitalWrite(GENERIC_LED_3 , HIGH);
  } else {
    digitalWrite(GENERIC_LED_3 , LOW);
  }

  // Glide 
  if ( _sequencer[_step_edit].glide == true ) {
    digitalWrite(GENERIC_LED_4 , HIGH);
  } else {
    digitalWrite(GENERIC_LED_4 , LOW);
  }  

  // Accent 
  if ( _sequencer[_step_edit].accent == true ) {
    digitalWrite(GENERIC_LED_5 , HIGH);
  } else {
    digitalWrite(GENERIC_LED_5 , LOW);
  } 

  // shut down play led if we are stoped
  if ( _playing == false ) {
    digitalWrite(GENERIC_LED_6 , LOW);
  }
}

// mixolidian
uint8_t _scale[8] = { 0, 2, 4, 5, 7, 9, 10, 12 };
// phrygian
//uint8_t _scale[8] = { 0, 1, 3, 5, 7, 8, 10, 12 };

uint8_t harmonizer(uint8_t note)
{
  uint8_t octave, note_octave;
  
  octave = floor(note/12);
  note_octave = note%12;
  return (octave*12) + _scale[note_octave];
}

void acidRandomize() 
{
  // ramdom it all
  for ( uint16_t i = 0; i < STEP_MAX_SIZE; i++ ) {
    ATOMIC(_sequencer[i].note = random(36, 70)); // octave 2 to 4. octave 3 to 5 (40 - 83)
    //ATOMIC(_sequencer[i].note = harmonizer(random(36, 70)));
    ATOMIC(_sequencer[i].accent = random(0, 2));
    ATOMIC(_sequencer[i].glide = random(0, 2));
    ATOMIC(_sequencer[i].rest = random(0, 1));
  }
}
