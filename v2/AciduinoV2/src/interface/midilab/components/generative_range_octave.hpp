#include "../../../uCtrl/uCtrl.h"

struct RangeOctave : PageComponent {
  
    void view() {
      genericOptionView("octaves", String(_generative_303[_selected_track].range_octave), line, col, selected);
    }

    void change(int16_t data) {
      data = parseData(data, 1, 11, _generative_303[_selected_track].range_octave);
      _generative_303[_selected_track].range_octave = data;
    }
    
} rangeOctaveComponent;