#include "../../../uCtrl/uCtrl.h"

struct TonesNumber : PageComponent {

    void view() {
      genericOptionView("tones", String(_generative_303[_selected_track].number_of_tones), line, col, selected);
    }

    void change(int16_t data) {
      data = parseData(data, 1, 7, _generative_303[_selected_track].number_of_tones);
      _generative_303[_selected_track].number_of_tones = data;
    }
    
} tonesNumberComponent;