/*!
 *  @file       engine_303.h
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

#ifndef __ENGINE_303_H__
#define __ENGINE_303_H__

#include "setup.h"
#include "engine.h"
#include "harmonizer.h"
#include "bjorklund.h"

// sequencer data
typedef struct
{
  uint8_t note;
  int16_t length;
} STACK_NOTE_DATA_303;

typedef struct
{
  uint8_t note:7;
  uint8_t accent:1;
  uint8_t slide:1;
  uint8_t rest:1;
  uint8_t reserved:6;
} SEQUENCER_STEP_DATA_303;
// 2 bytes per step

// or
// octave: 2 bits: 4 values / 3 bits: 8 values
// note: 4 bits: 16 values

typedef struct
{
  SEQUENCER_STEP_DATA_303 step[STEP_MAX_SIZE_303];
  int8_t shift;
  uint8_t step_length;
  int8_t transpose;
} SEQUENCER_TRACK_DATA_303;
// 32 bytes per 16 step + 2 bytes config = 34 bytes [STEP_MAX_SIZE_303=16]

typedef struct
{
  SEQUENCER_TRACK_DATA_303 data;
  uint8_t step_location;
  uint8_t channel;
  bool mute;
  STACK_NOTE_DATA_303 stack[NOTE_STACK_SIZE_303];  
} SEQUENCER_TRACK_303;

class Engine303 : Engine
{
    public:

      void init();

      void rest(uint8_t track, uint8_t step, bool state);
      bool stepOn(uint8_t track, uint8_t step);
      bool accentOn(uint8_t track, uint8_t step);
      bool slideOn(uint8_t track, uint8_t step);
      void setAccent(uint8_t track, uint8_t step, bool state);
      void setSlide(uint8_t track, uint8_t step, bool state);
      void setStepData(uint8_t track, uint8_t step, uint8_t data);
      uint8_t getStepData(uint8_t track, uint8_t step);
      uint8_t getCurrentStep(uint8_t track);
      uint8_t getTrackLength(uint8_t track);
      void setTrackLength(uint8_t track, uint16_t length);
      void setTrackChannel(uint8_t track, uint8_t channel);
      void acidRandomize(uint8_t track);

      // The callback function wich will be called by uClock each Pulse of 16PPQN clock resolution. Each call represents exactly one step.
      void onStepCall(uint32_t tick);

      // The callback function wich will be called by uClock each Pulse of 96PPQN clock resolution.
      void onClockCall(uint32_t tick);

      void clearStackNote(int8_t track);

      void setMidiOutputCallback(void (*callback)(uint8_t msg_type, uint8_t byte1, uint8_t byte2, uint8_t channel, uint8_t port)) {
        _onMidiEventCallback = callback;
      }

    private:

      SEQUENCER_TRACK_303 volatile _sequencer[TRACK_NUMBER_303];

      uint8_t _lower_note = 26;
      uint8_t _range_note = 23;
      uint8_t _accent_probability = ACCENT_PROBABILITY_GENERATION_303;
      uint8_t _slide_probability = SLIDE_PROBABILITY_GENERATION_303;
      uint8_t _rest_probability = REST_PROBABILITY_GENERATION_303;
      uint8_t _number_of_tones = 5;
      uint8_t _harmonize = 0;

};

#endif