struct LowOctave : PageComponent {
  
    void view() {
      genericOptionView("octave", String(aciduino.getGenerativeParam(GENERATIVE_LOWER_OCTAVE)), line, col, selected);
    }

    void change(int16_t data) {
      data = parseData(data, 1, 11, aciduino.getGenerativeParam(GENERATIVE_LOWER_OCTAVE));
      aciduino.setGenerativeParam(GENERATIVE_LOWER_OCTAVE, data);
    }
    
} lowOctaveComponent;