/* USB MIDI Sync Box
 *  
 * This example demonstrates how to change the USB MIDI 
 * device name on Seeedstudio XIAO M0. 
 * 
 * This example code is in the public domain.
 * 
 * Tested with Adafruit TinyUSB version 0.10.5
 * 
 */
#include <Adafruit_TinyUSB.h>
#include <MIDI.h>

Adafruit_USBD_MIDI usb_midi;
MIDI_CREATE_INSTANCE(Adafruit_USBD_MIDI, usb_midi, MIDI_USB);

//MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);
#include <uClock.h>

uint8_t bpm_blink_timer = 1;
void handle_bpm_led(uint32_t tick)
{
  // BPM led indicator
  if ( !(tick % (96)) || (tick == 1) ) {  // first compass step will flash longer
    bpm_blink_timer = 8;
    digitalWrite(LED_BUILTIN, LOW);
  } else if ( !(tick % (24)) ) {   // each quarter led on
    bpm_blink_timer = 1;
    digitalWrite(LED_BUILTIN, LOW);
  } else if ( !(tick % bpm_blink_timer) ) { // get led off
    digitalWrite(LED_BUILTIN, HIGH);
  }
}

// Internal clock handlers
void ClockOut96PPQN(uint32_t tick) {
  // Send MIDI_CLOCK to external gears
  MIDI_USB.sendRealTime(midi::Clock);
  handle_bpm_led(tick);
}

void onClockStart() {
  MIDI_USB.sendRealTime(midi::Start);
}

void onClockStop() {
  MIDI_USB.sendRealTime(midi::Stop);
}

void setup() {
  MIDI_USB.begin(MIDI_CHANNEL_OMNI);

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
