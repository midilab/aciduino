// Acid StepSequencer, a Roland TB303 step sequencer engine clone
// author: midilab contact@midilab.co
// under MIT license
#include <Arduino.h>

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

void setup() 
{
  // AcidStepSequencer Interface
  initAcidStepSequencer(0); // initing acid step sequencer in MIDI mode
  //setTrackChannel(1, 1);
  //setTrackChannel(2, 2);

  // pins, buttons, leds and pots config
  configureInterface();
}

// User interaction goes here
void loop() 
{
  processInterface();
}
