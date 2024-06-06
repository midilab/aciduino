#include "../../uCtrl/uCtrl.h"
#include "../../aciduino.hpp"
#include "../../interface/raw/main.h"

// only 808 interface
// Start by configuring sequencer/setup.h
// 0x 303 and 1x 808 to get start
// 4 voices for 808
// global or per voice accent?

//============================================
// Managed Devices Setup
//============================================

// Midi device
// initing midi devices
MIDI_CREATE_INSTANCE(HardwareSerial, Serial, MIDI1);
MIDI_CREATE_INSTANCE(HardwareSerial, Serial3, MIDI2);
//MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI3);
//MIDI_CREATE_INSTANCE(HardwareSerial, Serial2, MIDI4);

void initPort()
{
  //
  // Storage setup (EPPROM)
  //
  uCtrl.initStorage();

  //
  // DIN Module
  //
  // initDin(spi_device, latch_pin)
  uCtrl.initDin(&SPI, 8);
  // how many 165 shiftregisters to connect?
  uCtrl.din->plugSR(12);

  // set callback for din data input
  uCtrl.din->setCallback(dinHandle);

  //
  // DOUT Module
  //
  //uCtrl.initDout();
  //uCtrl.dout->plugFastLed(&fastled);

  //
  // AIN Module
  //
  //uCtrl.initAin();
  //uCtrl.din->plug(/* uc_ain_pin */);
  // set callback for ain data input
  //uCtrl.ain->setCallback(ainHandle);

  //
  // MIDI Module
  //
  uCtrl.initMidi();
  uCtrl.midi->plug(&MIDI1);
  uCtrl.midi->plug(&MIDI2);
  //uCtrl.midi->plug(&MIDI3);
  //uCtrl.midi->plug(&MIDI4);

  // our main loop program callback
  uCtrl.setLoopCallback(mainLoop);

  // init uCtrl modules and memory
  uCtrl.init();

  // sequencer parameters could be initialized here?
  // how many 303 tracks? 808 tracks? 
  aciduino.init();
}
