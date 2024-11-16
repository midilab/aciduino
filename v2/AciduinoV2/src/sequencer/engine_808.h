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

// based on 96PPQN place the pulse location of each FLAM option
typedef enum {
  FLAM_1 = 1,
  FLAM_2 = 3,
  FLAM_3 = 6,
  FLAM_4 = 9,
  FLAM_5 = 13,
  SUB_STEP_1,
  SUB_STEP_2,  
} ROLL_TYPE;

template <typename T> 
struct VOICE_DATA
{
  T steps; // 8 bytes, 64 steps max
#ifndef GLOBAL_ACCENT
  T accent; // 8 bytes, 64 steps max
#endif
  T roll; // 8 bytes, 64 steps max
  uint8_t note:7;
  uint8_t mute:1;
  uint8_t step_length;
  int8_t shift;
  char name[MAX_VOICE_NAME_CHARS];
  int8_t trigger_ctrl; 
};  // 48 bytes + 8bytes accent + char name[] for pattern voice
// track with 11 voices 616bytes or 561bytes with global accent

template <typename T> 
struct SEQUENCER_TRACK_808
{
  uint8_t step_location;
  uint8_t step_length;
  int8_t shift;
  //uint8_t channel;
  //uint8_t port;
  uint8_t mute;
  uint8_t roll_type;
#ifdef GLOBAL_ACCENT
  T accent;
#endif
  VOICE_DATA<T> voice[VOICE_MAX_SIZE_808];
};
// 48 bytes + 8bits accent + 

typedef struct
{
	const char * name;
  // for midi
	uint8_t note;
} CTRL_DATA; 
  
class Engine808 : public Engine
{
    public:
      void init();

      void rest(uint8_t track, uint8_t step, bool state);
      bool stepOn(uint8_t track, uint8_t step);
      bool accentOn(uint8_t track, uint8_t step);
      bool rollOn(uint8_t track, uint8_t step);
      void setAccent(uint8_t track, uint8_t step, bool state);
      void setRoll(uint8_t track, uint8_t step, bool state);
      void setRollType(uint8_t track, uint8_t type);
      uint8_t getRollType(uint8_t track);
      void setStepData(uint8_t track, uint8_t step, uint8_t data);
      uint8_t getStepData(uint8_t track, uint8_t step);
      uint8_t getCurrentStep(uint8_t track);
      uint16_t getTrackLength(uint8_t track);
      void setShiftPos(uint8_t track, int8_t shift);
      int8_t getShiftPos(uint8_t track);
      void setTrackLength(uint8_t track, uint16_t length);
      void setMute(uint8_t track, uint8_t voice, uint8_t mute);
      void setMute(uint8_t track, uint8_t mute);
      uint8_t getMute(uint8_t track, uint8_t voice);
      uint8_t getMute(uint8_t track);
      void clearStepData(uint8_t track, uint8_t voice);
      void clearTrack(uint8_t track, uint8_t mode = 1);
      void setTrackVoice(uint8_t track, uint8_t voice = 0);
      uint8_t getTrackVoice(uint8_t track);
      void setTrackVoiceConfig(uint8_t track, uint8_t note);
      uint8_t getTrackVoiceConfig(uint8_t track);
      int8_t getTrackVoiceByNote(uint8_t track, uint8_t note);
      const char * getTrackVoiceName(uint8_t track, uint8_t voice = 0);
      void acidRandomize(uint8_t track, uint8_t fill, uint8_t accent_probability, uint8_t roll_probability);
      // The callback function wich will be called by uClock each Pulse of 16PPQN clock resolution. Each call represents exactly one step.
      void onStepCall(uint32_t tick, int8_t shuffle_length_ctrl);
      void * getPatternData(uint8_t track);
      uint16_t getPatternMemorySize();
      uint16_t getPatternTrackSize();

      // The callback function wich will be called by uClock each Pulse of 96PPQN clock resolution.
      void on96PPQNCall(uint32_t tick);
      void clearStackNote(int8_t track);

    private:
#if STEP_MAX_SIZE_808 <= 16
      volatile SEQUENCER_TRACK_808<uint16_t> _sequencer[TRACK_NUMBER_808];
		  Bjorklund<uint16_t, 10> _bjorklund;
#elif STEP_MAX_SIZE_808 <= 32
      volatile SEQUENCER_TRACK_808<uint32_t> _sequencer[TRACK_NUMBER_808];
 		  Bjorklund<uint32_t, 10> _bjorklund;
#elif STEP_MAX_SIZE_808 <= 64
      volatile SEQUENCER_TRACK_808<uint64_t> _sequencer[TRACK_NUMBER_808];
		  Bjorklund<uint64_t, 10> _bjorklund;
#endif

      uint8_t _voice = 0;
      CTRL_DATA _default_voice_data_808[11] = {
            {"bd", 36},
            {"sd", 38},
            {"lt", 41},
            {"mt", 45},
            {"ht", 48},
            {"rs", 37},
            {"cp", 39},
            {"cb", 51},
            {"cy", 49},
            {"oh", 46},
            {"ch", 42}
          };

};

#endif