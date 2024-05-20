#include "../../uCtrl.h"

struct TempoTransport : PageComponent {

    void view() {
      genericOptionView("tempo", String(uClock.getTempo(), 1), line, col, selected);

      setF1("rec", AcidSequencer.getRecStatus());
      setF2("play", _playing ? true : false);
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
      AcidSequencer.setRecStatus(!AcidSequencer.getRecStatus());
    }

    void function2() {
      // play/stop
      if (_playing)
        uClock.stop();
      else
        uClock.start();
    }

} tempoTransportComponent;
