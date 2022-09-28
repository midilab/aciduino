//#include <MIDI.h>
// UART MIDI port 1
//MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI1);

void handleMidiInput() {
  while (usbMIDI.read()) {
  }
}

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

void midiOutHandler(uint8_t msg_type, uint8_t byte1, uint8_t byte2, uint8_t channel, uint8_t port)
{
  usbMIDI.send(msg_type, byte1, byte2, channel+1, 0);
}

void sendMidiClock() {
  usbMIDI.sendRealTime(usbMIDI.Clock);
}

void sendMidiStart() {
  usbMIDI.sendRealTime(usbMIDI.Start);
}

void sendMidiStop() {
  usbMIDI.sendRealTime(usbMIDI.Stop);
}
  
// read midi input at port 1 each 250us by uCtrl
void midiHandle() {
  while (usbMIDI.read()) {
  }
}
