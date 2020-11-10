/*!
 *  @file       uClock.cpp
 *  Project     BPM clock generator for Arduino
 *  @brief      A Library to implement BPM clock tick calls using hardware timer1 interruption. Tested on ATmega168/328, ATmega16u4/32u4 and ATmega2560.
 *              Derived work from mididuino MidiClock class. (c) 2008 - 2011 - Manuel Odendahl - wesen@ruinwesen.com
 *  @version    0.10.5
 *  @author     Romulo Silva
 *  @date       08/21/2020
 *  @license    MIT - (c) 2020 - Romulo Silva - contact@midilab.co
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
#if defined(TEENSYDUINO) && !defined(__AVR_ATmega32U4__)
IntervalTimer _uclockTimer;
void uclockISR();
void uclockInitTimer()
{
	_uclockTimer.begin(uclockISR, 16);
	// Set the interrupt priority level, controlling which other interrupts
	// this timer is allowed to interrupt. Lower numbers are higher priority, 
	// with 0 the highest and 255 the lowest. Most other interrupts default to 128. 
	// As a general guideline, interrupt routines that run longer should be given 
	// lower priority (higher numerical values).
	_uclockTimer.priority(0);
}
#else
void uclockInitTimer()
{
	ATOMIC(
		// Timer1
		TCCR1A = 0;
		TCCR1B = 0;
		TCNT1  = 0;
		// set the speed of our internal clock system
		OCR1A = 255;
		// turn on CTC mode
		TCCR1B |= (1 << WGM12);
		// Set CS12, CS11 and CS10 bits for 1 prescaler
		TCCR1B |= (0 << CS12) | (0 << CS11) | (1 << CS10);
		// enable timer compare interrupt
		TIMSK1 |= (1 << OCIE1A);
	)
	/*
	ATOMIC(
		// Timer2
		TCCR2A = 0;
		TCCR2B = 0;
		TCNT2  = 0;
		// set the speed of our internal clock system
		OCR2A = 255;
		// turn on CTC mode
		TCCR2B |= (1 << WGM21);
		// Set CS22, CS21 and CS20 bits for 1 prescaler
		TCCR2B |= (0 << CS22) | (0 << CS21) | (1 << CS20);
		// enable timer compare interrupt
		TIMSK2 |= (1 << OCIE2A);
	)
	*/
}
#endif

namespace umodular { namespace clock {

static inline uint32_t phase_mult(uint32_t val) 
{
	return (val * PHASE_FACTOR) >> 8;
}

static inline uint16_t clock_diff(uint16_t old_clock, uint16_t new_clock) 
{
	if (new_clock >= old_clock) {
		return new_clock - old_clock;
	} else {
		return new_clock + (65535 - old_clock);
	}
}

uClockClass::uClockClass()
{
	// drift is used to sligth calibrate with your slave clock
	drift = 1;
	slave_drift = 0;
	pll_x = 220;
	tempo = 120;
	start_timer = 0;
	last_interval = 0;
	sync_interval = 0;
	state = PAUSED;
	mode = INTERNAL_CLOCK;
	ext_interval_acc = 0;
	resetCounters();

	onClock96PPQNCallback = NULL;
	onClock32PPQNCallback = NULL;
	onClock16PPQNCallback = NULL;
	onClockStartCallback = NULL;
	onClockStopCallback = NULL;

	// first interval calculus
	setTempo(tempo);
}

void uClockClass::init() 
{
	// init work clock timer interrupt at 16 microseconds
	uclockInitTimer();
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

void uClockClass::setTempo(float bpm) 
{
	if (mode == EXTERNAL_CLOCK) {
		return;
	}
	
	if (bpm < MIN_BPM || bpm > MAX_BPM) {
		return;
	}

	tempo = bpm;

	ATOMIC(
		//interval = (freq_resolution / (tempo * 24 / 60)) - drift;
		//interval = 62500 / (tempo * 24 / 60) - drift;
		interval = (uint16_t)((156250.0 / tempo) - drift);
	)
}

float uClockClass::getTempo() 
{
	if (mode == EXTERNAL_CLOCK) {
		uint32_t acc = 0;
		uint8_t acc_counter = 0;
		for (uint8_t i=0; i < EXT_INTERVAL_BUFFER_SIZE; i++) {
			if ( ext_interval_buffer[i] != 0) {
				acc += ext_interval_buffer[i];
				++acc_counter;
			}
		}
		if (acc != 0) {
			// get average interval, because MIDI sync world is a wild place...
			//tempo = (((float)freq_resolution/24) * 60) / (float)(acc / acc_counter);
			// derivated one time calc value = ( freq_resolution / 24 ) * 60
			tempo = (float)(156250.0 / (acc / acc_counter));
		}
	}
	return tempo;
}

void uClockClass::setDrift(uint8_t value)
{
	ATOMIC(drift = value)
	// force set tempo to update runtime interval
	setTempo(tempo);
}

void uClockClass::setSlaveDrift(uint8_t value)
{
	ATOMIC(slave_drift = value)
}

uint8_t uClockClass::getDrift()
{
	return drift;
}

// each interval is 16us
// this method is usefull for debug
uint16_t uClockClass::getInterval()
{
	return interval;
}

// Main poolling tick call
uint8_t uClockClass::getTick(uint32_t * tick)
{
	ATOMIC(
		uint32_t last_tick = internal_tick;
	)
	if (*tick != last_tick) {
		*tick = last_tick;
		return 1;
	}
	if (last_tick - *tick > 1) {
		*tick++;
		return 1;
	}
	return 0;
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
	counter = 0;
	last_clock = 0;
	internal_tick = 0;
	external_tick = 0;
	div32th_counter = 0;
	div16th_counter = 0;
	mod6_counter = 0;
	inmod6_counter = 0;
	ext_interval_idx = 0;
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
	last_interval = clock_diff(last_clock, _clock);
	last_clock = _clock;

	// accumulate interval incomming ticks data for getTempo() smooth reads on slave mode
	ext_interval_buffer[ext_interval_idx++ % EXT_INTERVAL_BUFFER_SIZE] = last_interval;	
	
	// slave tick me!
	external_tick++;

	switch (state) {
		case PAUSED:
			break;

		case STARTING:
			state = STARTED;
			break;

		case STARTED:
			if (external_tick == 1) {
				interval = last_interval;
			} else {
				interval = (((uint32_t)interval * (uint32_t)pll_x) + (uint32_t)(256 - pll_x) * (uint32_t)last_interval) >> 8;
			}
			break;
	}
}

void uClockClass::handleTimerInt()  
{
	if (counter == 0) {
		// update internal clock base counter
		counter = interval;

		// need a callback?
		// please, use the polling method with getTick() instead...
		if (onClock96PPQNCallback) {
			onClock96PPQNCallback(&internal_tick);
		}

		if (mod6_counter == 0) {
			if (onClock32PPQNCallback) {
				onClock32PPQNCallback(&div32th_counter);
			}
			if (onClock16PPQNCallback) {
				onClock16PPQNCallback(&div16th_counter);
			}
			div16th_counter++;
			div32th_counter++;
		}

		if (mod6_counter == 3) {
			if (onClock32PPQNCallback) {
				onClock32PPQNCallback(&div32th_counter);
			}
			div32th_counter++;
		}

		// tick me!
		internal_tick++;
		mod6_counter++;

		if (mode == EXTERNAL_CLOCK) {
			sync_interval = clock_diff(last_clock, _clock);
			if ((internal_tick < external_tick) || (internal_tick > (external_tick + 1))) {
				internal_tick = external_tick;
			}
			if (internal_tick <= external_tick) {
				counter -= phase_mult(sync_interval);
			} else {
				if (counter > sync_interval) {
					counter += phase_mult(counter - sync_interval);
				}
			}
		}
		
		if (mod6_counter == 6) {
			mod6_counter = 0;
		}

	} else {
		counter--;
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

volatile uint16_t _clock = 0;
volatile uint32_t _timer = 0;

//
// TIMER INTERRUPT HANDLER 
// Clocked at: 62.5kHz/16usec
//
#if defined(TEENSYDUINO) && !defined(__AVR_ATmega32U4__)
void uclockISR() 
#else
ISR(TIMER1_COMPA_vect) 
//ISR(TIMER2_COMPA_vect) 
#endif
{
	// global timer counter
	_timer = millis();
	
	if (uClock.state == uClock.STARTED) {
		_clock++;
		uClock.handleTimerInt();
	}
}
