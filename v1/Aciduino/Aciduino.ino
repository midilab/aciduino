// Acid StepSequencer, a Roland TB303 step sequencer engine clone
// author: midilab contact@midilab.co
// under MIT license
#include <Arduino.h>

#include "config.h"

void setup() 
{
  // AcidStepSequencer Interface
  initAcidStepSequencer(MIDI_MODE);
  setTrackChannel(1, TRACK1_CHANNEL);
  setTrackChannel(2, TRACK2_CHANNEL);

  // pins, buttons, leds and pots config
  configureInterface();

  // last pattern user had load before power off
  loadLastPattern();
}

// User interaction goes here
void loop() 
{
  processInterface();
}
