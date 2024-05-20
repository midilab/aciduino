#include "../../../uCtrl/uCtrl.h"

struct SlideAmount : PageComponent {
  
    void view() {
      genericOptionView("slide", String(_generative_303[_selected_track].slide_probability), line, col, selected);
    }

    void change(int16_t data) {
      data = parseData(data, 0, 100, _generative_303[_selected_track].slide_probability);
      _generative_303[_selected_track].slide_probability = data;
    }
    
} slideAmountComponent;