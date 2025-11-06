/*!
 *  @file       AciduinoV2.ino
 *  Project     Aciduino V2
 *  @brief      Roland 303 and 808 step sequencer clone
 *  @version    2.2.0
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
#include <Arduino.h>

//
// Aciduino
//
#include "src/aciduino.hpp"

//
// Select your platform port
//
#include "src/ports/esp32/wroom.h"
//#include "src/ports/esp32/s3.h"
//#include "src/ports/avr/mega.h"
//#include "src/ports/teensy/protoboard.h"
//#include "src/ports/esp32/wroom-ext1.h"
//#include "src/ports/teensy/uone.h"
//#include "src/ports/avr/midilab_mega.h"
//#include "src/ports/avr/afourtrackmind_mega.h"

void setup() {
  // inits all hardware setup for the selected port
  initPort();
}

void loop() {
  aciduino.run();
}
