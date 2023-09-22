void initSequencer()
{
  // init default track output data
  for(uint8_t track=0; track < TRACK_NUMBER_303+TRACK_NUMBER_808; track++) {
    _track_output_setup[track].output = MIDI_OUTPUT;
    _track_output_setup[track].channel = track;
    _track_output_setup[track].port = 0;
  }
  
  // the acid sequencer main output callback
  AcidSequencer.setOutputCallback(sequencerOutHandler);

  // we're going to init epprom in two cases:
  // if track layout changes || if no aciduino data present at epprom
  // so if you want to keep your pattenrs and change layout consider backup pattenrs into another sequencer
  // because they will be wipeout for the new memory layout to take place
  if (checkEppromDataLayoutChange()) {
    eppromInit();
  }

  // load shuffle form memory pattern?
  int8_t init_template[2] = {0,1};
  uClock.setShuffleTemplate(init_template);

  // load epprom saved session
  loadSession();
  // load first pattern
  loadPattern(0);
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


void playStop()
{
  if (_playing)
    uClock.stop();
  else
    uClock.start();
}

void recToggle()
{
  AcidSequencer.setRecStatus(!AcidSequencer.getRecStatus());
}

void previousTrack()
{
  if (_selected_track == 0) {
    _selected_track = AcidSequencer.getTrackNumber() - 1;
  } else {
    --_selected_track;
  }
}

void nextTrack()
{
  if (_selected_track == AcidSequencer.getTrackNumber() - 1) {
    _selected_track = 0;
  } else {
    ++_selected_track;
  }
}