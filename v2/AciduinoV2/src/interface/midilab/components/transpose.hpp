#include "../../../uCtrl/uCtrl.h"

struct Transpose : PageComponent {
  
    void view() {
      genericOptionView("transpose", String(AcidSequencer.getTranspose(_selected_track)), line, col, selected);
    }

    void change(int16_t data) {
      //clearStackNote(_selected_track);
      data = parseData(data, -12, 12, AcidSequencer.getTranspose(_selected_track));
      AcidSequencer.setTranspose(_selected_track, data);
    }
    
} transposeComponent;