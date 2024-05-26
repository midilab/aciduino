struct TieAmount : PageComponent {
  
    void view() {
      genericOptionView("tie", String(aciduino.getGenerativeParam(GENERATIVE_TIE)), line, col, selected);
    }

    void change(int16_t data) {
      data = parseData(data, 0, 100, aciduino.getGenerativeParam(GENERATIVE_TIE));
      aciduino.setGenerativeParam(GENERATIVE_TIE, data);
    }
    
} tieAmountComponent;