#include "../../../uCtrl/uCtrl.h"

struct TieAmount : PageComponent {
  
    void view() {
      genericOptionView("tie", String(_generative_303[_selected_track].tie_probability), line, col, selected);
    }

    void change(int16_t data) {
      data = parseData(data, 0, 100, _generative_303[_selected_track].tie_probability);
      _generative_303[_selected_track].tie_probability = data;
    }
    
} tieAmountComponent;