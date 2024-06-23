struct VoiceConfig : PageComponent {
  
    void view() {
      genericOptionView("note", String(aciduino.seq.getNoteString(aciduino.seq.getTrackVoiceConfig(aciduino.getSelectedTrack()))), line, col, selected);
    }

    void change(int16_t data) {
      //clearStackNote(aciduino.getSelectedTrack());
      data = parseData(data, 0, 127, aciduino.seq.getTrackVoiceConfig(aciduino.getSelectedTrack()));
      aciduino.seq.setTrackVoiceConfig(aciduino.getSelectedTrack(), data);
      // send note for preview while change data
      aciduino.sendNote(data, aciduino.getTrackOutputParam(TRACK_CHANNEL), aciduino.getTrackOutputParam(TRACK_PORT), NOTE_VELOCITY_808);
      aciduino.sendNote(data, aciduino.getTrackOutputParam(TRACK_CHANNEL), aciduino.getTrackOutputParam(TRACK_PORT), 0);
    }
    
} voiceConfigComponent;
