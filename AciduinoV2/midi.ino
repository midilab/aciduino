void midiOutHandler(uint8_t msg_type, uint8_t byte1, uint8_t byte2, uint8_t channel, uint8_t port)
{
  msg.type = msg_type;
  msg.data1 = byte1;
  msg.data2 = byte2;
  msg.channel = channel;
  uCtrl.midi->write(&msg, 1);
}
