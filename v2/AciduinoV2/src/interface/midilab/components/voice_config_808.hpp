struct VoiceConfig : PageComponent {
  
    void view() {
      genericOptionView("note", String(aciduino.seq.getNoteString(aciduino.seq.getTrackVoiceConfig(aciduino.getSelectedTrack()))), line, col, selected);
    }

    void change(int16_t data) {
      //clearStackNote(aciduino.getSelectedTrack());
      data = parseData(data, 0, 127, aciduino.seq.getTrackVoiceConfig(aciduino.getSelectedTrack()));
      aciduino.seq.setTrackVoiceConfig(aciduino.getSelectedTrack(), data);
      // send note for preview while change data
      aciduino.sendNote(data, _track_output_setup[aciduino.getSelectedTrack()].channel, _track_output_setup[aciduino.getSelectedTrack()].port, NOTE_VELOCITY_808);
      aciduino.sendNote(data, _track_output_setup[aciduino.getSelectedTrack()].channel, _track_output_setup[aciduino.getSelectedTrack()].port, 0);
    }
    
} voiceConfigComponent;
