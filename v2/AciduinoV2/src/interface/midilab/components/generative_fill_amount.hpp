struct TrackFill : PageComponent {

    TrackFill() {
      // we want this component to be 2 grids navigable object
      grid_size = 2;
    }
    
    void view() {
      genericOptionView("fill", String(aciduino.getGenerativeParam(GENERATIVE_FILL)), line, col, selected, true);
    }

    void change(int16_t data) {
      data = parseData(data, 1, 100, aciduino.getGenerativeParam(GENERATIVE_FILL));
      aciduino.setGenerativeParam(GENERATIVE_FILL, data);
    }
    
} fillComponent;