struct TempoClockSource : PageComponent {

    void view() {
      genericOptionView("clock", aciduino.getMidiClockPort() == 0 ? String("int") : String(String("midi") + aciduino.getMidiClockPort()), line, col, selected);
    }

    void change(int16_t data) {
      data = parseData(data, 0, uCtrl.midi->sizeOf(), aciduino.getMidiClockPort());
      aciduino.setMidiClockPort(data);
      if (aciduino.getMidiClockPort() > 0) {
        uClock.setMode(uClock.EXTERNAL_CLOCK);
      } else {
        uClock.setMode(uClock.INTERNAL_CLOCK);
      }
    }
    
} tempoClockSourceComponent;