// ussing 3 midi_messages data structure to
// keep interrupted and non interrupted memory area safe
uctrl::protocol::midi::MIDI_MESSAGE msg;
uctrl::protocol::midi::MIDI_MESSAGE msg_interrupt;
uctrl::protocol::midi::MIDI_MESSAGE msg_interrupt_pots;

volatile uint8_t _midi_clock_port = 0; // 0 = internal
volatile uint8_t _midi_rec_port = 1;

// All midi interface registred thru uCtrl get incomming data thru this callback
void midiInputHandler(uctrl::protocol::midi::MIDI_MESSAGE * msg, uint8_t port, uint8_t interrupted) 
{
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

  // control goes thru selected track(no matter what channel)
  switch (msg->type) {

    //
    // Note ON/OFF
    //  
    case uctrl::protocol::midi::NoteOn:
    case uctrl::protocol::midi::NoteOff:
      AcidSequencer.input(_selected_track, msg->type == uctrl::protocol::midi::NoteOn ? NOTE_ON : NOTE_OFF, msg->data1, msg->data2, interrupted);
      break;

    //
    // Control Change
    //
    case uctrl::protocol::midi::ControlChange:

      // CC MAP Control
      switch ( msg->data1 ) {

        // Sustain Controller 64
        case 64:
          //if (_foot_pedal_rec == true) {
          //  if ( msg->data2 == 127 ) {
              //footSwitchHandle(HIGH);
          //  } else if ( msg->data2 == 0 ) {
              //footSwitchHandle(LOW);
          //  }
          //  return;
          //}
          break;
      }
      break;
      
    default:
      break;
      
  }  
}

// used by AcidSequencer object as callback to spill midi messages out
void midiSequencerOutHandler(uint8_t msg_type, uint8_t byte1, uint8_t byte2, uint8_t channel, uint8_t port)
{
  msg_interrupt.type = msg_type == NOTE_ON ? uctrl::protocol::midi::NoteOn : uctrl::protocol::midi::NoteOff;
  msg_interrupt.data1 = byte1;
  msg_interrupt.data2 = byte2;
  msg_interrupt.channel = channel;
  uCtrl.midi->write(&msg_interrupt, port+1, 1);
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

// this is used inside interruption and outside, controlled by interrupted var
void sendMidiCC(uint8_t cc, uint8_t value, uint8_t channel, uint8_t port, uint8_t interrupted = 0) {
  if (interrupted) {
    msg_interrupt_pots.type = uctrl::protocol::midi::ControlChange;
    msg_interrupt_pots.data1 = cc;
    msg_interrupt_pots.data2 = value;
    msg_interrupt_pots.channel = channel;
    uCtrl.midi->write(&msg_interrupt_pots, port+1, 0);
  } else {
    msg.type = uctrl::protocol::midi::ControlChange;
    msg.data1 = cc;
    msg.data2 = value;
    msg.channel = channel;
    uCtrl.midi->write(&msg, port+1, interrupted);
  }
}

void sendNote(uint8_t note, uint8_t channel, uint8_t port, uint8_t velocity) {
  msg.data1 = note;
  msg.data2 = velocity;
  msg.channel = channel;
  
  if (velocity == 0) {
    msg.type = uctrl::protocol::midi::NoteOff;
  } else {
    msg.type = uctrl::protocol::midi::NoteOn;
  }

   uCtrl.midi->write(&msg, port+1, 0);
}

// a port to read midi notes 1ms
void midiHandle() {
  //while (uCtrl.midi->read(2)) {
  //}
  uCtrl.midi->read(_midi_rec_port);
}

// used by uCtrl at 250us speed to get MIDI sync input messages on time
void midiHandleSync() {
  if (_midi_clock_port > 0) {
    //while (uCtrl.midi->read(_midi_clock_port)) {
    //}
    uCtrl.midi->read(_midi_clock_port);
  }
}
