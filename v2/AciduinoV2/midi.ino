
// midi handler
uctrl::protocol::midi::MIDI_MESSAGE msg;
uctrl::protocol::midi::MIDI_MESSAGE msg_interrupt;

uint8_t _port = 1;

// All midi interface registred thru uCtrl get incomming data thru callback
void midiInputHandler(uctrl::protocol::midi::MIDI_MESSAGE * msg, uint8_t port, uint8_t interrupted) 
{
  // control goes thru selected track(no matter what channel)
  if ( uClock.getMode() == uClock.EXTERNAL_CLOCK ) {
    // external tempo control
    switch (msg->type) {
        case uctrl::protocol::midi::Clock:
          uClock.clockMe();
          return;
 
        case uctrl::protocol::midi::Start:
          uClock.start();
          return;
  
        case uctrl::protocol::midi::Stop:
          uClock.stop();
          return;
    }  
  }
}

// used by AcidSequencer object as callback to spill midi messages out
void midiSequencerOutHandler(uint8_t msg_type, uint8_t byte1, uint8_t byte2, uint8_t channel, uint8_t port)
{
  msg.type = msg_type == NOTE_ON ? uctrl::protocol::midi::NoteOn : uctrl::protocol::midi::NoteOff;
  msg.data1 = byte1;
  msg.data2 = byte2;
  msg.channel = channel;
  uCtrl.midi->write(&msg, port+1, 1);
}

// 3 realtime messages used by uClock object inside interruption
void sendMidiClock() {
  msg_interrupt.type = uctrl::protocol::midi::Clock;
  uCtrl.midi->writeAllPorts(&msg_interrupt, 1);
}

void sendMidiStart() {
  msg.type = uctrl::protocol::midi::Start;
  uCtrl.midi->writeAllPorts(&msg, 0);  
}

void sendMidiStop() {
  msg.type = uctrl::protocol::midi::Stop;
  uCtrl.midi->writeAllPorts(&msg, 0);
}

// called outside interruption by user request on PageComponent
// ATOMIC all of them!
void sendMidiCC(uint8_t cc, uint8_t value, uint8_t channel) {
  msg.type = uctrl::protocol::midi::ControlChange;
  msg.data1 = cc;
  msg.data2 = value;
  msg.channel = channel;
  uCtrl.midi->write(&msg, _port, 0);
}

void sendNote(uint8_t note, uint8_t channel, uint8_t velocity) {
  msg.data1 = note;
  msg.data2 = velocity;
  msg.channel = channel;
  
  if (velocity == 0) {
    msg.type = uctrl::protocol::midi::NoteOff;
  } else {
    msg.type = uctrl::protocol::midi::NoteOn;
  }

   uCtrl.midi->write(&msg, _port, 0);
}

// a port to read midi notes 1ms
void midiHandle() {
  while (uCtrl.midi->read(2)) {
  }
}

// used by uCtrl at 250us speed to get MIDI sync input messages on time
void midiHandleSync() {
  while (uCtrl.midi->read(1)) {
  }
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
