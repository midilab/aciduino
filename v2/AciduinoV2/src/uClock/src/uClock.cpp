/*!
 *  @file       uClock.cpp
 *  Project     BPM clock generator for Arduino
 *  @brief      A Library to implement BPM clock tick calls using hardware timer interruption. Tested on ATmega168/328, ATmega16u4/32u4, ATmega2560, Teensy ARM boards and Seedstudio XIAO M0
 *  @version    1.1.0
 *  @author     Romulo Silva
 *  @date       04/03/2022
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
#include "uClock.h"

// 
// Timer setup for work clock
//
// all non-avr timmers setup
// Teensyduino port
#if defined(TEENSYDUINO)
IntervalTimer _uclockTimer;
#endif
// Seedstudio XIAO M0 port
#if defined(SEEED_XIAO_M0)
// 24 bits timer
#include <TimerTCC0.h>
#define _uclockTimer TimerTcc0
// 16 bits timer
//#include <TimerTC3.h>
//#define _uclockTimer TimerTc3
#endif

#if defined(ARDUINO_ARCH_AVR)
void uclockInitTimer()
{
	ATOMIC(
		// 16bits Timer1 init
		// begin at 120bpm (48.0007680122882 Hz)
		TCCR1A = 0; // set entire TCCR1A register to 0
		TCCR1B = 0; // same for TCCR1B
		TCNT1  = 0; // initialize counter value to 0
		// set compare match register for 48.0007680122882 Hz increments
		OCR1A = 41665; // = 16000000 / (8 * 48.0007680122882) - 1 (must be <65536)
		// turn on CTC mode
		TCCR1B |= (1 << WGM12);
		// Set CS12, CS11 and CS10 bits for 8 prescaler
		TCCR1B |= (0 << CS12) | (1 << CS11) | (0 << CS10);
		// enable timer compare interrupt
		TIMSK1 |= (1 << OCIE1A);
	)
}
#else
void uclockISR();
void uclockInitTimer()
{
	// begin at 120bpm (20833us)
	const uint16_t init_clock = 20833;
	#if defined(TEENSYDUINO)
		_uclockTimer.begin(uclockISR, init_clock); 

		// Set the interrupt priority level, controlling which other interrupts
		// this timer is allowed to interrupt. Lower numbers are higher priority, 
		// with 0 the highest and 255 the lowest. Most other interrupts default to 128. 
		// As a general guideline, interrupt routines that run longer should be given 
		// lower priority (higher numerical values).
		_uclockTimer.priority(0);
	#endif

	#if defined(SEEED_XIAO_M0)
		_uclockTimer.initialize(init_clock);

		// attach to generic uclock ISR
		_uclockTimer.attachInterrupt(uclockISR);
	#endif
}
#endif

namespace umodular { namespace clock {

static inline uint32_t phase_mult(uint32_t val) 
{
	return (val * PHASE_FACTOR) >> 8;
}

static inline uint32_t clock_diff(uint32_t old_clock, uint32_t new_clock) 
{
	if (new_clock >= old_clock) {
		return new_clock - old_clock;
	} else {
		return new_clock + (4294967295 - old_clock);
	}
}

uClockClass::uClockClass()
{
	tempo = 120;
	start_timer = 0;
	last_interval = 0;
	sync_interval = 0;
	state = PAUSED;
	mode = INTERNAL_CLOCK;
	resetCounters();

	onClock96PPQNCallback = NULL;
	onClock32PPQNCallback = NULL;
	onClock16PPQNCallback = NULL;
	onClockStartCallback = NULL;
	onClockStopCallback = NULL;
}

void uClockClass::init() 
{
	uclockInitTimer();
	// first interval calculus
	setTempo(tempo);
}

void uClockClass::start() 
{
	resetCounters();
	start_timer = millis();
	
	if (onClockStartCallback) {
		onClockStartCallback();
	}	
	
	if (mode == INTERNAL_CLOCK) {
		state = STARTED;
	} else {
		state = STARTING;
	}	
}

void uClockClass::stop()
{
	state = PAUSED;
	start_timer = 0;
	resetCounters();
	if (onClockStopCallback) {
		onClockStopCallback();
	}
}

void uClockClass::pause() 
{
	if (mode == INTERNAL_CLOCK) {
		if (state == PAUSED) {
			start();
		} else {
			stop();
		}
	}
}

void uClockClass::setTimerTempo(float bpm) 
{
	// 96 ppqn resolution
	uint32_t tick_us_interval = (60000000 / 24 / bpm);

#if defined(ARDUINO_ARCH_AVR)
	float tick_hertz_interval = 1/((float)tick_us_interval/1000000);

	uint32_t ocr;
	uint8_t tccr = 0;

	// 16bits avr timer setup
	if ((ocr = AVR_CLOCK_FREQ / ( tick_hertz_interval * 1 )) < 65535) {
		// Set CS12, CS11 and CS10 bits for 1 prescaler
		tccr |= (0 << CS12) | (0 << CS11) | (1 << CS10);
	} else if ((ocr = AVR_CLOCK_FREQ / ( tick_hertz_interval * 8 )) < 65535) {
		// Set CS12, CS11 and CS10 bits for 8 prescaler
		tccr |= (0 << CS12) | (1 << CS11) | (0 << CS10);
	} else if ((ocr = AVR_CLOCK_FREQ / ( tick_hertz_interval * 64 )) < 65535) {
		// Set CS12, CS11 and CS10 bits for 64 prescaler
		tccr |= (0 << CS12) | (1 << CS11) | (1 << CS10);
	} else if ((ocr = AVR_CLOCK_FREQ / ( tick_hertz_interval * 256 )) < 65535) {
		// Set CS12, CS11 and CS10 bits for 256 prescaler
		tccr |= (1 << CS12) | (0 << CS11) | (0 << CS10);
	} else if ((ocr = AVR_CLOCK_FREQ / ( tick_hertz_interval * 1024 )) < 65535) {
		// Set CS12, CS11 and CS10 bits for 1024 prescaler
		tccr |= (1 << CS12) | (0 << CS11) | (1 << CS10);
	} else {
		// tempo not achiavable
		return;
	}

	ATOMIC(
		TCCR1B = 0;
		OCR1A = ocr-1;
		TCCR1B |= (1 << WGM12);
		TCCR1B |= tccr;
	)
#else
	#if defined(TEENSYDUINO)
		_uclockTimer.update(tick_us_interval);
	#endif

	#if defined(SEEED_XIAO_M0)
		_uclockTimer.setPeriod(tick_us_interval);
	#endif
#endif
}

void uClockClass::setTempo(float bpm) 
{
	if (mode == EXTERNAL_CLOCK) {
		return;
	}
	
	if (bpm < MIN_BPM || bpm > MAX_BPM) {
		return;
	}

	ATOMIC(
		tempo = bpm
	)

	setTimerTempo(bpm);
	
}

float inline uClockClass::freqToBpm(uint32_t freq)
{
	float usecs = 1/((float)freq/1000000.0);
	return (float)((float)(usecs/24.0) * 60.0);
}

float uClockClass::getTempo() 
{
	if (mode == EXTERNAL_CLOCK) {
		uint32_t acc = 0;
		// wait the buffer get full
		if (ext_interval_buffer[EXT_INTERVAL_BUFFER_SIZE-1] == 0) {
			return tempo;
		}
		for (uint8_t i=0; i < EXT_INTERVAL_BUFFER_SIZE; i++) {
			acc += ext_interval_buffer[i];
		}
		if (acc != 0) {
			return freqToBpm(acc / EXT_INTERVAL_BUFFER_SIZE);
		}
	}
	return tempo;
}

void uClockClass::setMode(uint8_t tempo_mode) 
{
	mode = tempo_mode;
}

uint8_t uClockClass::getMode() 
{
	return mode;
}

void uClockClass::clockMe() 
{
	if (mode == EXTERNAL_CLOCK) {
		ATOMIC(
			handleExternalClock()
		)
	}
}

void uClockClass::resetCounters() 
{
	external_clock = 0;
	internal_tick = 0;
	external_tick = 0;
	div32th_counter = 0;
	div16th_counter = 0;
	mod6_counter = 0;	
	indiv32th_counter = 0;
	indiv16th_counter = 0;
	inmod6_counter = 0;
	ext_interval_idx = 0;
	for (uint8_t i=0; i < EXT_INTERVAL_BUFFER_SIZE; i++) {
		ext_interval_buffer[i] = 0;
	}
}

// TODO: Tap stuff
void uClockClass::tap() 
{
	// tap me
}

// TODO: Shuffle stuff
void uClockClass::shuffle() 
{
	// shuffle me
}

void uClockClass::handleExternalClock() 
{

	switch (state) {
		case PAUSED:
			break;

		case STARTING:
			state = STARTED;
			external_clock = micros();
			break;

		case STARTED:

			uint32_t u_timer = micros();
			last_interval = clock_diff(external_clock, u_timer);
			external_clock = u_timer;

			if (inmod6_counter == 0) {
				indiv16th_counter++;
				indiv32th_counter++;
			}

			if (inmod6_counter == 3) {
				indiv32th_counter++;
			}

			// slave tick me!
			external_tick++;
			inmod6_counter++;

			if (inmod6_counter == 6) {
				inmod6_counter = 0;
			}

			// accumulate interval incomming ticks data for getTempo() smooth reads on slave mode
			if(++ext_interval_idx >= EXT_INTERVAL_BUFFER_SIZE) {
				ext_interval_idx = 0;
			}
			ext_interval_buffer[ext_interval_idx] = last_interval;

			if (external_tick == 1) {
				interval = last_interval;
			} else {
				interval = (((uint32_t)interval * (uint32_t)PLL_X) + (uint32_t)(256 - PLL_X) * (uint32_t)last_interval) >> 8;
			}
			break;
	}
}

void uClockClass::handleTimerInt()  
{
	if (mode == EXTERNAL_CLOCK) {
		// sync tick position with external tick clock
		if ((internal_tick < external_tick) || (internal_tick > (external_tick + 1))) {
			internal_tick = external_tick;
			div32th_counter = indiv32th_counter;
			div16th_counter = indiv16th_counter;
			mod6_counter = inmod6_counter;
		}

		uint32_t counter = interval;
		uint32_t u_timer = micros();
		sync_interval = clock_diff(external_clock, u_timer);

		if (internal_tick <= external_tick) {
			counter -= phase_mult(sync_interval);
		} else {
			if (counter > sync_interval) {
				counter += phase_mult(counter - sync_interval);
			}
		}

		// update internal clock timer frequency
		float bpm = freqToBpm(counter);
		if (bpm != tempo) {
			if (bpm >= MIN_BPM && bpm <= MAX_BPM) {
				tempo = bpm;
				setTimerTempo(bpm);
			}
		}
	}

	if (onClock96PPQNCallback) {
		onClock96PPQNCallback(internal_tick);
	}

	if (mod6_counter == 0) {
		if (onClock32PPQNCallback) {
			onClock32PPQNCallback(div32th_counter);
		}
		if (onClock16PPQNCallback) {
			onClock16PPQNCallback(div16th_counter);
		}
		div16th_counter++;
		div32th_counter++;
	}

	if (mod6_counter == 3) {
		if (onClock32PPQNCallback) {
			onClock32PPQNCallback(div32th_counter);
		}
		div32th_counter++;
	}

	// tick me!
	internal_tick++;
	mod6_counter++;

	if (mod6_counter == 6) {
		mod6_counter = 0;
	}
	
}

// elapsed time support
uint8_t uClockClass::getNumberOfSeconds(uint32_t time)
{
	if ( time == 0 ) {
		return time;
	}
	return ((_timer - time) / 1000) % SECS_PER_MIN;
}

uint8_t uClockClass::getNumberOfMinutes(uint32_t time)
{
	if ( time == 0 ) {
		return time;
	}	
	return (((_timer - time) / 1000) / SECS_PER_MIN) % SECS_PER_MIN;
}

uint8_t uClockClass::getNumberOfHours(uint32_t time)
{
	if ( time == 0 ) {
		return time;
	}	
	return (((_timer - time) / 1000) % SECS_PER_DAY) / SECS_PER_HOUR;
}

uint8_t uClockClass::getNumberOfDays(uint32_t time)
{
	if ( time == 0 ) {
		return time;
	}	
	return ((_timer - time) / 1000) / SECS_PER_DAY;
}

uint32_t uClockClass::getNowTimer()
{
	return _timer;
}
	
uint32_t uClockClass::getPlayTime()
{
	return start_timer;
}
	
} } // end namespace umodular::clock

umodular::clock::uClockClass uClock;

volatile uint32_t _timer = 0;

//
// TIMER INTERRUPT HANDLER 
// 
//
#if defined(ARDUINO_ARCH_AVR)
ISR(TIMER1_COMPA_vect)
#else
void uclockISR() 
#endif
{
	// global timer counter
	_timer = millis();
	
	if (uClock.state == uClock.STARTED) {
		uClock.handleTimerInt();
	}
}
