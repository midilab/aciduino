/*!
 *  @file       uClock.cpp
 *  Project     BPM clock generator for Arduino
 *  @brief      A Library to implement BPM clock tick calls using hardware timer1 interruption. Tested on ATmega168/328, ATmega16u4/32u4 and ATmega2560.
 *              Derived work from mididuino MidiClock class. (c) 2008 - 2011 - Manuel Odendahl - wesen@ruinwesen.com
 *  @version    0.8
 *  @author     Romulo Silva
 *  @date       10/06/17
 *  @license    MIT - (c) 2017 - Romulo Silva - contact@midilab.co
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

#ifndef __U_CLOCK_H__
#define __U_CLOCK_H__

#include <Arduino.h>
#include <inttypes.h>

#define SECS_PER_MIN  (60UL)
#define SECS_PER_HOUR (3600UL)
#define SECS_PER_DAY  (SECS_PER_HOUR * 24L)

namespace umodular { namespace clock {

class uClockClass {

	public:

		void (*onClock96PPQNCallback)(uint32_t * tick);
		void (*onClock32PPQNCallback)(uint32_t * tick);
		void (*onClock16PPQNCallback)(uint32_t * tick);
		void (*onClockStartCallback)();
		void (*onClockStopCallback)();

		uint32_t div96th_counter;
		uint32_t div32th_counter;
		uint32_t div16th_counter;
		uint8_t mod6_counter;
		uint8_t inmod6_counter;
		uint16_t interval;
		uint16_t counter;
		uint16_t last_clock;
		uint16_t last_interval;
		uint32_t indiv96th_counter;
		uint16_t pll_x;
		uint16_t tempo;
		uint32_t start_timer;

		enum {
			PAUSED = 0,
			STARTING,
			STARTED
		} state;

		enum {
			INTERNAL_CLOCK = 0,
			EXTERNAL_CLOCK
		} mode;

		uClockClass();

		void setClock96PPQNOutput(void (*callback)(uint32_t * tick)) {
			onClock96PPQNCallback = callback;
		}

		void setClock32PPQNOutput(void (*callback)(uint32_t * tick)) {
			onClock32PPQNCallback = callback;
		}

		void setClock16PPQNOutput(void (*callback)(uint32_t * tick)) {
			onClock16PPQNCallback = callback;
		}

		void setOnClockStartOutput(void (*callback)()) {
			onClockStartCallback = callback;
		}

		void setOnClockStopOutput(void (*callback)()) {
			onClockStopCallback = callback;
		}

		void init();
		void handleClock();
		void handleTimerInt();
		
		// external class control
		void start();
		void stop();
		void pause();
		void setTempo(uint16_t tempo);
		uint16_t getTempo();

		// External timming control
		void setMode(uint8_t tempo_mode);
		uint8_t getMode();
		void clockMe();
		
		void shuffle();
		void tap();
		
		// elapsed time support
		uint8_t getNumberOfSeconds(uint32_t time);
		uint8_t getNumberOfMinutes(uint32_t time);
		uint8_t getNumberOfHours(uint32_t time);
		uint8_t getNumberOfDays(uint32_t time);
		uint32_t getNowTimer();
		uint32_t getPlayTime();

};

} } // end namespace umodular::clock

extern umodular::clock::uClockClass uClock;

extern "C" {
extern volatile uint16_t _clock;
extern volatile uint32_t _timer;
}

#endif /* __U_CLOCK_H__ */

