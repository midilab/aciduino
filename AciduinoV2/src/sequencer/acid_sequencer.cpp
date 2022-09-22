/*!
 *  @file       acid_sequencer.cpp
 *  Project     Acid Sequencer
 *  @brief      Step Sequencer that emulates roland TX series sequencers
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

// Acid StepSequencer, 
// a modular roland classic machines sequencer engine clone
// TB303, TR808, TR909
// each module can be setup output(MIDI, CV/GATE) a page to config will be nice!

// idea for 808 roll 
// use a flag or value for note_off to get into loop inside 96ppqn call with 
// note on and off controlled until it finished the step
#include "acid_sequencer.h"

AcidSequencerClass::AcidSequencerClass()
{
  // initing our sequencer engines
  if (TRACK_NUMBER_303 > 0) {
    _engine303 = new Engine303();
    _engine303->init();
  }

  if (TRACK_NUMBER_808 > 0) {
    _engine808 = new Engine808();
    _engine808->init();
  }
}

// The callback function wich will be called by uClock each Pulse of 16PPQN clock resolution. Each call represents exactly one step.
void AcidSequencerClass::on16PPQN(uint32_t tick) 
{
  // 303 sequencer call
  _engine303->onStepCall(tick);
  // 808 sequencer call
  _engine808->onStepCall(tick);
}

// The callback function wich will be called by uClock each Pulse of 96PPQN clock resolution.
void AcidSequencerClass::on96PPQN(uint32_t tick) 
{
  // 303 clock call
  _engine303->onClockCall(tick);
  // 808 clock call
  _engine808->onClockCall(tick);
}

void AcidSequencerClass::clearStackNote(int8_t track)
{
  if (track == -1) {
    _engine808->clearStackNote(track);
    _engine303->clearStackNote(track);
    return;
  }
  // 808 request?
  if (track >= TRACK_NUMBER_303)
    _engine808->clearStackNote(track-TRACK_NUMBER_303);
  // 303 request
  else
    _engine303->clearStackNote(track);
}

void AcidSequencerClass::setTrackChannel(uint8_t track, uint8_t channel) 
{
  // 808 request?
  if (track >= TRACK_NUMBER_303)
    _engine808->setTrackChannel(track-TRACK_NUMBER_303, channel);
  // 303 request
  else
    _engine303->setTrackChannel(track, channel);
}

void AcidSequencerClass::rest(uint8_t track, uint8_t step, bool state) 
{
  // 808 request?
  if (track >= TRACK_NUMBER_303)
    _engine808->rest(track-TRACK_NUMBER_303, step, state);
  // 303 request
  else
    _engine303->rest(track, step, state);
}

void AcidSequencerClass::setAccent(uint8_t track, uint8_t step, bool state) 
{
  // 808 request?
  if (track >= TRACK_NUMBER_303)
    _engine808->setAccent(track-TRACK_NUMBER_303, step, state);
  // 303 request
  else
    _engine303->setAccent(track, step, state);
}

void AcidSequencerClass::setSlide(uint8_t track, uint8_t step, bool state) 
{
  // 303 request
  if (track < TRACK_NUMBER_303)
    _engine303->setSlide(track, step, state);
}

void AcidSequencerClass::setRoll(uint8_t track, uint8_t step, bool state) 
{
  // 808 request?
  if (track >= TRACK_NUMBER_303)
    _engine808->setRoll(track-TRACK_NUMBER_303, step, state);
}

bool AcidSequencerClass::stepOn(uint8_t track, uint8_t step)
{
  // 808 request?
  if (track >= TRACK_NUMBER_303)
    return _engine808->stepOn(track-TRACK_NUMBER_303, step);
  
  // 303 request
  return _engine303->stepOn(track, step);
}

bool AcidSequencerClass::accentOn(uint8_t track, uint8_t step)
{
  // 808 request?
  if (track >= TRACK_NUMBER_303)
    return _engine808->accentOn(track-TRACK_NUMBER_303, step);
  
  // 303 request
  return _engine303->accentOn(track, step);
}

bool AcidSequencerClass::slideOn(uint8_t track, uint8_t step)
{
  // 808 request?
  if (track >= TRACK_NUMBER_303)
    return false;
  
  // 303 request
  return _engine303->slideOn(track, step);
}

bool AcidSequencerClass::rollOn(uint8_t track, uint8_t step)
{
  // 808 request?
  if (track >= TRACK_NUMBER_303)
    return _engine808->rollOn(track-TRACK_NUMBER_303, step);
    
  return false;
}

void AcidSequencerClass::setStepData(uint8_t track, uint8_t step, uint8_t data)
{
  // 808 request?
  if (track >= TRACK_NUMBER_303)
    _engine808->setStepData(track-TRACK_NUMBER_303, step, data);
  // 303 request
  else
    _engine303->setStepData(track, step, data);
}

uint8_t AcidSequencerClass::getStepData(uint8_t track, uint8_t step)
{
  // 808 request?
  if (track >= TRACK_NUMBER_303)
    return _engine808->getStepData(track-TRACK_NUMBER_303, step);
  
  // 303 request
  return _engine303->getStepData(track, step);
}

uint8_t AcidSequencerClass::getCurrentStep(uint8_t track)
{
  // 808 request?
  if (track >= TRACK_NUMBER_303)
    return _engine808->getCurrentStep(track-TRACK_NUMBER_303);
  
  // 303 request
  return _engine303->getCurrentStep(track);
}

uint8_t AcidSequencerClass::getTrackNumber()
{
  return TRACK_NUMBER_303 + TRACK_NUMBER_808;
}

bool AcidSequencerClass::is303(uint8_t track)
{
  if (track < TRACK_NUMBER_303) {
    return true;
  }

  return false;
}

uint8_t AcidSequencerClass::getTrackLength(uint8_t track)
{
  // 808 request?
  if (track >= TRACK_NUMBER_303)
    return _engine808->getTrackLength(track-TRACK_NUMBER_303);
    
  // 303 request
  return _engine303->getTrackLength(track);
}

void AcidSequencerClass::setTrackLength(uint8_t track, uint16_t length)
{
  // 808 request?
  if (track >= TRACK_NUMBER_303)
    _engine808->setTrackLength(track-TRACK_NUMBER_303, length);
  // 303 request
  else
    _engine303->setTrackLength(track, length);  
}

void AcidSequencerClass::setTrackVoice(uint8_t track, uint8_t voice)
{
  _engine808->setTrackVoice(track-TRACK_NUMBER_303, voice);
}

uint8_t AcidSequencerClass::getTrackVoice(uint8_t track)
{
  return _engine808->getTrackVoice(track-TRACK_NUMBER_303);
}

const char * AcidSequencerClass::getTrackVoiceName(uint8_t track, uint8_t voice)
{
  return _engine808->getTrackVoiceName(track-TRACK_NUMBER_303, voice);
}

void AcidSequencerClass::setMidiOutputCallback(void (*callback)(uint8_t msg_type, uint8_t byte1, uint8_t byte2, uint8_t channel, uint8_t port)) 
{
  _engine303->setMidiOutputCallback(callback);
  _engine808->setMidiOutputCallback(callback);
}

void AcidSequencerClass::acidRandomize(uint8_t track) 
{
  // 808 request?
  if (track >= TRACK_NUMBER_303)
    _engine808->acidRandomize(track-TRACK_NUMBER_303);
  // 303 request
  else
    _engine303->acidRandomize(track);
}

const char * AcidSequencerClass::getNoteString(uint8_t note)
{
  static char szBuffer[6];  
  uint8_t octave, interval;

  octave = floor(note/12);
  interval = floor(note%12);

  sprintf(szBuffer, "%s%d", _note_string_table[interval], octave+1);
  return szBuffer;
}

AcidSequencerClass AcidSequencer;