struct RecInputSource : PageComponent {
    
    void view() {
      genericOptionView("rec", String(String("midi") + aciduino.getMidiRecPort()), line, col, selected);
    }

    void change(int16_t data) {
      data = parseData(data, 1, uCtrl.midi->sizeOf(), aciduino.getMidiRecPort());
      aciduino.setMidiRecPort(data);
    }

} recInputComponent;