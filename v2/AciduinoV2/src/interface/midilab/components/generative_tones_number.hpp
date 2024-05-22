struct TonesNumber : PageComponent {

    void view() {
      genericOptionView("tones", String(_generative_303[aciduino.getSelectedTrack()].number_of_tones), line, col, selected);
    }

    void change(int16_t data) {
      data = parseData(data, 1, 7, _generative_303[aciduino.getSelectedTrack()].number_of_tones);
      _generative_303[aciduino.getSelectedTrack()].number_of_tones = data;
    }
    
} tonesNumberComponent;