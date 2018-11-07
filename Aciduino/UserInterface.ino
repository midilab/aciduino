/*
[page select]: press button1 and button2 together
knobs: none, none, none, none

buttons: track 1, track 2, [live mode], [generative], [step edit], play/stop
*/

uint32_t _page_blink_timer = 0; 
uint8_t _bpm_blink_timer = 1;
uint8_t _selected_page = 0;

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

  // first read to fill our registers
  getPotChanges(GENERIC_POT_1, 0, ADC_RESOLUTION);
  getPotChanges(GENERIC_POT_2, 0, ADC_RESOLUTION);
  getPotChanges(GENERIC_POT_3, 0, ADC_RESOLUTION);
  getPotChanges(GENERIC_POT_4, 0, ADC_RESOLUTION);
}

void processInterface()
{
  static int16_t tempo;
  
  // global controllers play and tempo
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

  // shut down play led if we are stoped
  if ( _playing == false ) {
    digitalWrite(GENERIC_LED_6 , LOW);
  }

  // page select request
  if ( doublePressed(GENERIC_BUTTON_1, GENERIC_BUTTON_2) ) {
    _selected_page = 0;
  }

  switch ( _selected_page ) {

    // Select Track/Page
    case 0:
      processPageButtons();
      processPageLeds();
      break;
      
    // Midi controller  
    case 1:
      processControllerButtons();
      processControllerLeds();
      processControllerPots();  
      break;
      
    // Generative  
    case 2:
      processGenerativeButtons();
      processGenerativeLeds();
      processGenerativePots();  
      break;
      
    // Sequencer
    case 3:
      processSequencerButtons();
      processSequencerLeds();
      processSequencerPots();  
      break;        
  }

}

void processPageButtons()
{

  if ( pressed(GENERIC_BUTTON_1) ) {
    _selected_track = 0;
  }

  if ( pressed(GENERIC_BUTTON_2) ) {
    _selected_track = 1;
  }

  if ( pressed(GENERIC_BUTTON_3) ) {
    lockPotsState(true);
    _selected_page = 1;
  }

  if ( pressed(GENERIC_BUTTON_4) ) {
    lockPotsState(true);
    _selected_page = 2;
  }

  if ( pressed(GENERIC_BUTTON_5) ) {
    lockPotsState(true);
    _selected_page = 3;
  }
  
}

void processPageLeds()
{
  static bool blink_state = true;
  
  // blink interface here for button 3 to 5
  if ( millis() - _page_blink_timer >= 300 ) {
    blink_state = !blink_state;
    _page_blink_timer = millis();
  }

  digitalWrite(GENERIC_LED_3, blink_state);
  digitalWrite(GENERIC_LED_4, blink_state);
  digitalWrite(GENERIC_LED_5, blink_state);

  if ( _selected_track == 0 ) {
    digitalWrite(GENERIC_LED_1, HIGH);
    digitalWrite(GENERIC_LED_2, LOW);
  } else if ( _selected_track == 1 ) {
    digitalWrite(GENERIC_LED_1, LOW);
    digitalWrite(GENERIC_LED_2, HIGH);
  } 
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
