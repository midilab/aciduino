/* minimal Arduino.h stub for host syntax-checking of the facade */
#ifndef ARDUINO_STUB_H
#define ARDUINO_STUB_H
#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef uint8_t byte;
typedef bool boolean;

#define HIGH 1
#define LOW 0
#define B11011100 0b11011100
#define B10111010 0b10111010
#define B01110110 0b01110110
#define B11101100 0b11101100
#define B11011010 0b11011010
#define B10110110 0b10110110
#define B01101110 0b01101110
#define B10111100 0b10111100
#define B01111010 0b01111010
#define B11110100 0b11110100
#define B11101010 0b11101010
#define B11010110 0b11010110
#define B10101110 0b10101110
#define B01011110 0b01011110

inline void noInterrupts() {}
inline void interrupts() {}
inline void delay(unsigned long) {}
inline unsigned long micros() { return 0; }
inline unsigned long millis() { return 0; }
inline int random(int lo, int hi) { return lo + (int)(rand() % (unsigned)(hi - lo + 1)); }

#endif
