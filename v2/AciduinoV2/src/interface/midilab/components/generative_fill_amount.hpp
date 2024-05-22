struct TrackFill : PageComponent {

    TrackFill() {
      // we want this component to be 2 grids navigable object
      grid_size = 2;
    }
    
    void view() {
      uint8_t generative_fill = aciduino.seq.is303(aciduino.getSelectedTrack()) ? _generative_303[aciduino.getSelectedTrack()].generative_fill : _generative_808[aciduino.getSelectedTrack()-TRACK_NUMBER_303].generative_fill; 
      genericOptionView("fill", String(generative_fill), line, col, selected, true);
    }

    void change(int16_t data) {
      //clearStackNote(aciduino.getSelectedTrack());
      uint8_t * generative_fill = aciduino.seq.is303(aciduino.getSelectedTrack()) ? &_generative_303[aciduino.getSelectedTrack()].generative_fill : &_generative_808[aciduino.getSelectedTrack()-TRACK_NUMBER_303].generative_fill; 
      data = parseData(data, 1, 100, *generative_fill);
      *generative_fill = data;
    }
    
} fillComponent;