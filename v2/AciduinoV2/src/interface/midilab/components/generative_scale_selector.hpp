struct TrackScale : PageComponent {

    TrackScale() {
      // we want this component to be 2 grids navigable object
      grid_size = 2;
    }
    
    void view() {
      genericOptionView("scale", String(aciduino.seq.getTemperamentName(aciduino.seq.getTemperamentId())), line, col, selected, true);
    }

    void change(int16_t data) {
      data = parseData(data, 0, 13, aciduino.seq.getTemperamentId());
      aciduino.seq.setTemperament(data);
    }

} scaleComponent;