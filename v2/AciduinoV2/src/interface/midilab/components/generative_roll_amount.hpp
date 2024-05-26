struct RollAmount : PageComponent {
  
    void view() {
      genericOptionView("roll", String(aciduino.getGenerativeParam(GENERATIVE_ROLL)), line, col, selected);
    }

    void change(int16_t data) {
      data = parseData(data, 0, 100, aciduino.getGenerativeParam(GENERATIVE_ROLL));
      aciduino.setGenerativeParam(GENERATIVE_ROLL, data);
    }
    
} rollAmountComponent;