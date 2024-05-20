#include "../../../uCtrl/uCtrl.h"

struct TrackTune : PageComponent {
  
    void view() {
      genericOptionView("tune", AcidSequencer.getTune(_selected_track) == 0 ? String("off") : String(AcidSequencer.getNoteString(AcidSequencer.getTune(_selected_track)-1)), line, col, selected);
    }

    void change(int16_t data) {
      // 0 = harmonizer off
      // 1 = C..., 12 = B
      //clearStackNote(_selected_track);
      data = parseData(data, 0, 12, AcidSequencer.getTune(_selected_track));
      AcidSequencer.setTune(_selected_track, data);
    }
    
} tuneComponent;