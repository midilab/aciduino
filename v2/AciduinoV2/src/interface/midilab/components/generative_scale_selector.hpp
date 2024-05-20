#include "../../../uCtrl/uCtrl.h"

struct TrackScale : PageComponent {

    TrackScale() {
      // we want this component to be 2 grids navigable object
      grid_size = 2;
    }
    
    void view() {
      genericOptionView("scale", String(AcidSequencer.getTemperamentName(AcidSequencer.getTemperamentId())), line, col, selected, true);
    }

    void change(int16_t data) {
      data = parseData(data, 0, 13, AcidSequencer.getTemperamentId());
      AcidSequencer.setTemperament(data);
    }

} scaleComponent;