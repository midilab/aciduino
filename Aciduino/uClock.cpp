/*!
 *  @file       uClock.cpp
 *  Project     BPM clock generator for Arduino
 *  @brief      A Library to implement BPM clock tick calls using hardware timer1 interruption. Tested on ATmega168/328, ATmega16u4/32u4 and ATmega2560.
 *              Derived work from mididuino MidiClock class. (c) 2008 - 2011 - Manuel Odendahl - wesen@ruinwesen.com
 *  @version    0.8.3
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

// TODO: use float for decimal timming precision: 120.10, 120.20
// each 0.1 bpm equals 15.625 intervals on 16Mhz clock
// each 1 bpm equals 156.250 intervals on 16Mhz clock

#include "uClock.h"

namespace umodular { namespace clock {

uint8_t _tmpSREG;

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
  // 11 is good for native 31250bps midi interface
  // 4 is good for usb-to-midi hid
  drift = 11;
  pll_x = 220;
  start_timer = 0;
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
  setTempo(120);
  //
  // Configure timers and prescale
  // Timmer1: ATMega128, ATMega328, AtMega16U4 and AtMega32U4
  _tmpSREG = SREG;
  cli();
  // Waveform Generation Mode (WGM) 16-bit timer settings
  // (WGM10, WGM12) Mode 5: Fast Pulse Width Modulation (PWM), 8-bit
  // Clock Speed Selection
  // CS10: Clock (No prescaling)  
  TCCR1A = _BV(WGM10);
  TCCR1B = _BV(CS10) | _BV(WGM12);
  TIMSK1 |= _BV(TOIE1);
  SREG = _tmpSREG;
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

void uClockClass::setTempo(uint16_t bpm) 
{
  if (mode == EXTERNAL_CLOCK) {
    return;
  }
  
  if (tempo == bpm) {
    return;
  }
  
  if (bpm > 300 || bpm == 0) {
    return;
  }
  
  _tmpSREG = SREG;
  cli();
  tempo = bpm;
  //interval = 62500 / (tempo * 24 / 60) - drift;
  interval = (uint16_t)(156250 / tempo) - drift;
  SREG = _tmpSREG;
}

uint16_t uClockClass::getTempo() 
{
  if (mode == EXTERNAL_CLOCK) {
    tempo = (156250 / interval);
  }
  return tempo;
}

void uClockClass::setDrift(uint8_t value)
{
  drift = value;
}

uint8_t uClockClass::getMode() 
{
  return mode;
}

void uClockClass::setMode(uint8_t tempo_mode) 
{
  mode = tempo_mode;
}

void uClockClass::clockMe() 
{
  if (mode == EXTERNAL_CLOCK) {
    handleClock();
  }
}

void uClockClass::resetCounters() 
{
  counter = 0;
  last_clock = 0;
  div96th_counter = 0;
  div32th_counter = 0;
  div16th_counter = 0;
  mod6_counter = 0;
  indiv96th_counter = 0;
  inmod6_counter = 0;
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

void uClockClass::handleClock() 
{
  uint16_t cur_clock = _clock;
  uint16_t diff = clock_diff(last_clock, cur_clock);
  
  last_interval = diff;
  last_clock = cur_clock;
  indiv96th_counter++;
  inmod6_counter++;

  if (inmod6_counter == 6) {
    inmod6_counter = 0;
  }

  switch (state) {

    case PAUSED:
      break;

    case STARTING:
      state = STARTED;
      break;

    case STARTED:
      if (indiv96th_counter == 2) {
        interval = diff;
      } else {
        interval = (((uint32_t)interval * (uint32_t)pll_x) + (uint32_t)(256 - pll_x) * (uint32_t)diff) >> 8;
      }
      break;
  }

}

void uClockClass::handleTimerInt()  
{
  if (counter == 0) {
    
    counter = interval;

    if (onClock96PPQNCallback) {
      onClock96PPQNCallback(&div96th_counter);
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
    
    div96th_counter++;
    mod6_counter++;
  
    if (mode == EXTERNAL_CLOCK) {
      uint16_t cur_clock = _clock;
      uint16_t diff = clock_diff(last_clock, cur_clock);
      if ((div96th_counter < indiv96th_counter) || (div96th_counter > (indiv96th_counter + 1))) {
        div96th_counter = indiv96th_counter;
        mod6_counter = inmod6_counter;
      }
      if (div96th_counter <= indiv96th_counter) {
        counter -= phase_mult(diff);
      } else {
        if (counter > diff) {
          counter += phase_mult(counter - diff);
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
// TIMER1 HANDLER INTERRUPT
//
ISR(TIMER1_OVF_vect) 
{
  // global timer counter
  _timer = millis();
  
  if (uClock.state == umodular::clock::STARTED) {
    _clock++;
    uClock.handleTimerInt();
  }
}



