#ifdef USE_UART_MIDI
#include <MIDI.h>
// UART MIDI port 1
struct MidiDefaultSettings : public midi::DefaultSettings
{
    static const unsigned SysExMaxSize = 16; // Accept SysEx messages up to 1024 bytes long.
    static const bool UseRunningStatus = false; // My devices seem to be ok with it.
};

// initing midi devices
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI1);
#endif

// External clock handlers
void onExternalClock()
{
  if (uClock.getMode() == uClock.EXTERNAL_CLOCK)
    uClock.clockMe();
}

void onExternalStart()
{
  if (uClock.getMode() == uClock.EXTERNAL_CLOCK)
    uClock.start();
}

void onExternalStop()
{
  if (uClock.getMode() == uClock.EXTERNAL_CLOCK)
    uClock.stop();
}

void midiSetup()
{
#ifdef USE_USB_MIDI
  usbMIDI.begin();
  usbMIDI.setHandleClock(onExternalClock);
  usbMIDI.setHandleStart(onExternalStart);
  usbMIDI.setHandleStop(onExternalStop);
#endif
#ifdef USE_UART_MIDI
  MIDI1.begin();
  MIDI1.setHandleClock(onExternalClock);
  MIDI1.setHandleStart(onExternalStart);
  MIDI1.setHandleStop(onExternalStop);
#endif
}

// used by AcidSequencer object as callback to spill midi messages out
void midiOutHandler(uint8_t msg_type, uint8_t byte1, uint8_t byte2, uint8_t channel, uint8_t port)
{
#ifdef USE_USB_MIDI
  usbMIDI.send(msg_type, byte1, byte2, channel+1, 0);
#endif
#ifdef USE_UART_MIDI
  MIDI1.send(msg_type, byte1, byte2, channel+1);
#endif
}

// 3 realtime messages used by uClock object inside interruption
void sendMidiClock() {
#ifdef USE_USB_MIDI
  usbMIDI.sendRealTime(usbMIDI.Clock);
#endif
#ifdef USE_UART_MIDI
  MIDI1.sendRealTime(midi::Clock);
#endif
}

void sendMidiStart() {
#ifdef USE_USB_MIDI
  usbMIDI.sendRealTime(usbMIDI.Start);
#endif
#ifdef USE_UART_MIDI
  MIDI1.sendRealTime(midi::Start);
#endif
}

void sendMidiStop() {
#ifdef USE_USB_MIDI
  usbMIDI.sendRealTime(usbMIDI.Stop);
#endif
#ifdef USE_UART_MIDI
  MIDI1.sendRealTime(midi::Stop);
#endif
}

// called outside interruption by user request on PageComponent
// ATOMIC all of them!
void sendMidiCC(uint8_t cc, uint8_t value, uint8_t channel) {
#ifdef USE_USB_MIDI
  ATOMIC(usbMIDI.sendControlChange(cc, value, channel+1))
#endif
#ifdef USE_UART_MIDI
  ATOMIC(MIDI1.sendControlChange(cc, value, channel+1))
#endif
}

void sendNote(uint8_t note, uint8_t channel, uint8_t velocity) {
#ifdef USE_USB_MIDI
  if (velocity == 0) {
    ATOMIC(usbMIDI.sendNoteOff(note, 0, channel+1))
  } else {
    ATOMIC(usbMIDI.sendNoteOn(note, velocity, channel+1))
  }
#endif
#ifdef USE_UART_MIDI
  if (velocity == 0) {
    ATOMIC(MIDI1.sendNoteOff(note, 0, channel+1))
  } else {
    ATOMIC(MIDI1.sendNoteOn(note, velocity, channel+1))
  }
#endif
}

// used by uCtrl at 250us speed to get MIDI sync input messages on time
void midiInputHandle() {
#ifdef USE_USB_MIDI
  while (usbMIDI.read()) {
  }
#endif
#ifdef USE_UART_MIDI
  while (MIDI1.read()) {
  }
#endif
}

/*
void sendPreviewNote(uint8_t step)
{
  unsigned long milliTime, preMilliTime;
  uint8_t note;

  // enable or disable harmonizer
  if ( _harmonize == 1 ) {
    note = harmonizer(_sequencer[_selected_track].data.step[step].note);
  } else {
    note = _sequencer[_selected_track].data.step[step].note;
  }
  ATOMIC(sendMidiMessage(NOTE_ON, note, _sequencer[_selected_track].data.step[step].accent ? ACCENT_VELOCITY : NOTE_VELOCITY, _sequencer[_selected_track].channel))

  // avoid delay() call because of uClock timmer1 usage
  //delay(200);
  preMilliTime = millis();
  while ( true ) {
    milliTime = millis();
    if (abs(milliTime - preMilliTime) >= 200) {
      break;
    }
  }
  
  ATOMIC(sendMidiMessage(NOTE_OFF, note, 0, _sequencer[_selected_track].channel))
}
 */
