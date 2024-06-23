struct SlideAmount : PageComponent {
  
    void view() {
      genericOptionView("slide", String(aciduino.getGenerativeParam(GENERATIVE_SLIDE)), line, col, selected);
    }

    void change(int16_t data) {
      data = parseData(data, 0, 100, aciduino.getGenerativeParam(GENERATIVE_SLIDE));
      aciduino.setGenerativeParam(GENERATIVE_SLIDE, data);
    }
    
} slideAmountComponent;