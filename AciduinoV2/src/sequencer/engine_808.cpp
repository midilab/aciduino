/*!
 *  @file       engine_808.cpp
 *  Project     Acid Sequencer
 *  @brief      Step Sequencer engine that emulates TR 808
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

#include "engine_808.h"

void Engine808::setTrackChannel(uint8_t track, uint8_t channel)
{
  //ATOMIC(_sequencer[track].channel = channel);
}

void Engine808::init()
{
  // initing sequencer memory data
  for ( uint8_t track = 0; track < TRACK_NUMBER_808; track++ ) {

    _sequencer[track].channel = track+TRACK_NUMBER_808;
    _sequencer[track].step_location = 0;
    _sequencer[track].mute = false;

    // initing voice data
    for ( uint16_t i = 0; i < VOICE_MAX_SIZE_808; i++ ) {
      //_sequencer[track].voice[i].name[0] = (char)(i+97); // 97=a assci
      _sequencer[track].voice[i].name[0] = _default_voice_data_808[i].name[0]; 
      _sequencer[track].voice[i].name[1] = _default_voice_data_808[i].name[1]; 
      _sequencer[track].voice[i].shift = 0;
      _sequencer[track].voice[i].step_length = STEP_MAX_SIZE_808;
      _sequencer[track].voice[i].stack_length = -1;\
      //_sequencer[track].voice[i].note = 36+i; // general midi drums map #36 kick drum
      _sequencer[track].voice[i].note = _default_voice_data_808[i].note; //36+i; // general midi drums map #36 kick drum
      _sequencer[track].voice[i].accent = 0ULL;
      _sequencer[track].voice[i].roll = 0ULL;
      _sequencer[track].voice[i].steps = 0ULL;
      // get a 4/4 kick mark on firts voice of each channel
      if (i == 0) {
        for ( uint16_t j = 0; j < STEP_MAX_SIZE_808; j++ ) {
          if (j % 4 == 0) {
            SET_BIT(_sequencer[track].voice[i].steps, j);
          }
        }
      }
    }
  }
}

// The callback function wich will be called by uClock each Pulse of 16PPQN clock resolution. Each call represents exactly one step.
void Engine808::onStepCall(uint32_t tick) 
{
  uint8_t step;
  int8_t note;
  bool accent, roll = false;

  for ( uint8_t track = 0; track < TRACK_NUMBER_808; track++ ) {

    if ( _sequencer[track].mute == true ) {
      continue;
    }

    // get global step location.
    _sequencer[track].step_location = uint32_t(tick + _sequencer[track].shift) % _sequencer[track].step_length;

    // walk thru all track voices
    for (uint8_t voice = 0; voice < VOICE_MAX_SIZE_808; voice++) {
      
      step = (_sequencer[track].step_location + _sequencer[track].voice[voice].shift) % _sequencer[track].voice[voice].step_length;

      // send note on only if this step are not in rest mode
      if ( GET_BIT(_sequencer[track].voice[voice].steps, step) ) {

        // is this roll?
        roll = GET_BIT(_sequencer[track].voice[voice].roll, step);

        // does it have accent?
        accent = GET_BIT(_sequencer[track].voice[voice].accent, step);

        // it this a roll? prepare the data
        _sequencer[track].voice[voice].stack_length = NOTE_LENGTH_808;

        // send the drum triger
        _onMidiEventCallback(NOTE_ON, _sequencer[track].voice[voice].note, accent ? ACCENT_VELOCITY_808 : NOTE_VELOCITY_808, _sequencer[track].channel, 0);   

      } 
    }
    
  } 
}

// The callback function wich will be called by uClock each Pulse of 96PPQN clock resolution.
void Engine808::onClockCall(uint32_t tick) 
{
  for ( uint8_t track = 0; track < TRACK_NUMBER_808; track++ ) {

    // handle note on stack
    for ( uint8_t i = 0; i < VOICE_MAX_SIZE_808; i++ ) {
      if ( _sequencer[track].voice[i].stack_length != -1 ) {
        --_sequencer[track].voice[i].stack_length;
        if ( _sequencer[track].voice[i].stack_length == 0 ) {
          _onMidiEventCallback(NOTE_OFF, _sequencer[track].voice[i].note, 0, _sequencer[track].channel, 0);
          _sequencer[track].voice[i].stack_length = -1;
        }
      }  
    }
  
  }
}

void Engine808::clearStackNote(int8_t track)
{
  if ( track <= -1 ) {
    // clear all tracks stack note
    for ( uint8_t i = 0; i < TRACK_NUMBER_808; i++ ) {
      // clear and send any note off 
      for ( uint8_t j = 0; j < VOICE_MAX_SIZE_808; j++ ) {
        _onMidiEventCallback(NOTE_OFF, _sequencer[i].voice[j].note, 0, _sequencer[i].channel, 0);
        _sequencer[i].voice[j].stack_length = -1;
      } 
    }
  } else {
    // clear and send any note off 
    for ( uint8_t i = 0; i < VOICE_MAX_SIZE_808; i++ ) {
      _onMidiEventCallback(NOTE_OFF, _sequencer[track].voice[i].note, 0, _sequencer[track].channel, 0);
      _sequencer[track].voice[i].stack_length = -1;
    }     
  }
}

void Engine808::rest(uint8_t track, uint8_t step, bool state) 
{
  if (state) {
    ATOMIC(CLR_BIT(_sequencer[track].voice[_voice].steps, step));
  } else {
    ATOMIC(SET_BIT(_sequencer[track].voice[_voice].steps, step));
  }
}

void Engine808::setAccent(uint8_t track, uint8_t step, bool state) 
{
  if (state) {
    ATOMIC(SET_BIT(_sequencer[track].voice[_voice].accent, step));
  } else {
    ATOMIC(CLR_BIT(_sequencer[track].voice[_voice].accent, step));
  }
}

void Engine808::setRoll(uint8_t track, uint8_t step, bool state) 
{
  if (state) {
    ATOMIC(SET_BIT(_sequencer[track].voice[_voice].roll, step));
  } else {
    ATOMIC(CLR_BIT(_sequencer[track].voice[_voice].roll, step));
  }
}

bool Engine808::stepOn(uint8_t track, uint8_t step)
{
    return GET_BIT(_sequencer[track].voice[_voice].steps, step);
}

bool Engine808::accentOn(uint8_t track, uint8_t step)
{
    return GET_BIT(_sequencer[track].voice[_voice].accent, step);
}

bool Engine808::rollOn(uint8_t track, uint8_t step)
{
    return GET_BIT(_sequencer[track].voice[_voice].roll, step);
}

void Engine808::setStepData(uint8_t track, uint8_t step, uint8_t data)
{
  ATOMIC(_sequencer[track].voice[_voice].note = data);
}

uint8_t Engine808::getStepData(uint8_t track, uint8_t step)
{
  return _sequencer[track].voice[_voice].note;
}

uint8_t Engine808::getCurrentStep(uint8_t track)
{
    static uint8_t step;
    //ATOMIC(step = _sequencer[track].step_location) 
    // for voices
    ATOMIC(step = (_sequencer[track].step_location + _sequencer[track].voice[_voice].shift) % _sequencer[track].voice[_voice].step_length) 
    return step;
}

uint8_t Engine808::getTrackChannel(uint8_t track)
{
  return _sequencer[track].channel;
}

uint8_t Engine808::getTrackLength(uint8_t track)
{
    static uint8_t length;
    ATOMIC(length = _sequencer[track].voice[_voice].step_length) 
    return length;
}

void Engine808::setTrackLength(uint8_t track, uint16_t length)
{
  ATOMIC(_sequencer[track].voice[_voice].step_length = length);  
}

void Engine808::setShiftPos(uint8_t track, int8_t shift)
{
  ATOMIC(_sequencer[track].voice[_voice].shift = shift);
}

int8_t Engine808::getShiftPos(uint8_t track)
{
  return _sequencer[track].voice[_voice].shift;
}

void Engine808::setTrackVoice(uint8_t track = 0, uint8_t voice = 0)
{
  _voice = voice;  
}

uint8_t Engine808::getTrackVoice(uint8_t track = 0)
{
  return _voice;  
}

uint8_t Engine808::getTrackVoiceConfig(uint8_t track)
{
  return _sequencer[track].voice[_voice].note;
}

void Engine808::setTrackVoiceConfig(uint8_t track, uint8_t note)
{
  ATOMIC(_sequencer[track].voice[_voice].note = note);
}

const char * Engine808::getTrackVoiceName(uint8_t track = 0, uint8_t voice = 0)
{
  return (const char *)_sequencer[track].voice[voice].name;  
}

void Engine808::acidRandomize(uint8_t track, uint8_t fill, uint8_t accent_probability, uint8_t roll_probability) 
{
  uint16_t accent, roll;

  uint16_t bjorklund_data = _bjorklund.compute(_sequencer[track].voice[_voice].step_length, ceil(_sequencer[track].voice[_voice].step_length*(float)(fill/100.0)));

  ATOMIC(_sequencer[track].mute = true)
  //clearStackNote(track); // PS: this is global! how to not affect other tracks?
  _sequencer[track].voice[_voice].steps = bjorklund_data;

  // randomize accent and roll?
  _sequencer[track].voice[_voice].accent = 0ULL;
  _sequencer[track].voice[_voice].roll = 0ULL;
  for ( uint16_t i = 0; i < STEP_MAX_SIZE_808; i++ ) {

    // classic randomizer
    // random(0, 100) < fill ? 1 : 0;
    // we are going to randomize only parameters where step is on
    if (GET_BIT(_sequencer[track].voice[_voice].steps, i)) {

      if (random(0, 100) < accent_probability)
        SET_BIT(_sequencer[track].voice[_voice].accent, i);

      if (random(0, 100) < roll_probability)
        SET_BIT(_sequencer[track].voice[_voice].roll, i);    
      
    }

  }

  ATOMIC(_sequencer[track].mute = false);
}