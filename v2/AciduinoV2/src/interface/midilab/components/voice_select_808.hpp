struct VoiceSelect : PageComponent {
  
    void view() {
      genericOptionView("voice", String(aciduino.seq.getTrackVoiceName(aciduino.getSelectedTrack(), aciduino.seq.getTrackVoice(aciduino.getSelectedTrack()))), line, col, selected);
    }

    void change(int16_t data) {
      //clearStackNote(aciduino.getSelectedTrack());
      // create a getter for track voice size
      data = parseData(data, 0, VOICE_MAX_SIZE_808-1, aciduino.seq.getTrackVoice(aciduino.getSelectedTrack()));
      aciduino.seq.setTrackVoice(aciduino.getSelectedTrack(), data);
    }
    
} voiceSelectComponent;