/*!
 *  @file       AciduinoV2.ino
 *  Project     Aciduino V2
 *  @brief      Roland 303 and 808 step sequencer clone
 *  @version    2.0.0
 *  @author     Romulo Silva
 *  @date       11/01/22
 *  @license    MIT - (c) 2022 - Romulo Silva - contact@midilab.co
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE. 
 */

/* 
 
  Aciduino v2
  
  4 tracks/2 engines: 303, 808

  track types:
  MIDI: 303, 808
  CV: 303(1cv 3gates), 808(4 gates)
  SAMPLER: 808
  SYNTH: 303
  
  load any engine on any track
  default: 303, 303, 808, 808

  shift+knob turn = last used midi control of the channel

  shift+ generic1/2 changes track...

  shift+ nav left/nav right changes subpage

  shift+ nav up/nav down changes page

  shift+ page button 2 play/stop

  shift+ page button 1 tempo setup


*/
//
// BPM Clock support
//
#include "src/uClock/src/uClock.h"

//
// Controlling interfaces support
//
#include "src/uCtrl/uCtrl.h"

//
// Acid step sequencer
//
#include "src/sequencer/acid_sequencer.h"

//
// Select your platform port
//
//#include "src/ports/avr/mega.h"
#include "src/ports/teensy/generic.h"
//#include "src/ports/esp32/wroom.h"
//#include "src/ports/teensy/uone.h"
//#include "src/ports/avr/midilab_mega.h"

// globals
bool _playing = false;
uint8_t _selected_track = 0;

void setup() {
  // setup uctrl hardware and control interfaces
  uCtrlSetup();
  // setup clock system
  uClockSetup();
  // init the sequencer
  initSequencer();
}

void loop() {
  // let uCtrl do his job
  uCtrl.run();
}
