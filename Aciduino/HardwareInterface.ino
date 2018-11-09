#include <EEPROM.h>

// pot data
typedef struct
{
  uint8_t pin;
  uint16_t state;
  bool lock;
} POT_DATA;

// button data
typedef struct
{
  uint8_t pin;
  bool state;
  uint8_t hold_seconds;
  bool hold_trigger;
} BUTTON_DATA;

POT_DATA _pot[POT_NUMBER];
BUTTON_DATA _button[BUTTON_NUMBER];

// pattern memory layout 72 bytes(2 tracks with 16 steps)
// byte1: pattern_exist
// byte2: _transpose
// byte3: _harmonize
// byte4: _selected_mode
// byte5...: _sequencer[0].data, _sequencer[1].data 
// EEPROM data access: 1024 bytes total
// total 14 patterns
// use for load and save pattern
#define PATTERN_SIZE (sizeof(SEQUENCER_TRACK_DATA)*TRACK_NUMBER)+4
#define PATTERN_NUMBER  14

SEQUENCER_TRACK_DATA _track_data;

void loadLastPattern()
{
  uint8_t last_pattern;

  EEPROM.get(1023, last_pattern);

  if ( last_pattern < PATTERN_NUMBER ) {
    loadPattern(last_pattern);
  }
}

void resetPattern(uint8_t number)
{
  uint16_t eeprom_address = PATTERN_SIZE;

  // get address base for pattern
  eeprom_address *= number;

  // load defaults
  ATOMIC(
    _transpose = 0;
    _harmonize = 0;
    _selected_mode = 0;
  );
  // reset sequencer data
  for ( uint8_t track = 0; track < TRACK_NUMBER; track++ ) {
    ATOMIC(_sequencer[track].mute = true);
    clearStackNote(track);    
    _track_data.step_init_point = 0;
    _track_data.step_length = STEP_MAX_SIZE;
    // initing note data
    for ( uint8_t i = 0; i < STEP_MAX_SIZE; i++ ) {
      _track_data.step[i].note = 48;
      _track_data.step[i].accent = 0;
      _track_data.step[i].glide = 0;
      _track_data.step[i].rest = 0;
    }

    ATOMIC(memcpy((void*)&_sequencer[track].data, &_track_data, sizeof(SEQUENCER_TRACK_DATA)));
    ATOMIC(_sequencer[track].mute = false);
  }  
  // mark pattern slot as not in use 
  EEPROM.write(eeprom_address, 0);
}

void loadPattern(uint8_t number) 
{
  uint16_t eeprom_address = PATTERN_SIZE;
  uint8_t pattern_exist, harmonize, selected_mode = 0;
  int8_t transpose = 0;
  
  if ( number >= PATTERN_NUMBER ) {
    return;
  }

  // get address base for pattern
  eeprom_address *= number;
  
  // do we have pattern data to read it here?
  EEPROM.get(eeprom_address, pattern_exist);
  if ( pattern_exist != 1 ) {
    resetPattern(number);
    return;
  }

  // global pattern config
  EEPROM.get(++eeprom_address, transpose);
  EEPROM.get(++eeprom_address, harmonize);
  EEPROM.get(++eeprom_address, selected_mode);
  // constrains to avoid trash data from memory
  if ( transpose > 12 ) {
    transpose = 12;
  } else if ( transpose < -12 ) {
    transpose = -12;
  }
  if (selected_mode >= MODES_NUMBER) {
    selected_mode = MODES_NUMBER-1;
  }
  ATOMIC(
    _transpose = transpose;
    _harmonize = harmonize;
    _selected_mode = selected_mode;
  );
  // track data
  for (uint8_t track=0; track < TRACK_NUMBER; track++) {
    ATOMIC(_sequencer[track].mute = true);
    clearStackNote(track);    
    EEPROM.get(++eeprom_address, _track_data); // 34 bytes long
    // constrains to avoid trash data from memory
    if ( _track_data.step_length > STEP_MAX_SIZE ) {
      _track_data.step_length = STEP_MAX_SIZE;
    }
    ATOMIC(memcpy((void*)&_sequencer[track].data, &_track_data, sizeof(SEQUENCER_TRACK_DATA)));
    ATOMIC(_sequencer[track].mute = false);
    eeprom_address += (sizeof(SEQUENCER_TRACK_DATA)-1);
  }

  // save last pattern loaded
  EEPROM.write(1023, number);

  _selected_pattern = number;
  
}

void savePattern(uint8_t number) 
{
  uint16_t eeprom_address = PATTERN_SIZE;
  
  if ( number >= PATTERN_NUMBER ) {
    return;
  }

  // get address base for pattern
  eeprom_address *= number;
  
  // mark pattern slot as in use pattern_exist
  EEPROM.write(eeprom_address, 1);
  // global pattern config
  EEPROM.write(++eeprom_address, _transpose);
  EEPROM.write(++eeprom_address, _harmonize);
  EEPROM.write(++eeprom_address, _selected_mode);    
  // track data
  for (uint8_t track=0; track < TRACK_NUMBER; track++) {
    memcpy(&_track_data, (void*)&_sequencer[track].data, sizeof(SEQUENCER_TRACK_DATA));
    EEPROM.put(++eeprom_address, _track_data); // 34 bytes long
    eeprom_address += (sizeof(SEQUENCER_TRACK_DATA)-1);
  }
  
}

void connectPot(uint8_t pot_id, uint8_t pot_pin)
{
  _pot[pot_id].pin = pot_pin;
  // get first state data
  _pot[pot_id].state = analogRead(_pot[pot_id].pin);
  _pot[pot_id].lock = true;
}

void connectButton(uint8_t button_id, uint8_t button_pin)
{
  _button[button_id].pin = button_pin;
  // use internal pullup for buttons
  pinMode(_button[button_id].pin, INPUT_PULLUP);
  // get first state data
  _button[button_id].state = digitalRead(_pot[button_id].pin);
  _button[button_id].hold_seconds = 0;
  _button[button_id].hold_trigger = false;
}
 
void lockPotsState(bool lock)
{
  for ( uint8_t i = 0; i < POT_NUMBER; i++ ) {
    _pot[i].lock = lock;
  }
}

bool pressed(uint8_t button_id)
{
  bool value;
  
  value = digitalRead(_button[button_id].pin);
  
  // using internal pullup pressed button goes LOW
  if ( value != _button[button_id].state && value == LOW ) {
    _button[button_id].state = value; 
    return true;    
  } else {
    _button[button_id].state = value; 
    return false;
  }
}

bool doublePressed(uint8_t button1_id, uint8_t button2_id)
{
  bool value1, value2;
  
  value1 = digitalRead(_button[button1_id].pin);
  value2 = digitalRead(_button[button2_id].pin);
  
  // using internal pullup pressed button goes LOW
  if ( value1 == LOW && value2 == LOW ) {
    _button[button1_id].state = LOW; 
    _button[button2_id].state = LOW;
    return true;    
  } else {
    return false;
  }
}

bool holded(uint8_t button_id, uint8_t seconds)
{
  bool value;
  
  value = digitalRead(_button[button_id].pin);
  
  // using internal pullup pressed button goes LOW
  if ( _button[button_id].hold_trigger == false && value == LOW ) {
    if ( _button[button_id].hold_seconds == 0 ) {
      _button[button_id].hold_seconds = millis()/1000;
    } else if ( (millis()/1000) >= _button[button_id].hold_seconds+seconds ) {
      _button[button_id].hold_trigger = true; // avoid released triger after action.
      return true;    
    }
    return false;
  } else if ( value == HIGH ) {
    _button[button_id].hold_trigger = false;
    _button[button_id].hold_seconds = 0;
    return false;
  }
}

bool released(uint8_t button_id)
{
  bool value;
  
  value = digitalRead(_button[button_id].pin);
  
  // using internal pullup released button goes HIGH
  if ( value != _button[button_id].state && value == HIGH && _button[button_id].hold_trigger == false ) {
    _button[button_id].state = value; 
    return true;    
  } else {
    _button[button_id].state = value; 
    return false;
  }
}

int16_t getPotChanges(uint8_t pot_id, uint16_t min_value, uint16_t max_value)
{
  uint16_t value, value_ranged, last_value_ranged;
  uint8_t pot_sensitivity = POT_SENSITIVITY;

  // get absolute value
  value = analogRead(_pot[pot_id].pin);
    
  // range that value and our last_value
  value_ranged = (value / (ADC_RESOLUTION / ((max_value - min_value) + 1))) + min_value;
  last_value_ranged = (_pot[pot_id].state / (ADC_RESOLUTION / ((max_value - min_value) + 1))) + min_value; 

  // a lock system to not mess with some data(pots are terrible for some kinda of user interface data controls, but lets keep it low cost!)
  if ( _pot[pot_id].lock == true ) {
    // user needs to move 1/8 of total adc range to get pot unlocked
    if ( abs(value - _pot[pot_id].state) < (ADC_RESOLUTION/8) ) {
      return -1;
    }
  }
  
  if ( abs(value_ranged - last_value_ranged) >= pot_sensitivity ) {
    _pot[pot_id].state = value;
    if ( _pot[pot_id].lock == true ) {
      _pot[pot_id].lock = false;
    }
    if ( value_ranged > max_value ) {
      value_ranged = max_value;
    }
    return value_ranged;    
  } else {
    return -1;
  }  
}
