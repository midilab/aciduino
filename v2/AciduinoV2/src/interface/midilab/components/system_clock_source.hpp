#include "../../uCtrl.h"

struct TempoClockSource : PageComponent {

    void view() {
      genericOptionView("clock", _midi_clock_port == 0 ? String("int") : String(String("midi") + _midi_clock_port), line, col, selected);
    }

    void change(int16_t data) {
      data = parseData(data, 0, uCtrl.midi->sizeOf(), _midi_clock_port);
      ATOMIC(_midi_clock_port = data)
      if (_midi_clock_port > 0) {
        uClock.setMode(uClock.EXTERNAL_CLOCK);
      } else {
        uClock.setMode(uClock.INTERNAL_CLOCK);
      }
    }
    
} tempoClockSourceComponent;