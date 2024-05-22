struct TieAmount : PageComponent {
  
    void view() {
      genericOptionView("tie", String(_generative_303[aciduino.getSelectedTrack()].tie_probability), line, col, selected);
    }

    void change(int16_t data) {
      data = parseData(data, 0, 100, _generative_303[aciduino.getSelectedTrack()].tie_probability);
      _generative_303[aciduino.getSelectedTrack()].tie_probability = data;
    }
    
} tieAmountComponent;