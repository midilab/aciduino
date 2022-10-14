/*!
 *  @file       acid_sequencer.h
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

#ifndef __ACID_SEQUENCER_H__
#define __ACID_SEQUENCER_H__

#include <Arduino.h>
#include "setup.h"

// acid sequencers modules
#include "engine_303.h"
#include "engine_808.h"

// Acid StepSequencer, 
// a modular roland classic machines sequencer engine clone
// TB303, TR808
// each module can be setup output(MIDI, CV/GATE) a page to config will be nice!
// author: midilab contact@midilab.co
// under MIT license

// idea for 808 roll 
// use a flag or value for note_off to get into loop inside 96ppqn call with 
// note on and off controlled until it finished the step

class AcidSequencerClass
{
    private:

      Engine303 _engine303;
      Engine808 _engine808;
      
      uint8_t _selected_pattern = 0;

      const char * _note_string_table[12] = {"c", "c#", "d", "d#", "e", "f", "f#", "g", "g#", "a", "a#", "b"};

    public:

      AcidSequencerClass();

      // The callback function wich will be called by uClock each Pulse of 16PPQN clock resolution. Each call represents exactly one step.
      void on16PPQN(uint32_t tick);
      void on96PPQN(uint32_t tick);
      // The callback function wich will be called when clock starts by using Clock.start() method.
      void onClockStart();
      // The callback function wich will be called when clock stops by using Clock.stop() method.
      void onClockStop();
      void clearStackNote(int8_t track = -1);

      // general interface for UI/Sequencer for 303 and 808 generic access
      void setTrackChannel(uint8_t track, uint8_t channel);
      void rest(uint8_t track, uint8_t step, bool state);
      bool stepOn(uint8_t track, uint8_t step);
      void setStepData(uint8_t track, uint8_t step, uint8_t data);
      uint8_t getStepData(uint8_t track, uint8_t step);
      bool accentOn(uint8_t track, uint8_t step);
      void setAccent(uint8_t track, uint8_t step, bool state);
      void setShiftPos(uint8_t track, int8_t shift);
      int8_t getShiftPos(uint8_t track);
      uint8_t getCurrentStep(uint8_t track);
      uint8_t getTrackChannel(uint8_t track);
      uint8_t getTrackPort(uint8_t track) { return 0; };
      uint8_t getTrackNumber();
      uint8_t getTrackMaxLength(uint8_t track);
      uint8_t getTrackLength(uint8_t track);
      void setTrackLength(uint8_t track, uint16_t length);
      void clearTrack(uint8_t track);
      void acidRandomize(uint8_t track, uint8_t fill, uint8_t param_1 = 0, uint8_t param_2 = 0, uint8_t param_3 = 0, uint8_t param_4 = 0, uint8_t param_5 = 0, uint8_t param_6 = 0);
      bool is303(uint8_t track);

      // 303 specific
      uint8_t getTune(uint8_t track);
      void setTune(uint8_t track, uint8_t tune);
      int8_t getTranspose(uint8_t track);
      void setTranspose(uint8_t track, int8_t transpose);
      void setSlide(uint8_t track, uint8_t step, bool state);
      void setTie(uint8_t track, uint8_t step, bool state);
      bool slideOn(uint8_t track, uint8_t step);
      bool tieOn(uint8_t track, uint8_t step);
      const char * getTemperamentName(uint8_t temperament_id);
      void setTemperament(uint8_t temperament_id);
      uint8_t getTemperamentId();
      const char * getNoteString(uint8_t note);

      // 808 specific
      void setRoll(uint8_t track, uint8_t step, bool state);
      void setRollType(uint8_t track, uint8_t type);
      uint8_t getRollType(uint8_t track);
      bool rollOn(uint8_t track, uint8_t step);
      void setTrackVoice(uint8_t track = 0, uint8_t voice = 0);
      uint8_t getTrackVoice(uint8_t track);
      void setTrackVoiceConfig(uint8_t track, uint8_t note);
      uint8_t getTrackVoiceConfig(uint8_t track);
      const char * getTrackVoiceName(uint8_t track = 0, uint8_t voice = 0);

      // main callback outputs... CV ouput for next release please!
      void setMidiOutputCallback(void (*callback)(uint8_t msg_type, uint8_t byte1, uint8_t byte2, uint8_t channel, uint8_t port));
};

extern AcidSequencerClass AcidSequencer;

#endif