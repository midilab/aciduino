
void initSequencer()
{
  for(uint8_t track=0; track < TRACK_NUMBER_303+TRACK_NUMBER_808; track++) {
    _track_output_setup[track].output = MIDI_OUTPUT;
    _track_output_setup[track].channel = track;
    _track_output_setup[track].port = 0;
  }
  // the acid sequencer main output
  AcidSequencer.setOutputCallback(sequencerOutHandler);

  // we going to init epprom in two cases:
  // if track layout changes || if no aciduino data present at epprom
  // so if you want to keep your pattenrs and change layout consider backup pattenrs into another sequencer
  // because they will be wipeout for the new memory layout to take place
  if (checkEppromDataLayoutChange()) {
    eppomInit();
  }
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
