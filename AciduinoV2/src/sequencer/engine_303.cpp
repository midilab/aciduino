/*!
 *  @file       engine_303.cpp
 *  Project     Acid Sequencer
 *  @brief      Step Sequencer engine that emulates TB 303
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
 
#include "engine_303.h"

void Engine303::setTrackChannel(uint8_t track, uint8_t channel)
{
  ATOMIC(_sequencer[track].channel = channel);
}

void Engine303::init()
{
  // initing sequencer memory data
  for ( uint8_t track = 0; track < TRACK_NUMBER_303; track++ ) {

    _sequencer[track].channel = track;
    _sequencer[track].data.shift = 0;
    _sequencer[track].data.step_length = STEP_MAX_SIZE_303;
    _sequencer[track].data.transpose = 0;
    _sequencer[track].step_location = 0;
    _sequencer[track].mute = false;

    // initing note data
    for ( uint16_t i = 0; i < STEP_MAX_SIZE_303; i++ ) {
      _sequencer[track].data.step[i].note = 36;
      _sequencer[track].data.step[i].accent = 0;
      _sequencer[track].data.step[i].slide = 0;
      _sequencer[track].data.step[i].rest = 0;
      //_sequencer[track].data.step[i].rest = (i+2) % 4 == 0 ? 0 : 1;
      //_sequencer[track].data.step[i].rest = i % 4 == 0 ? 0 : 1;
    }
  
    // initing note stack data
    for ( uint8_t i = 0; i < NOTE_STACK_SIZE_303; i++ ) {
      _sequencer[track].stack[i].note = 0;
      _sequencer[track].stack[i].length = -1;
    }
    
  }
}

void Engine303::rest(uint8_t track, uint8_t step, bool state) 
{
    ATOMIC(_sequencer[track].data.step[step].rest = state);
}

void Engine303::setAccent(uint8_t track, uint8_t step, bool state) 
{
    ATOMIC(_sequencer[track].data.step[step].accent = state);
}

void Engine303::setSlide(uint8_t track, uint8_t step, bool state) 
{
    ATOMIC(_sequencer[track].data.step[step].slide = state);
}

bool Engine303::stepOn(uint8_t track, uint8_t step)
{
    return !_sequencer[track].data.step[step].rest;
}

bool Engine303::accentOn(uint8_t track, uint8_t step)
{
    return _sequencer[track].data.step[step].accent;
}

bool Engine303::slideOn(uint8_t track, uint8_t step)
{
    return _sequencer[track].data.step[step].slide;
}

void Engine303::setStepData(uint8_t track, uint8_t step, uint8_t data)
{
  ATOMIC(_sequencer[track].data.step[step].note = data);
}

uint8_t Engine303::getStepData(uint8_t track, uint8_t step)
{
  return _sequencer[track].data.step[step].note;
}

uint8_t Engine303::getCurrentStep(uint8_t track)
{
    static uint8_t step;
    ATOMIC(step = _sequencer[track].step_location) 
    return step;
}

uint8_t Engine303::getTrackLength(uint8_t track)
{
    static uint8_t length;
    ATOMIC(length = _sequencer[track].data.step_length) 
    return length;
}

void Engine303::setTrackLength(uint8_t track, uint16_t length)
{
  ATOMIC(_sequencer[track].data.step_length = length);  
}

void Engine303::acidRandomize(uint8_t track) 
{
  uint8_t note, high_note, accent, slide, rest;
  //uint64_t euclidian;

  // clear track before random data or only clear stack note?
  // probably clear stack note is a better idea

      /*
		if ( track_fill <= 7 && _seq.track[track_number].pattern.type != 0 ) { // TODO: good idea to handle the euclidian track_fill 1 just in case
			// set to zero, means auto progression handler
			euclidian = 0ULL;
		} else {	
			// Calculate the euclidian
			euclidian = _bjorklund.compute(_track_step_size, (track_fill-7));
		}
    */

  // ramdom it all
  ATOMIC(_sequencer[track].mute = true);
  clearStackNote(track);
  for ( uint16_t i = 0; i < STEP_MAX_SIZE_303; i++ ) {
    high_note = _lower_note+_range_note;
    if ( high_note > 127 ) {
      high_note = 127;
    }

    note = Harmonizer.getNoteByMaxNumOfTones(random(_lower_note, high_note)) + _lower_note;
    accent = random(0, 100) < _accent_probability ? 1 : 0;
    slide = random(0, 100) < _slide_probability ? 1 : 0;
    rest = random(0, 100) < _rest_probability ? 1 : 0;
    
    ATOMIC(_sequencer[track].data.step[i].note = note);
    ATOMIC(_sequencer[track].data.step[i].accent = accent);
    ATOMIC(_sequencer[track].data.step[i].slide = slide);
    ATOMIC(_sequencer[track].data.step[i].rest = rest);
  }
  ATOMIC(_sequencer[track].mute = false);
}

// The callback function wich will be called by uClock each Pulse of 16PPQN clock resolution. Each call represents exactly one step.
void Engine303::onStepCall(uint32_t tick) 
{
  uint8_t step;
  uint16_t length;
  int8_t note;

  for ( uint8_t track = 0; track < TRACK_NUMBER_303; track++ ) {

    if ( _sequencer[track].mute == true ) {
      continue;
    }
    
    length = NOTE_LENGTH_303;
    
    // get actual step location.
    _sequencer[track].step_location = uint32_t(tick + _sequencer[track].data.shift) % _sequencer[track].data.step_length;
    
    // send note on only if this step are not in rest mode
    if ( _sequencer[track].data.step[_sequencer[track].step_location].rest == 0 ) {
  
      // check for slide event ahead of _sequencer[track].step_location
      step = _sequencer[track].step_location;
      for ( uint8_t i = 1; i < _sequencer[track].data.step_length; i++  ) {
        ++step;
        step = step % _sequencer[track].data.step_length;
        if ( _sequencer[track].data.step[step].slide == 1 && _sequencer[track].data.step[step].rest == 1 ) {
          length = NOTE_LENGTH_303 + (i * 6);
          break;
        } else if ( _sequencer[track].data.step[step].rest == 0 ) {
          break;
        }
      }
  
      // find a free note stack to fit in
      for ( uint8_t i = 0; i < NOTE_STACK_SIZE_303; i++ ) {
        if ( _sequencer[track].stack[i].length == -1 ) {
          if ( _harmonize == 1 ) {
            note = Harmonizer.harmonizer(_sequencer[track].data.step[_sequencer[track].step_location].note);
          } else {
            note = _sequencer[track].data.step[_sequencer[track].step_location].note;
          }
          note += _sequencer[track].data.transpose;
          // in case transpose push note away from the lower or higher midi note range barrier do not play it
          if ( note < 0 || note > 127 ) {
            break;
          }
          _sequencer[track].stack[i].note = note;
          _sequencer[track].stack[i].length = length;
          // send note on
          _onMidiEventCallback(NOTE_ON, note, _sequencer[track].data.step[_sequencer[track].step_location].accent ? ACCENT_VELOCITY_303 : NOTE_VELOCITY_303, _sequencer[track].channel, 0);
          break;
        }
      }
      
    } 
    
  } 
  
}

// The callback function wich will be called by uClock each Pulse of 96PPQN clock resolution.
void Engine303::onClockCall(uint32_t tick) 
{
  for ( uint8_t track = 0; track < TRACK_NUMBER_303; track++ ) {

    // handle note on stack
    for ( uint8_t i = 0; i < NOTE_STACK_SIZE_303; i++ ) {
      if ( _sequencer[track].stack[i].length != -1 ) {
        --_sequencer[track].stack[i].length;
        if ( _sequencer[track].stack[i].length == 0 ) {
          _onMidiEventCallback(NOTE_OFF, _sequencer[track].stack[i].note, 0, _sequencer[track].channel, 0);
          _sequencer[track].stack[i].length = -1;
        }
      }  
    }
  
  }
}

void Engine303::clearStackNote(int8_t track)
{
  if ( track <= -1 ) {
    // clear all tracks stack note
    for ( uint8_t i = 0; i < TRACK_NUMBER_303; i++ ) {
      // clear and send any note off 
      for ( uint8_t j = 0; j < NOTE_STACK_SIZE_303; j++ ) {
        _onMidiEventCallback(NOTE_OFF, _sequencer[i].stack[j].note, 0, _sequencer[i].channel, 0);
        _sequencer[i].stack[j].length = -1;
      } 
    }
  } else {
    // clear and send any note off 
    for ( uint8_t i = 0; i < NOTE_STACK_SIZE_303; i++ ) {
      _onMidiEventCallback(NOTE_OFF, _sequencer[track].stack[i].note, 0, _sequencer[track].channel, 0);
      _sequencer[track].stack[i].length = -1;
    }     
  }

}