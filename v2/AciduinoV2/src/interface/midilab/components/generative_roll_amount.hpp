struct RollAmount : PageComponent {
  
    void view() {
      genericOptionView("roll", String(_generative_808[aciduino.getSelectedTrack()-TRACK_NUMBER_303].roll_probability), line, col, selected);
    }

    void change(int16_t data) {
      data = parseData(data, 0, 100, _generative_808[aciduino.getSelectedTrack()-TRACK_NUMBER_303].roll_probability);
      _generative_808[aciduino.getSelectedTrack()-TRACK_NUMBER_303].roll_probability = data;
    }
    
} rollAmountComponent;