
#define POT_NUMBER    4
#define BUTTON_NUMBER 6

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

void lockPotsState(bool lock)
{
  for ( uint8_t i = 0; i < POT_NUMBER; i++ ) {
    _pot[i].lock = lock;
  }
}

bool pressed(uint8_t button_pin)
{
  bool value;
  bool * last_value;

  switch(button_pin) {
    case PREVIOUS_STEP_BUTTON_PIN:
      last_value = &_button[0].state;
      break;
    case NEXT_STEP_BUTTON_PIN:
      last_value = &_button[1].state;
      break;
    case REST_BUTTON_PIN:
      last_value = &_button[2].state;
      break;   
    case GLIDE_BUTTON_PIN:
      last_value = &_button[3].state;
      break;
    case ACCENT_BUTTON_PIN:
      last_value = &_button[4].state;
      break;
    case PLAY_STOP_BUTTON_PIN:
      last_value = &_button[5].state;
      break;    
    default:
      return false;                    
  }
  
  value = digitalRead(button_pin);
  
  // check, using pullup pressed button goes LOW
  if ( value != *last_value && value == LOW ) {
    *last_value = value; 
    return true;    
  } else {
    *last_value = value; 
    return false;
  }
   
}

int16_t getPotChanges(uint8_t pot_pin, uint16_t min_value, uint16_t max_value)
{
  uint16_t value, value_ranged, last_value_ranged;
  uint16_t * last_value;
  bool * lock_pot;
  uint8_t pot_sensitivity = 1;

  switch(pot_pin) {
    case OCTAVE_POT_PIN:
      last_value = &_pot[0].state;
      lock_pot = &_pot[0].lock;
      break;
    case NOTE_POT_PIN:
      last_value = &_pot[1].state;
      lock_pot = &_pot[1].lock;
      break;
    case STEP_LENGTH_POT_PIN:
      last_value = &_pot[2].state;
      lock_pot = &_pot[2].lock;
      break;   
    case TEMPO_POT_PIN:
      last_value = &_pot[3].state;
      lock_pot = &_pot[3].lock;
      break;
    default:
      return -1;
  }

  // get absolute value
  value = analogRead(pot_pin);
    
  // range that value and our last_value
  value_ranged = (value / (1024 / ((max_value - min_value) + 1))) + min_value;
  last_value_ranged = (*last_value / (1024 / ((max_value - min_value) + 1))) + min_value; 

  // a lock system to not mess with some data(pots are terrible for some kinda of user interface data controls, but lets keep it low cost!)
  if ( *lock_pot == true ) {
      pot_sensitivity = LOCK_POT_SENSTIVITY;
  }
  
  if ( abs(value_ranged - last_value_ranged) >= pot_sensitivity ) {
    *last_value = value;
    if ( *lock_pot == true ) {
      *lock_pot = false;
    }
    return value_ranged;    
  } else {
    return -1;
  }  
}
