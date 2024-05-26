struct AccentAmount : PageComponent {
  
    void view() {
      genericOptionView("accent", String(aciduino.getGenerativeParam(GENERATIVE_ACCENT)), line, col, selected);
    }

    void change(int16_t data) {
      data = parseData(data, 0, 100, aciduino.getGenerativeParam(GENERATIVE_ACCENT));
      aciduino.setGenerativeParam(GENERATIVE_ACCENT, data);
    }

} accentAmountComponent;