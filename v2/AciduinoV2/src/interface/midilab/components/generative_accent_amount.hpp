#include "../../../uCtrl/uCtrl.h"

struct AccentAmount : PageComponent {
  
    void view() {
      uint8_t accent_probability = AcidSequencer.is303(_selected_track) ? _generative_303[_selected_track].accent_probability : _generative_808[_selected_track-TRACK_NUMBER_303].accent_probability; 
      genericOptionView("accent", String(accent_probability), line, col, selected);
    }

    void change(int16_t data) {
      uint8_t * accent_probability = AcidSequencer.is303(_selected_track) ? &_generative_303[_selected_track].accent_probability : &_generative_808[_selected_track-TRACK_NUMBER_303].accent_probability; 
      data = parseData(data, 0, 100, *accent_probability);
      *accent_probability = data;
    }

} accentAmountComponent;