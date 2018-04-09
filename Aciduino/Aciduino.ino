// Acid StepSequencer, a Roland TB303 step sequencer engine clone
// author: midilab contact@midilab.co
// under MIT license
#include <Arduino.h>

void setup() 
{
  // AcidStepSequencer Interface
  initAcidStepSequencer(0, 1); // initing acid step sequencer in MIDI mode for channel 1

  // pins, buttons, leds and pots config
  configureInterface();
}

// User interaction goes here
void loop() 
{
  processInterface();
}
