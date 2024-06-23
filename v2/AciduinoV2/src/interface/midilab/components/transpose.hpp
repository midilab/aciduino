struct Transpose : PageComponent {
  
    void view() {
      genericOptionView("transpose", String(aciduino.seq.getTranspose(aciduino.getSelectedTrack())), line, col, selected);
    }

    void change(int16_t data) {
      //clearStackNote(aciduino.getSelectedTrack());
      data = parseData(data, -12, 12, aciduino.seq.getTranspose(aciduino.getSelectedTrack()));
      aciduino.seq.setTranspose(aciduino.getSelectedTrack(), data);
    }
    
} transposeComponent;