/*!
 *  @file       harmonizer.cpp
 *  Project     Acid Sequencer
 *  @brief      Step Sequencer harmonizer for melodic lines
 *  @version    0.8.0
 *  @author     Romulo Silva
 *  @date       22/09/2022
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

#include "harmonizer.h"

uint8_t HarmonizerClass::harmonizer(uint8_t note)
{
    uint8_t octave, interval;

    octave = floor(note/12);
    interval = floor((note%12)/1.5);

    return _scale[interval] + (octave*12);
}

// limit the note range within the _number_of_tones
// if 1 tone: get the root
// instead of number of tones why not intervals?
// TODO
uint8_t HarmonizerClass::getNoteByMaxNumOfTones(uint8_t note)
{
  uint8_t octave, relative_note;

  octave = note/12;
  //relative_note = note%12;
  //return _allowed_tones[relative_note] + (octave*12);
  relative_note = floor((note%12)/1.5);
  return _scale[relative_note] + (octave*12);
  // TODO: we can use _bjorklund to compute the fill of 8bytes!!!! and a seed to vary it also
  // the seed moves it from interval I(root) along to VII
}

HarmonizerClass Harmonizer;