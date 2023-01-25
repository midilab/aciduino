
void initSequencer()
{
  for(uint8_t track=0; track < TRACK_NUMBER_303+TRACK_NUMBER_808; track++) {
    _track_output_setup[track].output = MIDI_OUTPUT;
    _track_output_setup[track].channel = track;
    _track_output_setup[track].port = 0;
  }
  // the acid sequencer main output
  AcidSequencer.setOutputCallback(sequencerOutHandler);
}

// used by AcidSequencer object as callback to spill data out
void sequencerOutHandler(uint8_t msg_type, uint8_t note, uint8_t velocity, uint8_t track)
{
  switch(_track_output_setup[track].output) {
    case MIDI_OUTPUT:
      midiSequencerOutHandler(msg_type, note, velocity, _track_output_setup[track].channel, _track_output_setup[track].port);
      break;
    case CV_OUTPUT:
      // soon boy...
      break;
    case GATE_OUTPUT:
      // soooooooon...
      break;
  }
}
