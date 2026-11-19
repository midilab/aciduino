#ifndef __SEQ_HELPERS_H__
#define __SEQ_HELPERS_H__

#if defined(ARDUINO)
#include <Arduino.h> // random(), noInterrupts(), FreeRTOS on ESP32
#else
#include <stdlib.h>  // rand/RAND_MAX for host builds
#endif

//
// small shared helpers for the note based sequencer and its facade
//

// bit helpers
#define GET_BIT(a,n)  (((a) >> (n)) & 0x01)
#define SET_BIT(a,n)  ((a) |=  ((uint32_t)1 << (n)))
#define CLR_BIT(a,n)  ((a) &= ~((uint32_t)1 << (n)))

// MIDI clock, start, stop, note on and note off byte definitions - MIDI 1.0
#ifndef NOTE_ON
#define NOTE_ON    0x90
#endif
#ifndef NOTE_OFF
#define NOTE_OFF   0x80
#endif

// uClock defines ATOMIC on supported platforms when it is included first;
// otherwise mirror its per platform semantics(ESP32 shares the uClock mutex,
// the clock callbacks run on the other core there).
#ifndef ATOMIC
#if defined(ARDUINO_ARCH_ESP32) || defined(ESP32)
  extern SemaphoreHandle_t _mutex;
  #define ATOMIC(X) xSemaphoreTake(_mutex, portMAX_DELAY); X; xSemaphoreGive(_mutex);
#elif defined(ARDUINO)
  #define ATOMIC(X) noInterrupts(); { X; } interrupts();
#else
  #define ATOMIC(X) do { X; } while (0)
#endif
#endif

// portable random in [lo, hi]
static inline uint16_t seq_random(uint16_t lo, uint16_t hi)
{
#if defined(ARDUINO)
  return random(lo, hi);
#else
  return lo + (uint16_t)(((uint32_t)rand() * (hi - lo + 1)) / (RAND_MAX + 1u));
#endif
}

#endif
