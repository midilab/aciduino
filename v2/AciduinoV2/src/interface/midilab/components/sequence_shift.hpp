struct SequenceShift : PageComponent {
  
    void view() {
      genericOptionView("shift", String(aciduino.seq.getShiftPos(aciduino.getSelectedTrack())), line, col, selected);
    }

    void change(int16_t data) {
      //clearStackNote(aciduino.getSelectedTrack());
      data = parseData(data, aciduino.seq.getTrackLength(aciduino.getSelectedTrack())*-1, aciduino.seq.getTrackLength(aciduino.getSelectedTrack()), aciduino.seq.getShiftPos(aciduino.getSelectedTrack()));
      aciduino.seq.setShiftPos(aciduino.getSelectedTrack(), data);
    }
    
} shiftComponent;