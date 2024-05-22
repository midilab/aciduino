struct LowOctave : PageComponent {
  
    void view() {
      genericOptionView("octave", String(_generative_303[aciduino.getSelectedTrack()].lower_octave), line, col, selected);
    }

    void change(int16_t data) {
      data = parseData(data, 1, 11, _generative_303[aciduino.getSelectedTrack()].lower_octave);
      _generative_303[aciduino.getSelectedTrack()].lower_octave = data;
    }
    
} lowOctaveComponent;