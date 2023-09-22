//
// uCtrl Shared PageComponent 
// all Shared GUI components are programmed as PageComponent to be reused on different pages
//

// for a 1x grid 1x line size
// large == true ? 2x grid size
// used by a lot of simple components
void genericOptionView(const char * title_char, String value, uint8_t line, uint8_t col, bool selected, bool large = false)
{
  String title(title_char);
  const uint8_t view_size = 12;
  uint8_t x=(col-1)*5, y=(line-1)*8;
  uint8_t value_col = large ? (view_size*2)-value.length()+1 : view_size-value.length();
  uCtrl.oled->display->drawBox(x+1, y, 1, 7);
  uCtrl.oled->print(title, line, col+1, selected);
  uCtrl.oled->print(value, line, col+value_col, selected);
}

struct TopBar : PageComponent {

    // http://dotmatrixtool.com/#
    // 8px by 8px, row major, little endian
    const uint8_t SUBPAGE[8] = {0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00};
    const uint8_t SUBPAGE_SELECTED[8] = {0x00, 0x00, 0x3c, 0x3c, 0x3c, 0x3c, 0x00, 0x00};
    const uint8_t PLAYING[8] = {0x00, 0x06, 0x1e, 0x7e, 0x7e, 0x1e, 0x06, 0x00};
    
    TopBar() {
      // no directly navigation thru here
      no_nav = true;
    }

    void view() {
      // short page name at left most with box over it
      // |T1| 808 |seqr|o.   |i| 120.0
      uint8_t subpage_size = uCtrl.page->getSubPageSize();
      uint8_t selected_subpage = uCtrl.page->getSubPage();

      // track number and type
      //uCtrl.oled->display->drawUTF8(2, 0, atoi(_selected_track+1)); 
      uCtrl.oled->print("T", 1, 1); 
      uCtrl.oled->print(String(_selected_track+1), 1, 2); 
      uCtrl.oled->display->drawBox(0, 0, 10, 8);
      uCtrl.oled->print(AcidSequencer.is303(_selected_track) ? "303" : "808", 1, 4); 
/*
      uCtrl.oled->display->drawBox(0, 0, 15, 8);
      //uCtrl.oled->drawBox();
      if (_selected_track < 9) {
        uCtrl.oled->print("T0", 1, 1); 
        uCtrl.oled->print(String(_selected_track+1), 1, 3); 
      } else {
        uCtrl.oled->print("T", 1, 1); 
        uCtrl.oled->print(String(_selected_track+1), 1, 2); 
      }
 */
/*
      // current pattern
      uint8_t placement = 8;
      uCtrl.oled->print("P", 1, placement);
      uCtrl.oled->display->drawBox(((placement-1)*5)-2, 0, 8, 8);
      uCtrl.oled->print("1", 1, placement+2);

      // page/subpage
      placement = 12;
      uCtrl.oled->print(uCtrl.page->getPageName(), 1, placement);
      uCtrl.oled->display->drawBox(((placement-1)*5)-2, 0, 23, 8);
      uCtrl.oled->print(String(selected_subpage+1), 1, placement+5);
*/
       
      // page/subpage 
      uCtrl.oled->print(uCtrl.page->getPageName(), 1, 9);
      uCtrl.oled->display->drawBox(39, 0, 22, 8);
      for (uint8_t i=0; i < subpage_size; i++) {
        if (i == selected_subpage) {
          uCtrl.oled->print(SUBPAGE_SELECTED, 1, i+9);
        } else {
          uCtrl.oled->print(SUBPAGE, 1, i+9);
        }
      }
      
      // bpm display and setup
      //uCtrl.oled->print(String(uClock.getTempo(), 1), 1, 21, selected);
      uCtrl.oled->display->drawBox(88, 0, 8, 8);
      uCtrl.oled->print(String(uClock.getTempo(), 1), 1, 21);
      if (_playing) {
        uCtrl.oled->print(PLAYING, 1, 19);
      } else {
        uCtrl.oled->print(uClock.getMode() == uClock.INTERNAL_CLOCK ? "i" : "e", 1, 19);  
      }

      // top bar big line 
      uCtrl.oled->display->drawBox(0, 9, 128, 1);
    }

} topBarComponent;

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

struct RollType : PageComponent {
  
    String options;
    void view() {
      uint8_t roll_type = AcidSequencer.getRollType(_selected_track);
      if (roll_type <= FLAM_5) {
        options = "f"  + String(roll_type+1);
      } else {
        options = "s"  + String(roll_type - FLAM_5);
      }
      genericOptionView("roll", options, line, col, selected);
      // flam1, flam2, flam3, flam4, flam5, flam6. sub6, sub3, sub2?
    }

    void change(int16_t data) {
      //clearStackNote(_selected_track);
      data = parseData(data, 0, 6, AcidSequencer.getRollType(_selected_track));
      AcidSequencer.setRollType(_selected_track, data);
    }
    
} rollTypeComponent;

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

// used by page module for themed function button display
void functionDrawCallback(const char * f1, const char * f2, uint8_t f1_state, uint8_t f2_state)
{
  // menu action
  uCtrl.oled->print(f1, 8, 1+((14-strlen(f1))/2)); 
  uCtrl.oled->print(f2, 8, 14+((14-strlen(f2))/2)); 
  // state variation
  if (f1_state == 1) {
    uCtrl.oled->display->drawBox(0, 57, 63, 7);
  }
  if (f2_state == 1) {
    uCtrl.oled->display->drawBox(64, 57, 64, 7);
  }
  // horizontal line
  uCtrl.oled->display->drawBox(0, 56, 128, 1);
  // vertical separator
  //uCtrl.oled->display->drawBox(64, 59, 1, 5);
}