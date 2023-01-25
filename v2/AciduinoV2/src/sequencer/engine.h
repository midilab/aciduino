/*!
 *  @file       engine.h
 *  Project     Acid Sequencer
 *  @brief      Step Sequencer engine base class for acid step emulations
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

#ifndef __SEQUENCER_ENGINE_H__
#define __SEQUENCER_ENGINE_H__

//#include <stdint.h>
#include <Arduino.h>

#include "bjorklund.h"

//
// multicore archs
//
#if defined(ARDUINO_ARCH_ESP32) || defined(ESP32)
  portMUX_TYPE _acidEngine303TimerMux = portMUX_INITIALIZER_UNLOCKED;
	#define ATOMIC(X) portENTER_CRITICAL_ISR(&_acidEngine303TimerMux); X; portEXIT_CRITICAL_ISR(&_acidEngine303TimerMux);
//
// singlecore archs
//
#else
	#define ATOMIC(X) noInterrupts(); X; interrupts();
#endif

// helper
#define GET_BIT(a,n) ((a >> n)  & 0x01)  
#define SET_BIT(a,n) (a |=  (1ULL<<n))
#define CLR_BIT(a,n) (a &= ~(1ULL<<n))

class Engine
{
  protected:

		void (*_onEventCallback)(uint8_t msg_type, uint8_t note, uint8_t velocity, uint8_t track);

  public:

    // main component view
    virtual void init() {}

    virtual void onStepCall(uint32_t tick) {}

    virtual void onClockCall(uint32_t tick) {}

    virtual void clearStackNote() {}

    virtual void setTrackChannel(uint8_t track, uint8_t channel) {}

    virtual void setTrackLength(uint8_t track, uint16_t length) {}
    virtual uint16_t getTrackLength(uint8_t track) {}

    virtual void rest(uint8_t track, uint8_t step, bool state) {}

    virtual bool stepOn(uint8_t track, uint8_t step) {}

    virtual void setStepData(uint8_t track, uint8_t step, uint8_t data) {}

    virtual uint8_t getStepData(uint8_t track, uint8_t step) {}

    virtual uint8_t getCurrentStep(uint8_t track) {}

    // 808 only
    virtual void setTrackVoice(uint8_t track = 0, uint8_t voice = 0) {}
    virtual uint8_t getTrackVoice(uint8_t track) {}
    virtual const char * getTrackVoiceName(uint8_t track = 0, uint8_t voice = 0) {}

    void setOutputCallback(void (*callback)(uint8_t msg_type, uint8_t note, uint8_t velocity, uint8_t track)) {
      _onEventCallback = callback;
    }
};

#endif