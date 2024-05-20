#include "../../../uCtrl/uCtrl.h"

struct LowOctave : PageComponent {
  
    void view() {
      genericOptionView("octave", String(_generative_303[_selected_track].lower_octave), line, col, selected);
    }

    void change(int16_t data) {
      data = parseData(data, 1, 11, _generative_303[_selected_track].lower_octave);
      _generative_303[_selected_track].lower_octave = data;
    }
    
} lowOctaveComponent;