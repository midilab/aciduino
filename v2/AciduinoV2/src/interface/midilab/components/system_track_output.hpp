#include "../../uCtrl.h"

struct TrackOutputSelector : PageComponent {

    void view() {
      genericOptionView("out", String(String("midi") + (_track_output_setup[_selected_track].port+1)), line, col, selected);
    }

    void change(int16_t data) {
      data = parseData(data, 0, uCtrl.midi->sizeOf()-1, _track_output_setup[_selected_track].port);
      ATOMIC(_track_output_setup[_selected_track].port = data)
    }

} trackOutputSelectorComponent;