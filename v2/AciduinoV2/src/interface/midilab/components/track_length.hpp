#include "../../../uCtrl/uCtrl.h"

struct TrackLength : PageComponent {

    void view() {
      genericOptionView("lenght", String(AcidSequencer.getTrackLength(_selected_track)), line, col, selected);
    }

    void change(int16_t data) {
      //clearStackNote(_selected_track);
      data = parseData(data, 1, AcidSequencer.getTrackMaxLength(_selected_track), AcidSequencer.getTrackLength(_selected_track));
      AcidSequencer.setTrackLength(_selected_track, data);
    }
    
} lengthComponent;