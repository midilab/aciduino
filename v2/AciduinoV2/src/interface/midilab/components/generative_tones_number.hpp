struct TonesNumber : PageComponent {

    void view() {
      genericOptionView("tones", String(aciduino.getGenerativeParam(GENERATIVE_NUM_TONES)), line, col, selected);
    }

    void change(int16_t data) {
      data = parseData(data, 1, 7, aciduino.getGenerativeParam(GENERATIVE_NUM_TONES));
      aciduino.setGenerativeParam(GENERATIVE_NUM_TONES, data);
    }
    
} tonesNumberComponent;