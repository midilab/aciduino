#include "../../uCtrl.h"

struct SessionConfig : PageComponent {
  
    SessionConfig() {
      grid_size = 2;    
    }

    void view() {
      genericOptionView("session", String("epprom"), line, col, selected, true);
      setF1("load");
      setF2("save");
    }
    
    void change(int16_t data) {
      //data = parseData(data, 0, 15, _track_output_setup[_selected_track].channel);
      //_track_output_setup[_selected_track].channel = data;
    }

    void function1() {
      loadSession();
    }

    void function2() {
      saveSession();
    }

} sessionConfigComponent;