/* USB MIDI Sync Box
 *  
 * This example demonstrates how to change the USB MIDI 
 * device name on Teensy LC and 3.x.  When creating more
 * that one MIDI device, custom names are much easier to
 * use when selecting each device in MIDI software on
 * your PC or Mac.  The custom name is in the "name.c" tab.
 *
 * Windows and Macintosh systems often cache USB info.
 * After changing the name, you may need to test on a
 * different computer to observe the new name, or take
 * steps to get your operating system to "forget" the
 * cached info.  (TODO: wanted... can anyone contribute
 * instructions for these systems)
 * 
 * You must select MIDI from the "Tools > USB Type" menu
 * 
 * This example code is in the public domain.
 */

#include <uClock.h>

uint8_t bpm_blink_timer = 1;
void handle_bpm_led(uint32_t tick)
{
  // BPM led indicator
  if ( !(tick % (96)) || (tick == 1) ) {  // first compass step will flash longer
    bpm_blink_timer = 8;
    digitalWrite(LED_BUILTIN, HIGH);
  } else if ( !(tick % (24)) ) {   // each quarter led on
    digitalWrite(LED_BUILTIN, HIGH);
  } else if ( !(tick % bpm_blink_timer) ) { // get led off
    digitalWrite(LED_BUILTIN, LOW);
    bpm_blink_timer = 1;
  }
}

// Internal clock handlers
void ClockOut96PPQN(uint32_t tick) {
  // Send MIDI_CLOCK to external gears
  usbMIDI.sendRealTime(usbMIDI.Clock);
  handle_bpm_led(tick);
}

void onClockStart() {
  usbMIDI.sendRealTime(usbMIDI.Start);
}

void onClockStop() {
  usbMIDI.sendRealTime(usbMIDI.Stop);
}

void setup() {
  // A led to count bpms
  pinMode(LED_BUILTIN, OUTPUT);
  
  // Setup our clock system
  // Inits the clock
  uClock.init();
  // Set the callback function for the clock output to send MIDI Sync message.
  uClock.setClock96PPQNOutput(ClockOut96PPQN);
  // Set the callback function for MIDI Start and Stop messages.
  uClock.setOnClockStartOutput(onClockStart);  
  uClock.setOnClockStopOutput(onClockStop);
  // Set the clock BPM to 126 BPM
  uClock.setTempo(126);
  // Starts the clock, tick-tac-tick-tac...
  uClock.start();
}

// Do it whatever to interface with Clock.stop(), Clock.start(), Clock.setTempo() and integrate your environment...
void loop() {

}
