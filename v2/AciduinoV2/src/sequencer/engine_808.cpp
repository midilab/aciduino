/*!
 *  @file       engine_808.cpp
 *  Project     Acid Sequencer
 *  @brief      Step Sequencer engine that emulates TR 808
 *  @version    0.9.0
 *  @author     Romulo Silva
 *  @date       22/09/2022
 *  @license    MIT - (c) 2024 - Romulo Silva - contact@midilab.co
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

// TODO:
// + pre-scale function
// + roll/flam 0-48ms one step hits two times place. (on our implementation of 96ppqn we can only use 6 steps and no millisecond precision)
// + substep: duplets, triplets, qudruplets... like flam but with more than 2 hits
// + add weak note? as 808 cloud
// + solo and mute for voices?
// + copy/paste
// + shuffle
// + var a/b? we can keep length as more usefull stuff but.. get a fill stuff will be cool instead
// + fill per pattern? the length control looks like better approach before goes into full 808 mode
// + global or single rack accent level controller(will vary the accent level for midi from base velocity to the setup velocity max)
// + clear accents
// + step button function (the tap) to record wilhe play the steps! this is a good one(generic1 and generic2: one could be the swap step selected just like the original one press invert the on/off state, the other button can be the one for live step record)
// the same idea for step button for record can be used for 303, with knob for note selection while press play
// global accent options vs per track accent(make an option, same programmin way but changes the whole track)
// on generative add the option to generate new accent only
#include "engine_808.h"

void Engine808::init()
{
  // initing sequencer memory data
  for ( uint8_t track = 0; track < TRACK_NUMBER_808; track++ ) {

    _sequencer[track].step_location = 0;
    _sequencer[track].step_length = STEP_MAX_SIZE_808;
    _sequencer[track].mute = 0;
    _sequencer[track].roll_type = SUB_STEP_1;

#ifdef GLOBAL_ACCENT
    for ( uint8_t i = 0; i < STEP_MAX_SIZE_808; i++ ) {
      CLR_BIT(_sequencer[track].accent, i);
    }
#endif

    // initing voice data
    for ( uint8_t i = 0; i < VOICE_MAX_SIZE_808; i++ ) {
      // avoid setup problems for our 11 pre defined names
      if (i < 11) {
        _sequencer[track].voice[i].name[0] = _default_voice_data_808[i].name[0]; 
        _sequencer[track].voice[i].name[1] = _default_voice_data_808[i].name[1]; 
      } else {
        _sequencer[track].voice[i].name[0] = (char)(i+97); // 97=a assci
      }
      _sequencer[track].voice[i].mute = 0;
      _sequencer[track].voice[i].shift = 0;
      _sequencer[track].voice[i].step_length = STEP_MAX_SIZE_808;
      _sequencer[track].voice[i].trigger_ctrl = 0;
      _sequencer[track].voice[i].note = _default_voice_data_808[i].note; //36+i; // general midi drums map #36 kick drum
      clearStepData(track, i);
      // if bd, fill 4/4 kick pattern;
      if (i == 0) {
        _sequencer[track].voice[i].steps = 1229782938247303441;
      }
    }
  }
}

// The callback function wich will be called by uClock each Pulse of 16PPQN clock resolution. Each call represents exactly one step.
void Engine808::onStepCall(uint32_t tick, int8_t shuffle_length_ctrl) 
{
  uint8_t step;
  int8_t note;
  bool accent, roll = false;

  for ( uint8_t track = 0; track < TRACK_NUMBER_808; track++ ) {

    if ( _sequencer[track].mute == true ) {
      continue;
    }
    
    // get global step location.
    _sequencer[track].step_location = (tick + _sequencer[track].shift) % _sequencer[track].step_length;

    // walk thru all track voices
    for (uint8_t voice = 0; voice < VOICE_MAX_SIZE_808; voice++) {
      
      if ( _sequencer[track].voice[voice].mute == true ) {
        continue;
      }

      step = (_sequencer[track].step_location + _sequencer[track].voice[voice].shift) % _sequencer[track].voice[voice].step_length;

      // send note on only if this step are not in rest mode
      if ( GET_BIT(_sequencer[track].voice[voice].steps, step) ) {

        // is this roll?
        roll = GET_BIT(_sequencer[track].voice[voice].roll, step);

        // does it have accent?
#ifdef GLOBAL_ACCENT
        accent = GET_BIT(_sequencer[track].accent, step);
#else
        accent = GET_BIT(_sequencer[track].voice[voice].accent, step);
#endif
        // it this a roll? prepare the data
        if (roll) {
          // a full one step in 96ppqn is 24 pulses. minus this shot one, we have 23 pulses left
          _sequencer[track].voice[voice].trigger_ctrl = -24;
          if (_sequencer[track].roll_type >= SUB_STEP_1)
            return; // handle on 96ppqn only
        } else {
          _sequencer[track].voice[voice].trigger_ctrl = NOTE_LENGTH_808;
        }
        
        // send the drum triger
        _onEventCallback(NOTE_ON, _sequencer[track].voice[voice].note, accent ? ACCENT_VELOCITY_808 : NOTE_VELOCITY_808, track+TRACK_NUMBER_303);   
      } 
    }
    
  } 
}

// The callback function wich will be called by uClock each Pulse of 96PPQN
void Engine808::on96PPQNCall(uint32_t tick) 
{
  for ( uint8_t track = 0; track < TRACK_NUMBER_808; track++ ) {

    // handle trigger stack
    for ( uint8_t i = 0; i < VOICE_MAX_SIZE_808; i++ ) {

      // are we mute?
      if (_sequencer[track].voice[i].mute)
        continue;
        
      // normal trigger on event
      if ( _sequencer[track].voice[i].trigger_ctrl > 0 ) { 

        --_sequencer[track].voice[i].trigger_ctrl;
        if ( _sequencer[track].voice[i].trigger_ctrl == 0 ) {
          _onEventCallback(NOTE_OFF, _sequencer[track].voice[i].note, 0, track+TRACK_NUMBER_303);
        }

      // roll handler
      } else if ( _sequencer[track].voice[i].trigger_ctrl < 0 ) { 
        
        bool shot_the_moon = false;
        ++_sequencer[track].voice[i].trigger_ctrl;
        if (_sequencer[track].roll_type <= FLAM_5) {
          if (_sequencer[track].voice[i].trigger_ctrl == _sequencer[track].roll_type*-1)
            shot_the_moon = true;
        } else {
          // SUB_STEP_1
          if (_sequencer[track].roll_type == SUB_STEP_1) {
            if (_sequencer[track].voice[i].trigger_ctrl%8 == 0) 
              shot_the_moon = true;
          // SUB_STEP_2
          } else if (_sequencer[track].roll_type == SUB_STEP_2) {
            if (_sequencer[track].voice[i].trigger_ctrl%4 == 0) 
              shot_the_moon = true;
          }
        }

        if (shot_the_moon)
          _onEventCallback(NOTE_ON, _sequencer[track].voice[i].note, ACCENT_VELOCITY_808, track+TRACK_NUMBER_303);
      }

    }

  }
}

void Engine808::clearStackNote(int8_t track)
{
  //if ( track <= -1 ) {
    // clear all tracks stack note
    for ( uint8_t i = 0; i < TRACK_NUMBER_808; i++ ) {
      // clear and send any note off 
      for ( uint8_t j = 0; j < VOICE_MAX_SIZE_808; j++ ) {
        _onEventCallback(NOTE_OFF, _sequencer[i].voice[j].note, 0, i+TRACK_NUMBER_303);
        _sequencer[i].voice[j].trigger_ctrl = 0;
      }
    }
  //} else {
  //  // clear and send any note off 
  //  for ( uint8_t i = 0; i < VOICE_MAX_SIZE_808; i++ ) {
  //    _onEventCallback(NOTE_OFF, _sequencer[track].voice[i].note, 0, track+TRACK_NUMBER_303);
  //    _sequencer[track].voice[i].trigger_ctrl = 0;
  //  }     
  //}
}

void * Engine808::getPatternData(uint8_t track)
{
  return (void *)&_sequencer[track];
}

uint16_t Engine808::getPatternMemorySize()
{
  return sizeof(_sequencer);
}

uint16_t Engine808::getPatternTrackSize()
{
  return sizeof(_sequencer[0]);
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
#ifdef GLOBAL_ACCENT
  if (state) {
    ATOMIC(SET_BIT(_sequencer[track].accent, step));
  } else {
    ATOMIC(CLR_BIT(_sequencer[track].accent, step));
  }
#else
  if (state) {
    ATOMIC(SET_BIT(_sequencer[track].voice[_voice].accent, step));
  } else {
    ATOMIC(CLR_BIT(_sequencer[track].voice[_voice].accent, step));
  }
#endif
}

void Engine808::setRoll(uint8_t track, uint8_t step, bool state) 
{
  if (state) {
    ATOMIC(SET_BIT(_sequencer[track].voice[_voice].roll, step));
  } else {
    ATOMIC(CLR_BIT(_sequencer[track].voice[_voice].roll, step));
  }
}

void Engine808::setRollType(uint8_t track, uint8_t type)
{
  ATOMIC(_sequencer[track].roll_type = type);
}

uint8_t Engine808::getRollType(uint8_t track)
{
  return _sequencer[track].roll_type;
}

bool Engine808::stepOn(uint8_t track, uint8_t step)
{
    return GET_BIT(_sequencer[track].voice[_voice].steps, step);
}

bool Engine808::accentOn(uint8_t track, uint8_t step)
{
#ifdef GLOBAL_ACCENT
  return GET_BIT(_sequencer[track].accent, step);
#else
  return GET_BIT(_sequencer[track].voice[_voice].accent, step);
#endif
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

void Engine808::clearStepData(uint8_t track, uint8_t voice)
{
  _sequencer[track].voice[voice].steps = 0UL;
  _sequencer[track].voice[voice].roll = 0UL;
#ifndef GLOBAL_ACCENT
  _sequencer[track].voice[voice].accent = 0UL;
#endif
}

void Engine808::clearTrack(uint8_t track, uint8_t mode)
{
  // clear voice
  if (mode == 1) {
    ATOMIC(_sequencer[track].voice[_voice].mute = true)
    clearStepData(track, _voice);
    ATOMIC(_sequencer[track].voice[_voice].mute = false)
  // clear all track voices
  } else if (mode == 0) {
    ATOMIC(_sequencer[track].mute = true)
    for ( uint8_t i = 0; i < VOICE_MAX_SIZE_808; i++ ) {
      clearStepData(track, i);
    }
#ifdef GLOBAL_ACCENT
    _sequencer[track].accent = 0UL;
#endif
    ATOMIC(_sequencer[track].mute = false)
  }
}

uint16_t Engine808::getTrackLength(uint8_t track)
{
  //static uint16_t length;
  //ATOMIC(length = _sequencer[track].voice[_voice].step_length) 
  //return length;
  return _sequencer[track].voice[_voice].step_length;
}

void Engine808::setTrackLength(uint8_t track, uint16_t length)
{
  ATOMIC(_sequencer[track].voice[_voice].step_length = length);  
}

void Engine808::setMute(uint8_t track, uint8_t voice, uint8_t mute)
{
  ATOMIC(_sequencer[track].voice[voice].mute = mute);  
}

void Engine808::setMute(uint8_t track, uint8_t mute)
{
  ATOMIC(_sequencer[track].mute = mute);  
}

uint8_t Engine808::getMute(uint8_t track, uint8_t voice)
{
  return _sequencer[track].voice[voice].mute;
}

uint8_t Engine808::getMute(uint8_t track)
{
  return _sequencer[track].mute;
}

void Engine808::setShiftPos(uint8_t track, int8_t shift)
{
  ATOMIC(_sequencer[track].voice[_voice].shift = shift);
}

int8_t Engine808::getShiftPos(uint8_t track)
{
  return _sequencer[track].voice[_voice].shift;
}

void Engine808::setTrackVoice(uint8_t track, uint8_t voice)
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

int8_t Engine808::getTrackVoiceByNote(uint8_t track, uint8_t note)
{
  for (uint8_t voice=0; voice < VOICE_MAX_SIZE_808; voice++) {
    if (_sequencer[track].voice[voice].note == note) {
      return voice;
    }
  }
  return -1;
}

void Engine808::setTrackVoiceConfig(uint8_t track, uint8_t note)
{
  ATOMIC(_sequencer[track].voice[_voice].note = note);
}

const char * Engine808::getTrackVoiceName(uint8_t track, uint8_t voice)
{
  return (const char *)_sequencer[track].voice[voice].name;  
}

void Engine808::acidRandomize(uint8_t track, uint8_t fill, uint8_t accent_probability, uint8_t roll_probability) 
{
  ATOMIC(_sequencer[track].mute = true)
  //clearStackNote(track); // PS: this is global! how to not affect other tracks?
  _sequencer[track].voice[_voice].steps = _bjorklund.compute(_sequencer[track].voice[_voice].step_length, ceil(_sequencer[track].voice[_voice].step_length*(float)(fill/100.0)));

  for ( uint16_t i = 0; i < STEP_MAX_SIZE_808; i++ ) {

#ifndef GLOBAL_ACCENT
    CLR_BIT(_sequencer[track].voice[_voice].accent, i);
#endif
    CLR_BIT(_sequencer[track].voice[_voice].roll, i);

    // classic randomizer
    // random(0, 100) < fill ? 1 : 0;
    // we are going to randomize only parameters where step is on
    if (GET_BIT(_sequencer[track].voice[_voice].steps, i)) {

#ifndef GLOBAL_ACCENT
      if (random(0, 100) < accent_probability)
        SET_BIT(_sequencer[track].voice[_voice].accent, i);
#endif

      if (random(0, 100) < roll_probability)
        SET_BIT(_sequencer[track].voice[_voice].roll, i);    
      
    }

  }

  ATOMIC(_sequencer[track].mute = false);
}