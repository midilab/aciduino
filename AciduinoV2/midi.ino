//#include <MIDI.h>
// UART MIDI port 1
//MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI1);

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
  usbMIDI.begin();
  usbMIDI.setHandleClock(onExternalClock);
  usbMIDI.setHandleStart(onExternalStart);
  usbMIDI.setHandleStop(onExternalStop);
}

// used by AcidSequencer object as callback to spill midi messages out
void midiOutHandler(uint8_t msg_type, uint8_t byte1, uint8_t byte2, uint8_t channel, uint8_t port)
{
  usbMIDI.send(msg_type, byte1, byte2, channel+1, 0);
}

// used by uClock object inside interruption
void sendMidiClock() {
  usbMIDI.sendRealTime(usbMIDI.Clock);
}

void sendMidiStart() {
  usbMIDI.sendRealTime(usbMIDI.Start);
}

void sendMidiStop() {
  usbMIDI.sendRealTime(usbMIDI.Stop);
}

// called outside interruption by user request on PageComponent
void sendMidiCC(uint8_t cc, uint8_t value, uint8_t channel) {
  ATOMIC(usbMIDI.sendControlChange(cc, value, channel+1))
}
  
// used by uCtrl at 250us speed to get MIDI sync input messages on time
void midiInputHandle() {
  while (usbMIDI.read()) {
  }
}
