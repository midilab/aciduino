#include "../../uCtrl.h"

struct VoiceConfig : PageComponent {
  
    void view() {
      genericOptionView("note", String(AcidSequencer.getNoteString(AcidSequencer.getTrackVoiceConfig(_selected_track))), line, col, selected);
    }

    void change(int16_t data) {
      //clearStackNote(_selected_track);
      data = parseData(data, 0, 127, AcidSequencer.getTrackVoiceConfig(_selected_track));
      AcidSequencer.setTrackVoiceConfig(_selected_track, data);
      // send note for preview while change data
      sendNote(data, _track_output_setup[_selected_track].channel, _track_output_setup[_selected_track].port, NOTE_VELOCITY_808);
      sendNote(data, _track_output_setup[_selected_track].channel, _track_output_setup[_selected_track].port, 0);
    }
    
} voiceConfigComponent;
