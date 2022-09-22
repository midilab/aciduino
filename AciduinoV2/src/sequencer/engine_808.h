/*!
 *  @file       engine_808.h
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

#ifndef __ENGINE_808_H__
#define __ENGINE_808_H__

#include "setup.h"
#include "engine.h"

// helper
#define GET_BIT(a,n) ((a >> n)  & 0x01)  
#define SET_BIT(a,n) (a |=  (1ULL<<n))
#define CLR_BIT(a,n) (a &= ~(1ULL<<n))

// For pre-processing plugins handle data
// The event can be a hole step with linked event data or just a single event data
typedef struct
{
  uint8_t note;  
  int8_t length;       
} STACK_NOTE_DATA_808;  

typedef struct
{
  uint64_t steps; // 8 bytes, 64 steps max
  uint64_t accent; // 8 bytes, 64 steps max
  uint64_t roll; // 8 bytes, 64 steps max
  uint8_t note;
  uint8_t step_length;
  int8_t shift;
  char name[MAX_VOICE_NAME_CHARS];
} VOICE_DATA;  // 27 bytes

typedef struct
{
  uint8_t step_location;
  uint8_t step_length;
  int8_t shift;
  uint8_t channel;
  bool mute;
  // euclidian voices linked list, only percusion voice types will make use of more than one voice.
  VOICE_DATA voice[VOICE_MAX_SIZE_808];
  STACK_NOTE_DATA_808 stack[VOICE_MAX_SIZE_808];
} SEQUENCER_TRACK_808;

class Engine808 : Engine
{
    public:

      void setTrackChannel(uint8_t track, uint8_t channel);
      void init();

      void rest(uint8_t track, uint8_t step, bool state);
      bool stepOn(uint8_t track, uint8_t step);
      bool accentOn(uint8_t track, uint8_t step);
      bool rollOn(uint8_t track, uint8_t step);
      void setAccent(uint8_t track, uint8_t step, bool state);
      void setRoll(uint8_t track, uint8_t step, bool state);
      void setStepData(uint8_t track, uint8_t step, uint8_t data);
      uint8_t getStepData(uint8_t track, uint8_t step);
      uint8_t getCurrentStep(uint8_t track);
      uint8_t getTrackLength(uint8_t track);
      void setTrackLength(uint8_t track, uint16_t length);
      void setTrackVoice(uint8_t track = 0, uint8_t voice = 0);
      uint8_t getTrackVoice(uint8_t track = 0);
      const char * getTrackVoiceName(uint8_t track = 0, uint8_t voice = 0);
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
      uint8_t _voice = 0;

      SEQUENCER_TRACK_808 volatile _sequencer[TRACK_NUMBER_808];
};

#endif