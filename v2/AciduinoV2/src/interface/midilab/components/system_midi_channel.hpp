#include "../../uCtrl.h"

struct MidiChannelConfig : PageComponent {

    void view() {
      genericOptionView("channel", String(_track_output_setup[_selected_track].channel+1), line, col, selected);
    }
    
    void change(int16_t data) {
      data = parseData(data, 0, 15, _track_output_setup[_selected_track].channel);
      ATOMIC(_track_output_setup[_selected_track].channel = data)
    }

} midiChannelConfigComponent;