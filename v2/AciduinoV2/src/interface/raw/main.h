//
// RAW entrypoint for aciduino custom interface
//
void mainLoop()
{
  // read sequencer states and feedback info to the user

  // fastLed updates...
  // select the voice to view step data
  // uint8_t voice = 0;
  //aciduino.seq.setTrackVoice(track, voice);
  // returns if the step/voice is on or off state
  //aciduino.seq.stepOn(track, step)
  
  // display updates...
  // any output feedback update...
}

void dinHandle(uint8_t control_id, uint16_t value, uint8_t subpage)
{ 
  // direct access to digital inputs(DIN and/or TOUCH)
  switch (control_id) {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
      if (value == HIGH) {
        uint16_t step = control_id;
        uint8_t track = aciduino.getSelectedTrack();
        uint8_t voice = 0;
        // select the voice to control
        aciduino.seq.setTrackVoice(track, voice);
        // toggles step on/off: rest(track, step, state)
        aciduino.seq.rest(track, step, aciduino.seq.stepOn(track, step));
      }
      break;
    case 16:
      if (value == HIGH) {
        // toggles playStop state
        aciduino.playStop();
      }
      break;
    default:
      break;
  }   
}

void ainHandle(uint8_t control_id, uint16_t value, uint8_t subpage)
{
  // direct access to analog inputs reads
  switch (control_id) {
    case 0:
      // aciduino.sendMidiCC(CC, value, channel, port)
      aciduino.sendMidiCC(40, value, 0, 0);
      break;
    case 1:
      aciduino.sendMidiCC(41, value, 0, 0);
      break;
    case 2:
      aciduino.sendMidiCC(42, value, 0, 0);
      break;
    case 3:
      aciduino.sendMidiCC(43, value, 0, 0);
      break;
    case 4:
      aciduino.sendMidiCC(44, value, 0, 0);
      break;
    default:
      break;
  }   
}