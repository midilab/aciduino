struct RangeOctave : PageComponent {
  
    void view() {
      genericOptionView("octaves", String(_generative_303[aciduino.getSelectedTrack()].range_octave), line, col, selected);
    }

    void change(int16_t data) {
      data = parseData(data, 1, 11, _generative_303[aciduino.getSelectedTrack()].range_octave);
      _generative_303[aciduino.getSelectedTrack()].range_octave = data;
    }
    
} rangeOctaveComponent;