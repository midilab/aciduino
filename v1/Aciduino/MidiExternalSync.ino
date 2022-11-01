// MIDI clock, start, stop, note on and note off byte definitions - based on MIDI 1.0 Standards.
#define MIDI_CLOCK 0xF8
#define MIDI_START 0xFA
#define MIDI_STOP  0xFC

byte in_data = 0X00;

// avr general timer2 - 8bits
// 250usecs timmer to read midi input clock/control messages
void setExternalSync(bool on)
{
  if (on == true) {
    noInterrupts();
    TCCR2A = 0; // set entire TCCR2A register to 0
    TCCR2B = 0; // same for TCCR2B
    TCNT2  = 0; // initialize counter value to 0
    // set compare match register for 4000 Hz increments
    OCR2A = 124; // = 16000000 / (32 * 4000) - 1 (must be <256)
    // turn on CTC mode
    TCCR2B |= (1 << WGM21);
    // Set CS22, CS21 and CS20 bits for 32 prescaler
    TCCR2B |= (0 << CS22) | (1 << CS21) | (1 << CS20);
    // enable timer compare interrupt
    TIMSK2 |= (1 << OCIE2A);
    interrupts();
    uClock.setMode(uClock.EXTERNAL_CLOCK);
  } else {
    uClock.setMode(uClock.INTERNAL_CLOCK);
    noInterrupts();
    // disable timer compare interrupt
    TIMSK2 |= (0 << OCIE2A);
    interrupts();
  }
}

ISR(TIMER2_COMPA_vect, ISR_NOBLOCK) 
{
  if(Serial.available() > 0) {
    in_data = Serial.read();
    switch (in_data) {
        case MIDI_CLOCK:
          uClock.clockMe();
          return;
        case MIDI_START:
          uClock.start();
          return;
        case MIDI_STOP:
          uClock.stop();
          return;
        default:
          return;
    }
  } 
}

