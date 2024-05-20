#include "../../uCtrl.h"

struct RecInputSource : PageComponent {
    
    void view() {
      genericOptionView("rec", String(String("midi") + _midi_rec_port), line, col, selected);
    }

    void change(int16_t data) {
      data = parseData(data, 1, uCtrl.midi->sizeOf(), _midi_rec_port);
      ATOMIC(_midi_rec_port = data)
    }

} recInputComponent;