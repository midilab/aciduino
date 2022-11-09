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

// build scale dorian as default on init

void HarmonizerClass::setTemperament(uint8_t temperament_id)
{
	uint8_t interval, interval_count, scale_pitch;
	uint8_t temperament = _harmony_mode_table[temperament_id].temperament;

	_temperament_id = temperament_id;
	// newpitch start point for analisys... on I interval of temperament. aka: Tonic
	scale_pitch = 0;
	interval = 0;

	// 8 bits scale temperament check, the last bit are discarted
	for (interval_count = 7; interval_count > 0; interval_count--) {

		// set scale pitch sequence based on interval
		_scale[interval] = scale_pitch;	
		
		if ( !(temperament & (1 << interval_count)) ) { // do we have half step setup inside bitmap? means 0
			scale_pitch += 1; // half-tone increment
		} else {
			scale_pitch += 2; // Whole-tone increment
		}	
		
		// increment the interval
		interval++;

	}    
}

const char * HarmonizerClass::getTemperamentName(uint8_t temperament_id)
{
  return _harmony_mode_table[temperament_id].name;
}

uint8_t HarmonizerClass::getTemperamentId()
{
  return _temperament_id;
}

uint8_t HarmonizerClass::harmonizer(uint8_t note)
{
  uint8_t octave, interval;

  octave = floor(note/12);
  interval = floor((note%12)/1.5);

  return _scale[interval] + (octave*12);
}

HarmonizerClass Harmonizer;