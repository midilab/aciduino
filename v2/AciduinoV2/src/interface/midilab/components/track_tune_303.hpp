struct TrackTune : PageComponent {
  
    void view() {
      genericOptionView("tune", aciduino.seq.getTune(aciduino.getSelectedTrack()) == 0 ? String("off") : String(aciduino.seq.getNoteString(aciduino.seq.getTune(aciduino.getSelectedTrack())-1)), line, col, selected);
    }

    void change(int16_t data) {
      // 0 = harmonizer off
      // 1 = C..., 12 = B
      //clearStackNote(aciduino.getSelectedTrack());
      data = parseData(data, 0, 12, aciduino.seq.getTune(aciduino.getSelectedTrack()));
      aciduino.seq.setTune(aciduino.getSelectedTrack(), data);
    }
    
} tuneComponent;