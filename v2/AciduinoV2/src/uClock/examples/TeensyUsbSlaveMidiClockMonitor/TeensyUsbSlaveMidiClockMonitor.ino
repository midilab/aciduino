/* USB MIDI Sync Slave Box Monitor
 *  
 * This example demonstrates how to create a
 * MIDI hid compilant slave clock box with 
 * monitor support using oled displays
 *
 * Making use of a 250 usceconds timer to
 * handle MIDI input to avoid jitter on clock
 * 
 * You need the following libraries to make it work
 * - u8g2
 * - uClock
 *
 * This example code is in the public domain.
 */
 
#include <U8x8lib.h>

//
// BPM Clock support
//
#include <uClock.h>

U8X8 * u8x8;
IntervalTimer teensyTimer;

// MIDI clock, start, stop, note on and note off byte definitions - based on MIDI 1.0 Standards.
#define MIDI_CLOCK 0xF8
#define MIDI_START 0xFA
#define MIDI_STOP  0xFC

float bpm = 126;
uint8_t bpm_blink_timer = 1;
uint8_t clock_state = 1;

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
  usbMIDI.sendRealTime(MIDI_CLOCK);
  handle_bpm_led(tick);
}

void onClockStart() {
  usbMIDI.sendRealTime(MIDI_START);
}

void onClockStop() {
  usbMIDI.sendRealTime(MIDI_STOP);
  digitalWrite(LED_BUILTIN, LOW);
}

// External clock handlers
void onExternalClock()
{
  uClock.clockMe();
}

void onExternalStart()
{
  uClock.start();
}

void onExternalStop()
{
  uClock.stop();
}

void setup() {
  // A led to count bpms
  pinMode(LED_BUILTIN, OUTPUT);

  //
  // MIDI setup
  //
  usbMIDI.begin();
  usbMIDI.setHandleClock(onExternalClock);
  usbMIDI.setHandleStart(onExternalStart);
  usbMIDI.setHandleStop(onExternalStop);

  //
  // OLED setup
  // Please check you oled model to correctly init him
  //
  u8x8 = new U8X8_SH1106_128X64_NONAME_HW_I2C(U8X8_PIN_NONE);
  u8x8->begin();
  u8x8->setFont(u8x8_font_pressstart2p_r); 
  u8x8->clear();
  u8x8->drawUTF8(0, 0, "uClock"); 

  //
  // uClock Setup
  //
  // Setup our clock system
  uClock.init();
  uClock.setClock96PPQNOutput(ClockOut96PPQN);
  // For MIDI Sync Start and Stop
  uClock.setOnClockStartOutput(onClockStart);
  uClock.setOnClockStopOutput(onClockStop);
  uClock.setMode(uClock.EXTERNAL_CLOCK);
  // make use of 250us timer to handle midi input sync
  teensyTimer.begin(handleMidiInput, 250); 
  teensyTimer.priority(80);
}

void handleMidiInput() {
  while (usbMIDI.read()) {
  }
}

void loop() {
  if (bpm != uClock.getTempo()) {
    bpm = uClock.getTempo();
    u8x8->drawUTF8(8, 7, String(bpm, 1).c_str());
    u8x8->drawUTF8(8+5, 7, "bpm");
    // clear display ghost number for 2 digit
    // coming from 3 digit bpm changes
    if (bpm < 100) {
      u8x8->drawUTF8(8+4, 7, " ");
    }
  }
  if (clock_state != uClock.state) { 
    clock_state = uClock.state;
    if (clock_state >= 1) {
      u8x8->drawUTF8(0, 7, "playing"); 
    } else { 
      u8x8->drawUTF8(0, 7, "stoped "); 
    }
  }
}
