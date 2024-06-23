struct TempoTransport : PageComponent {

    void view() {
      genericOptionView("tempo", String(uClock.getTempo(), 1), line, col, selected);

      setF1("rec", aciduino.seq.getRecStatus());
      setF2("play", aciduino.isPlaying() ? true : false);
    }

    void change(int16_t data) {
      // primary incrementer or decrementer
      if (data == DECREMENT || data == INCREMENT) {
        // inc and dec will fine update to 0.1 bpm
        uClock.setTempo(uClock.getTempo()+((data == DECREMENT ? -1 : 1) * 0.1));
      // secondary inc/dec or pot nav action
      } else {
        data = parseData(data, 30, 180, (uint16_t)uClock.getTempo());
        uClock.setTempo(data);
      }
    }

    void function1() {
      // rec mode
      aciduino.seq.setRecStatus(!aciduino.seq.getRecStatus());
    }

    void function2() {
      // play/stop
      if (aciduino.isPlaying())
        uClock.stop();
      else
        uClock.start();
    }

} tempoTransportComponent;
