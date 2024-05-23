//
// RAW entrypoint for aciduino custom interface
//
void mainLoop()
{
    // read sequencer states and feedback info to the user
    // fastLed updates...
    // display updates...
    // any output feedback update...
}

void dinHandle(uint8_t control_id, uint16_t value, uint8_t subpage)
{ 
  // direct access to digital inputs(DIN and/or TOUCH)
  switch (control_id) {
    case 0:
        if (value == HIGH) {
            aciduino.playStop();
        }
    default:
        break;
  }   
}

void ainHandle(uint8_t control_id, uint16_t value, uint8_t subpage)
{
  // direct access to analog inputs reads
  switch (control_id) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
        aciduino.sendMidiCC(control_id, value);
    default:
        break;
  }   
}