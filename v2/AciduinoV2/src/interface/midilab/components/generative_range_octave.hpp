struct RangeOctave : PageComponent {

  void view() {
    genericOptionView("octaves", String(aciduino.getGenerativeParam(GENERATIVE_RANGE_OCTAVE)), line, col, selected);
  }

  void change(int16_t data) {
    data = parseData(data, 1, 11, aciduino.getGenerativeParam(GENERATIVE_RANGE_OCTAVE));
    aciduino.setGenerativeParam(GENERATIVE_RANGE_OCTAVE, data);
  }
    
} rangeOctaveComponent;