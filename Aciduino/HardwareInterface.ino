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
} BUTTON_DATA;

POT_DATA _pot[POT_NUMBER];
BUTTON_DATA _button[BUTTON_NUMBER];

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

bool released(uint8_t button_id)
{
  bool value;
  
  value = digitalRead(_button[button_id].pin);
  
  // using internal pullup released button goes HIGH
  if ( value != _button[button_id].state && value == HIGH ) {
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
  value_ranged = (value / (1024 / ((max_value - min_value) + 1))) + min_value;
  last_value_ranged = (_pot[pot_id].state / (1024 / ((max_value - min_value) + 1))) + min_value; 

  // a lock system to not mess with some data(pots are terrible for some kinda of user interface data controls, but lets keep it low cost!)
  if ( _pot[pot_id].lock == true ) {
    // user needs to move 1/4 of total range to get pot unlocked
    pot_sensitivity = abs(max_value - min_value)/4;
  }
  
  if ( abs(value_ranged - last_value_ranged) >= pot_sensitivity ) {
    _pot[pot_id].state = value;
    if ( _pot[pot_id].lock == true ) {
      _pot[pot_id].lock = false;
    }
    return value_ranged;    
  } else {
    return -1;
  }  
}
