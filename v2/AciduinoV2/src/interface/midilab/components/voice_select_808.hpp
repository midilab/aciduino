#include "../../../uCtrl/uCtrl.h"

struct VoiceSelect : PageComponent {
  
    void view() {
      genericOptionView("voice", String(AcidSequencer.getTrackVoiceName(_selected_track, AcidSequencer.getTrackVoice(_selected_track))), line, col, selected);
    }

    void change(int16_t data) {
      //clearStackNote(_selected_track);
      // create a getter for track voice size
      data = parseData(data, 0, VOICE_MAX_SIZE_808-1, AcidSequencer.getTrackVoice(_selected_track));
      AcidSequencer.setTrackVoice(_selected_track, data);
    }
    
} voiceSelectComponent;