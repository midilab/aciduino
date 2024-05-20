#include "../../../uCtrl/uCtrl.h"

struct SequenceShift : PageComponent {
  
    void view() {
      genericOptionView("shift", String(AcidSequencer.getShiftPos(_selected_track)), line, col, selected);
    }

    void change(int16_t data) {
      //clearStackNote(_selected_track);
      data = parseData(data, AcidSequencer.getTrackLength(_selected_track)*-1, AcidSequencer.getTrackLength(_selected_track), AcidSequencer.getShiftPos(_selected_track));
      AcidSequencer.setShiftPos(_selected_track, data);
    }
    
} shiftComponent;