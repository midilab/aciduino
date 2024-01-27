/*!
 *  @file       engine_303.cpp
 *  Project     Acid Sequencer
 *  @brief      Step Sequencer engine that emulates TB 303
 *  @version    0.9.0
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

/*
 if selected step is in rest mode, show tie option(if we put step in on state automaticly also set tie off for that one), if its in on mode, show slide option
 note length
 50% of a step on normal patterns, or 62.5% on triplet patterns 
 triplets: plays 3 notes within a time of 4 notes. so sequencing looks like its slower than normal
*/

#include "engine_303.h"

void Engine303::init()
{
  // initing sequencer memory data
  for ( uint8_t track = 0; track < TRACK_NUMBER_303; track++ ) {

    _sequencer[track].data.shift = 0;
    _sequencer[track].data.step_length = STEP_MAX_SIZE_303;
    _sequencer[track].data.transpose = 0;
    _sequencer[track].data.tune = 0;
    _sequencer[track].step_location = 0;
    _sequencer[track].mute = 0;

    // clear step data
    clearStepData(track, 0);
  
    // initing note stack data
    for ( uint8_t i = 0; i < NOTE_STACK_SIZE_303; i++ ) {
      _sequencer[track].stack[i].note = 0;
      _sequencer[track].stack[i].length = -1;
    }
    
  }
}

void * Engine303::getPatternData(uint8_t track)
{
  return (void *)&_sequencer[track];
}

uint16_t Engine303::getPatternMemorySize()
{
  return sizeof(_sequencer);
}

uint16_t Engine303::getPatternTrackSize()
{
  return sizeof(_sequencer[0]);
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

void Engine303::setTie(uint8_t track, uint8_t step, bool state) 
{
  ATOMIC(_sequencer[track].data.step[step].tie = state);
}

bool Engine303::stepOn(uint8_t track, uint8_t step)
{
  //int8_t relative_step = uint8_t(step - _sequencer[track].data.shift) % _sequencer[track].data.step_length+1;
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

bool Engine303::tieOn(uint8_t track, uint8_t step)
{
  return _sequencer[track].data.step[step].tie;
}

void Engine303::setStepData(uint8_t track, uint8_t step, uint8_t data)
{
  ATOMIC(_sequencer[track].data.step[step].note = data);
}

uint8_t Engine303::getStepData(uint8_t track, uint8_t step)
{
  // harmonizer on?
  if ( _sequencer[track].data.tune > 0 )
    return Harmonizer.harmonizer(_sequencer[track].data.step[step].note) + (_sequencer[track].data.tune-1);

  return _sequencer[track].data.step[step].note;
}

void Engine303::setShiftPos(uint8_t track, int8_t shift)
{
  ATOMIC(_sequencer[track].data.shift = shift);
}

int8_t Engine303::getShiftPos(uint8_t track)
{
  return _sequencer[track].data.shift;
}

uint8_t Engine303::getTune(uint8_t track)
{
  return _sequencer[track].data.tune;
}

void Engine303::setTune(uint8_t track, uint8_t tune)
{
  ATOMIC(_sequencer[track].data.tune = tune)
}

int8_t Engine303::getTranspose(uint8_t track)
{
  return _sequencer[track].data.transpose;
}

void Engine303::setTranspose(uint8_t track, int8_t transpose)
{
  ATOMIC(_sequencer[track].data.transpose = transpose)
}

uint8_t Engine303::getTemperamentId()
{
  return Harmonizer.getTemperamentId();
}

void Engine303::setTemperament(uint8_t temperament_id)
{
  ATOMIC(Harmonizer.setTemperament(temperament_id))
}

const char * Engine303::getTemperamentName(uint8_t temperament_id)
{
  return Harmonizer.getTemperamentName(temperament_id);
}

uint8_t Engine303::getCurrentStep(uint8_t track)
{
  static uint8_t step;
  ATOMIC(step = _sequencer[track].step_location) 
  return step;
}

void Engine303::clearStepData(uint8_t track, uint8_t rest)
{
  for ( uint16_t i = 0; i < STEP_MAX_SIZE_303; i++ ) {
    _sequencer[track].data.step[i].note = 36;
    _sequencer[track].data.step[i].accent = 0;
    _sequencer[track].data.step[i].slide = 0;
    _sequencer[track].data.step[i].rest = rest;
    _sequencer[track].data.step[i].tie = 0;
  }
}

void Engine303::clearTrack(uint8_t track)
{
  ATOMIC(_sequencer[track].mute = true);
  clearStackNote(track);
  clearStepData(track, 1);
  ATOMIC(_sequencer[track].mute = false);
}

uint16_t Engine303::getTrackLength(uint8_t track)
{
  //static uint16_t length;
  //ATOMIC(length = _sequencer[track].data.step_length+1) 
  //return length;
  return _sequencer[track].data.step_length;
}

void Engine303::setTrackLength(uint8_t track, uint16_t length)
{
  ATOMIC(_sequencer[track].data.step_length = length);  
}

void Engine303::setMute(uint8_t track, uint8_t mute)
{
  ATOMIC(_sequencer[track].mute = mute);  
}

uint8_t Engine303::getMute(uint8_t track)
{
  return _sequencer[track].mute;
}

void Engine303::setLongTie(uint8_t track, uint8_t start_step, uint8_t step_end)
{
  uint8_t steps_holded = 0;
  uint8_t note = getStepData(track, start_step);
  uint8_t step_size = getTrackLength(track);

  start_step = (start_step+1) % getTrackLength(track);

  // hold longer than track end step
  if (step_end < start_step) {
    steps_holded = step_size - (start_step - step_end);
  } else if (step_end > start_step) {
    steps_holded = step_end - start_step;
  }

  //step_end = step_end == 0 ? getTrackLength(track)-1 : step_end-1;

  if (steps_holded > 0) {
    uint16_t end_step = start_step+steps_holded;
    for (uint16_t i=start_step; i < end_step; i++) {
      uint8_t step_idx = i%step_size;
      // set step to rest mode and tie it and same note holded!
      rest(track, step_idx, true);
      setTie(track, step_idx, true);
      setSlide(track, step_idx, false);
      setStepData(track, step_idx, note);
    }
  }
}

void Engine303::acidRandomize(uint8_t track, uint8_t fill, uint8_t accent_probability, uint8_t slide_probability, uint8_t tie_probability, uint8_t number_of_tones, uint8_t lower_note, uint8_t range_note) 
{
  uint8_t note, high_note, accent, slide, tie, rest, last_step;
  // initialize as full tone scale array(root c)
  uint8_t fix_tones[12] = {0};

  // clear track before random data or only clear stack note?
  // probably clear stack note is a better idea
  
  // prepare the allowed notes for number of tones parameter
  // fix notes outside number of tones choosed and pre computed
  if (number_of_tones > 0) {
    // add offset to fix number of tones requested
    // [0, -1, -2, -3, -4, 0, -1, -2, -3, -4, -5, -6]
    int8_t fix_counter = 0;
    int8_t fix_mod = 12/number_of_tones;
    for (uint8_t i=0; i < 12; i++) {
      if (!(i % fix_mod) || i == 0) {
        fix_tones[i] = 0;
        fix_counter = 0;
      } else {
        --fix_counter;
        fix_tones[i] = fix_counter;
      }
    }
  }

  // random it all
  ATOMIC(_sequencer[track].mute = true);
  clearStackNote(track);

  for ( uint16_t i = 0; i < STEP_MAX_SIZE_303; i++ ) {

    // step on/off
    _sequencer[track].data.step[i].rest = random(0, 100) < fill ? 0 : 1;

    // random tie and reset accent and slide in case of a rest
    if (_sequencer[track].data.step[i].rest) {

      _sequencer[track].data.step[i].accent = 0;
      _sequencer[track].data.step[i].slide = 0;
      _sequencer[track].data.step[i].tie = 0;
      
      if (i == 0) {
        last_step = STEP_MAX_SIZE_303-1;
      } else {
        last_step = i-1;
      }

      // only tie probrablity when last step has a note or another tie event
      if (_sequencer[track].data.step[last_step].rest == 0 || _sequencer[track].data.step[last_step].tie == 1)
        _sequencer[track].data.step[i].tie = random(0, 100) < tie_probability ? 1 : 0;

      continue;

    }

    high_note = lower_note+range_note;
    if ( high_note > 127 ) {
      high_note = 127;
    }

    note = random(lower_note, high_note);
    // fix notes outside number of tones choosed and pre computed
    if (number_of_tones > 0) {
      note += fix_tones[note%12];
    }

    accent = random(0, 100) < accent_probability ? 1 : 0;
    slide = random(0, 100) < slide_probability ? 1 : 0;
    
    _sequencer[track].data.step[i].note = note;
    _sequencer[track].data.step[i].accent = accent;
    _sequencer[track].data.step[i].slide = slide;
  }
  ATOMIC(_sequencer[track].mute = false);
}

// The callback function wich will be called by uClock each new step event time
void Engine303::onStepCall(uint32_t tick, int8_t shuffle_length_ctrl) 
{
  uint8_t step, next_step;
  uint16_t length, step_length;
  int8_t length_shuffe;
  int8_t note;

  for ( uint8_t track = 0; track < TRACK_NUMBER_303; track++ ) {

    // get actual step location.
    _sequencer[track].step_location = (tick + _sequencer[track].data.shift) % _sequencer[track].data.step_length;

    // are we mute?
    if (_sequencer[track].mute)
      continue;

    // length based on 96PPQN (24 pulses in between each step)
    length = NOTE_LENGTH_303;
    
    // send note on only if this step are not in rest mode
    if ( _sequencer[track].data.step[_sequencer[track].step_location].rest == 0 ) {

      // check for slide or tie event ahead of _sequencer[track].step_location
      step = _sequencer[track].step_location;
      next_step = step;
      step_length = _sequencer[track].data.step_length + 1;
      for ( uint8_t i = 1; i < step_length; i++  ) {
        next_step = ++next_step % step_length;
        if (_sequencer[track].data.step[step].slide == 1 && _sequencer[track].data.step[next_step].rest == 0) {
          length = NOTE_LENGTH_303 + 20;
          break;
        } else if (_sequencer[track].data.step[next_step].tie == 1 && _sequencer[track].data.step[next_step].rest == 1) {
          length = NOTE_LENGTH_303 + (i * 24);
        } else if ( _sequencer[track].data.step[next_step].rest == 0 || _sequencer[track].data.step[next_step].tie == 0) {
          break;
        }
      }

      // apply shuffle length?
      if (shuffle_length_ctrl != 0) {
        length_shuffe = length + shuffle_length_ctrl;
        length = length_shuffe <= 0 ? 1 : length_shuffe;
      }
      
      // find a free note stack to fit in
      for ( uint8_t i = 0; i < NOTE_STACK_SIZE_303; i++ ) {
        if ( _sequencer[track].stack[i].length == -1 ) {
          if ( _sequencer[track].data.tune > 0 ) {
            note = Harmonizer.harmonizer(_sequencer[track].data.step[_sequencer[track].step_location].note) + (_sequencer[track].data.tune-1);
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
          _onEventCallback(NOTE_ON, note, _sequencer[track].data.step[_sequencer[track].step_location].accent ? ACCENT_VELOCITY_303 : NOTE_VELOCITY_303, track);
          break;
        }
      }
      
    } 
    
  } 
  
}

// The callback function wich will be called by uClock each Pulse of 96PPQN clock resolution.
void Engine303::on96PPQNCall(uint32_t tick) 
{
  for ( uint8_t track = 0; track < TRACK_NUMBER_303; track++ ) {

    // handle note on stack
    for ( uint8_t i = 0; i < NOTE_STACK_SIZE_303; i++ ) {
      if ( _sequencer[track].stack[i].length != -1 ) {
        --_sequencer[track].stack[i].length;
        if ( _sequencer[track].stack[i].length == 0 ) {
          _onEventCallback(NOTE_OFF, _sequencer[track].stack[i].note, 0, track);
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
        _onEventCallback(NOTE_OFF, _sequencer[i].stack[j].note, 0, i);
        _sequencer[i].stack[j].length = -1;
      } 
    }
  } else {
    // clear and send any note off 
    for ( uint8_t i = 0; i < NOTE_STACK_SIZE_303; i++ ) {
      _onEventCallback(NOTE_OFF, _sequencer[track].stack[i].note, 0, track);
      _sequencer[track].stack[i].length = -1;
    }     
  }

}