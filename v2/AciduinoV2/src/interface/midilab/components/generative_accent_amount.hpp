struct AccentAmount : PageComponent {
  
    void view() {
      uint8_t accent_probability = aciduino.seq.is303(aciduino.getSelectedTrack()) ? _generative_303[aciduino.getSelectedTrack()].accent_probability : _generative_808[aciduino.getSelectedTrack()-TRACK_NUMBER_303].accent_probability; 
      genericOptionView("accent", String(accent_probability), line, col, selected);
    }

    void change(int16_t data) {
      uint8_t * accent_probability = aciduino.seq.is303(aciduino.getSelectedTrack()) ? &_generative_303[aciduino.getSelectedTrack()].accent_probability : &_generative_808[aciduino.getSelectedTrack()-TRACK_NUMBER_303].accent_probability; 
      data = parseData(data, 0, 100, *accent_probability);
      *accent_probability = data;
    }

} accentAmountComponent;