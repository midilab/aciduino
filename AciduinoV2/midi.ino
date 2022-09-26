// All midi interface registred thru uCtrl get incomming data thru callback
void midiInputHandler(uctrl::protocol::midi::MIDI_MESSAGE * msg, uint8_t port, uint8_t interrupted) 
{
  if (uClock.getMode() == uClock.EXTERNAL_CLOCK) {
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

void midiOutHandler(uint8_t msg_type, uint8_t byte1, uint8_t byte2, uint8_t channel, uint8_t port)
{
  Serial.println(channel);
  msg.type = msg_type;
  msg.data1 = byte1;
  msg.data2 = byte2;
  msg.channel = channel;
  uCtrl.midi->write(&msg, 1);
}

// read midi input at port 1 each 250us
void midiHandle() {
  while (uCtrl.midi->read(1)) {
  }
}
