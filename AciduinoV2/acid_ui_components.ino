
// all UI components are programmed as PageComponent to be reused on different pages
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

// for a 1x grid 1x line size
// large == true ? 2x grid size
void genericOptionView(String title, String value, uint8_t line, uint8_t col, bool selected, bool large = false)
{
  const uint8_t view_size = 12;
  uint8_t x=(col-1)*5, y=(line-1)*8;
  uint8_t value_col = large ? (view_size*2)-value.length()+1 : view_size-value.length();
  uCtrl.oled->display->drawBox(x+1, y, 1, 7);
  uCtrl.oled->print(title, line, col+1, selected);
  uCtrl.oled->print(value, line, col+value_col, selected);
}

// generic midi cc control object for 2 channels data
// 2x 303
// 2x 808
// always guard it behind a if (AcidSequencer.is303(_selected_track)) {
// to avoid array index access crash
struct MidiCCControl : PageComponent {

    // generic controler for 303 and 808 controlers
    String control_name;
    uint8_t control_cc = 0;
    //uint8_t * control_data;
    uint8_t control_data[2];

    MidiCCControl(String name, uint8_t cc, uint8_t data_slot = 1, uint8_t initial_value = 0)
    {
      control_name = name;
      control_cc = cc;
      //control_data = (uint8_t*) malloc(sizeof(uint8_t) * data_slot);
      for (uint8_t i; i < data_slot; i++) {
        control_data[i] = initial_value;
      }
    }
    
    void view() {
      uint8_t data_idx = AcidSequencer.is303(_selected_track) ? _selected_track : _selected_track - TRACK_NUMBER_303;
      genericOptionView(control_name, control_data[data_idx], line, col, selected);
    }

    void change(int8_t data) {
      // incrementer 1, decrementer -1
      uint8_t data_idx = AcidSequencer.is303(_selected_track) ? _selected_track : _selected_track - TRACK_NUMBER_303;
      data = parseData(data, 0, 127, control_data[data_idx]);
      control_data[data_idx] = data;
      // send data
      //sendMidiCC(control_cc, data, AcidSequencer.getTrackChannel(_selected_track));
    }
    
    void pot(uint16_t data) {
      uint8_t data_idx = AcidSequencer.is303(_selected_track) ? _selected_track : _selected_track - TRACK_NUMBER_303;
      data = parseData(data, 0, 127, control_data[data_idx]);
      control_data[data_idx] = data;
      // send data
      //sendMidiCC(control_cc, data, AcidSequencer.getTrackChannel(_selected_track));
    }
};

/*
// generic global scope data control
struct GeneralDataControl : PageComponent {

    String control_name;
    uint8_t * control_data;
    uint8_t control_min = 0;
    uint8_t control_max = 0;

    GeneralDataControl(String name, uint8_t * control_data_ptr, uint8_t min, uint8_t max)
    {
      control_name = name;
      control_data = control_data_ptr;
      control_cc = cc;
    }
    
    void view() {
      genericOptionView(control_name, control_data, line, col, selected);
    }

    void change(int8_t data) {
      // incrementer 1, decrementer -1
      data = parseData(data, 0, 127, *control_data);
      *control_data = data;
    }
    
    void pot(uint16_t data) {
      data = parseData(data, 0, 127, *control_data);
      *control_data = data;
    }
};
*/

struct TopBar : PageComponent {

    bool track_selected = true;
    bool tempo_selected = false;
    
    TopBar() {
      grid_size = 2;
      // not overried by hook callback of a page
      no_hook = true;
    }

    void view() {
      //set selected element
      track_selected = selected && selected_grid == 1;
      tempo_selected = selected && selected_grid == 2;

      // new layout
      // short page name at left most with box over it
      // |sequencer| track 1     |M| 120.0
      
      // track number and name
      //uCtrl.oled->display->drawUTF8(2, 0, atoi(_selected_track+1)); 
      uCtrl.oled->print("T", 1, 1); 
      uCtrl.oled->print(_selected_track+1, 1, 2); 
      uCtrl.oled->display->drawBox(0, 0, 10, 8);
      uCtrl.oled->print(AcidSequencer.is303(_selected_track) ? "303" : "808", 1, 4, track_selected); 
      uCtrl.oled->display->drawBox(0, 9, 128, 1);

      //uCtrl.oled->print(uCtrl.page->getPageName(), 2, 1);

      // bpm display and setup
      uCtrl.oled->print(uClock.getMode() == uClock.INTERNAL_CLOCK ? "i" : "e", 1, 19);    
      uCtrl.oled->display->drawBox(88, 0, 8, 8);
      uCtrl.oled->print(String(uClock.getTempo(), 1), 1, 21, tempo_selected);

      // f1 and f2
      if (track_selected) {
        setF1("save");
        setF2("paste");
      }
      if (tempo_selected) {
        setF1(uClock.getMode() == uClock.INTERNAL_CLOCK ? "external" : "internal");
        setF2(_playing ? "stop" : "play");
      }
    }
    
    void change(int8_t data) {
      // incrementer, decrementer
      if (track_selected) {
        // change track
        if (_selected_track == 0 && data < 0) {
          _selected_track = AcidSequencer.getTrackNumber() - 1;
        } else {
          _selected_track = (_selected_track + data) % AcidSequencer.getTrackNumber();
        }
      } else if (tempo_selected) {
        // inc and dec will fine update to 0.1 bpm
        uClock.setTempo(uClock.getTempo()+(data * 0.1));
      }
    }

    void pot(uint16_t data) {
      // incrementer, decrementer
      if (track_selected) {
        // change track
      } else if (tempo_selected) {
        // pot will increment with 1bpm fine update 
        // parseData(value, min, max, curr_value);
        data = parseData(data, 40, 160, (uint16_t)uClock.getTempo());
        uClock.setTempo(data);
      }
    }
    
    void function1() {
        uClock.setMode(uClock.getMode() == uClock.INTERNAL_CLOCK ? uClock.EXTERNAL_CLOCK : uClock.INTERNAL_CLOCK);
    }

    void function2() {
        if (_playing)
          uClock.stop();
        else
          uClock.start();
    }
    /*
    void nav(uint8_t dir) {
      // left, rigth
      switch (dir) {
        case LEFT:
        case RIGHT:
          if (selected_grid == 1) {
            track_selected = true;
            tempo_selected = false;
          } else if (selected_grid == 2) {
            tempo_selected = true;
            track_selected = false;
          }
          break;
      }
    }
    */
} topBarComponent;

struct StepSequencer : PageComponent {

    // step sequencer theme
    // http://dotmatrixtool.com/#
    // 8px by 8px, row major, little endian
    const uint8_t STEP_ON[8] = {0x00, 0x00, 0x3c, 0x3c, 0x3c, 0x3c, 0x00, 0x00};
    const uint8_t STEP_OFF[8] = {0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00};
    const uint8_t STEP_SELECTED[8] = {0xff, 0x81, 0x81, 0x81, 0x81, 0x81, 0x81, 0xff};
    const uint8_t STEP_TIME_SIGNATURE_MARK[8] = {0x03, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x03};
    uint8_t step_asset[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    uint8_t selected_locator = 0;
    uint8_t locator_length = 0;
    uint8_t locator_current = 0;
    uint8_t selected_step = 0;
    uint8_t curr_step = 0;
    uint8_t step_size = 0;
    
    StepSequencer() {
      // we want this component to be 2 lines and 2 grids navigable object
      line_size = 2;
      grid_size = 2;
    }
    
    void view() {
      uint8_t steps_line = line + 1;
      uint8_t idx = 0, step_on = 0;

      curr_step = AcidSequencer.getCurrentStep(_selected_track);
      step_size = AcidSequencer.getTrackLength(_selected_track);
      locator_length = ceil((float)step_size/16);
      locator_current = curr_step == 0 ? 0 : ceil((float)curr_step/16) - 1;
      
      // step locators
      uint8_t bar_size = locator_length > 1 ? 128 / locator_length : 0;
      for (uint8_t i=0; i < locator_length; i++) {
        uCtrl.oled->drawBox(y+(locator_current == i ? 2 : 3), x+(bar_size*i)+2, (locator_current == i ? 3 : 1), bar_size-4, (selected_locator == i && (selected_line == 1 || selected_line == 2) && selected));
      }

      // steps
      uint8_t first_step = (selected_line == 1 || selected_line == 2) && selected ? selected_locator*16 : locator_current*16;
      uint8_t last_step = first_step + 16 > step_size ? step_size : first_step + 16;
      for (uint8_t i=first_step; i < last_step; i++) {
        step_on = AcidSequencer.stepOn(_selected_track, i);
        idx = i % 16;

        // only use this in full sized mode
        //if (idx == 0 && i != 0) {
        //  steps_line++;
        //}
    
        // reset step asset memory 8 bytes array
        memset(step_asset, 0x00, sizeof(step_asset));
    
        // step on/off?
        if (step_on) {
          uCtrl.oled->mergeBitmap(step_asset, (uint8_t*)STEP_ON);
        } else {
          uCtrl.oled->mergeBitmap(step_asset, (uint8_t*)STEP_OFF);
        }
    
        // step time signature mark?
        if (i % 4 == 0) {
          uCtrl.oled->mergeBitmap(step_asset, (uint8_t*)STEP_TIME_SIGNATURE_MARK);
        }
    
        // step current?
        if (curr_step == i && _playing) {
          uCtrl.oled->mergeBitmap(step_asset, (uint8_t*)STEP_SELECTED);
        }
    
        // step selected?
        if (selected_step == i && selected && selected_line == 2) {
           uCtrl.oled->mergeBitmap(step_asset, (uint8_t*)STEP_SELECTED, true);
        }
    
        uCtrl.oled->print(step_asset, steps_line, idx+1);
      }

      // step info
      // 303
      if (AcidSequencer.is303(_selected_track)) {
        if (selected_line == 2) {
          uCtrl.oled->print(AcidSequencer.getNoteString(AcidSequencer.getStepData(_selected_track, selected_step)), line+2, 1);
        } else{
          if (AcidSequencer.stepOn(_selected_track, curr_step) && _playing) {
            uCtrl.oled->print(AcidSequencer.getNoteString(AcidSequencer.getStepData(_selected_track, curr_step)), line+2, 1);
          }
        }
      // 808
      } else {
        uCtrl.oled->print(AcidSequencer.getTrackVoiceName(_selected_track, AcidSequencer.getTrackVoice(_selected_track)), line+2, 1);
        //uCtrl.oled->print(AcidSequencer.getNoteString(AcidSequencer.getTrackVoiceConfig(_selected_track)), line+2, 1);
      }

      // f1 and f2
      // Selectors
      if (selected_line == 1) {
          setF1("copy");
          setF2("paste");
      // Steps
      } else if (selected_line == 2) {
        setF1("accent", AcidSequencer.accentOn(_selected_track, selected_step));
        // 303
        if (AcidSequencer.is303(_selected_track)) {
          setF2("slide", AcidSequencer.slideOn(_selected_track, selected_step));
        // 808
        } else {
          setF2("roll", AcidSequencer.rollOn(_selected_track, selected_step));
        }
      }
      
    }

    void nav(uint8_t dir) {
      // up, down, left, rigth
      switch (dir) {
        case UP:

          break;
        case DOWN:
          // step locator
          if (selected_line == 1) {
            //if (_playing) {
            //  selected_locator = locator_current;
            //}
          // steps
          } else if (selected_line == 2) {
            if (selected_step >= step_size) {
              selected_step = step_size-1;
            }
          }
          break;
        case LEFT:
          // step locator
          if (selected_line == 1) {
            if (selected_locator == 0) {
              selected_locator = locator_length-1;
              selected_step = (selected_step % 16) * locator_length;
              if (selected_step >= step_size) {
                selected_step = step_size-1;
              }
            } else {
              --selected_locator;
              selected_step -= 16;
            }
          // steps
          } else if(selected_line == 2) {
            if (selected_step == 0) {
              selected_step = step_size-1;
              selected_locator = locator_length-1;
            } else {
              --selected_step;
              selected_locator = selected_step / 16;
            }
          // ???
          } else if(selected_line == 3) {
            
          }

          break;
        case RIGHT:
          if (selected_line == 1) {
            if (selected_locator == locator_length-1) {
              selected_locator = 0;
              selected_step = selected_step % 16;
              if (selected_step >= step_size) {
                selected_step = step_size-1;
              }
            } else {
              ++selected_locator;
              selected_step += 16;
            }
          } else if(selected_line == 2) {
            if (selected_step == step_size-1) {
              selected_step = 0;
              selected_locator = 0;
            } else {
              ++selected_step;
              selected_locator = selected_step / 16;
            }
          } else if(selected_line == 3) {
            
          }

          break;
      }
      // keep grid nav aligned for user best ux experience
      selected_grid = (float)((float)(selected_locator+1)/(float)locator_length) > 0.5 ? 2 : 1;
      selected_grid = selected_step%16 >= 8 ? 2 : 1;
    }
    
    void change(int8_t data) {
      // incrementer, decrementer
      if(selected_line == 2) {
        AcidSequencer.rest(_selected_track, selected_step, data > 0 ? false : true);
      }
    }

    void pot(uint16_t data) {      
      // 303?
      if (AcidSequencer.is303(_selected_track)) {
        if(selected_line == 2) {
          // select step note
          // lets use 0-7 as off step, the rest is 0-127 midi range notes
          data = parseData(data, 0, 135, AcidSequencer.getStepData(_selected_track, selected_step));
          if (data < 8) {
            AcidSequencer.rest(_selected_track, selected_step, true);
          } else {
            AcidSequencer.setStepData(_selected_track, selected_step, data-8);
            AcidSequencer.rest(_selected_track, selected_step, false);
          }
        }
      // 808
      } else {
        // select voice
        data = parseData(data, 0, VOICE_MAX_SIZE_808-1, AcidSequencer.getTrackVoice(_selected_track));
        AcidSequencer.setTrackVoice(_selected_track, data);
        // select voice note[midi] or port[cv]
        //data = parseData(data, 0, 127, AcidSequencer.getTrackVoiceConfig(_selected_track));
        //AcidSequencer.setTrackVoiceConfig(_selected_track, data);
        // send note for preview while change data
        //sendNote(data, AcidSequencer.getTrackChannel(_selected_track));
      }
    }

    void function1() {
      if (selected_line == 2) {
        // 303 and 808 uses the same accent button f1
        AcidSequencer.setAccent(_selected_track, selected_step, !AcidSequencer.accentOn(_selected_track, selected_step));
      }
    }

    void function2() {
      if (selected_line == 2) {
        // 303
        if (AcidSequencer.is303(_selected_track)) {
          AcidSequencer.setSlide(_selected_track, selected_step, !AcidSequencer.slideOn(_selected_track, selected_step));
        // 808
        } else {
          AcidSequencer.setRoll(_selected_track, selected_step, !AcidSequencer.rollOn(_selected_track, selected_step));
        }
      }
    }
    
} stepSequencerComponent;

struct TrackLength : PageComponent {

    void view() {
      genericOptionView("lenght", AcidSequencer.getTrackLength(_selected_track), line, col, selected);
    }

    void change(int8_t data) {
      // incrementer 1, decrementer -1
      //clearStackNote(_selected_track);
      data = parseData(data, 1, AcidSequencer.getTrackMaxLength(_selected_track), AcidSequencer.getTrackLength(_selected_track));
      AcidSequencer.setTrackLength(_selected_track, data);
    }
    
    void pot(uint16_t data) {
      data = parseData(data, 1, AcidSequencer.getTrackMaxLength(_selected_track), AcidSequencer.getTrackLength(_selected_track));
      AcidSequencer.setTrackLength(_selected_track, data);
    }
    
    // F1
    // global: selected changes the whole track length
    // F2
    // voice: selected changes only the voice length
    // same for shift?
    
} lengthComponent;

struct SequenceShift : PageComponent {
    void view() {
      genericOptionView("shift", AcidSequencer.getShiftPos(_selected_track), line, col, selected);
    }

    void change(int8_t data) {
      // incrementer 1, decrementer -1
      //clearStackNote(_selected_track);
      data = parseData(data, AcidSequencer.getTrackLength(_selected_track)*-1, AcidSequencer.getTrackLength(_selected_track), AcidSequencer.getShiftPos(_selected_track));
      AcidSequencer.setShiftPos(_selected_track, data);
    }
    
    void pot(uint16_t data) {
      data = parseData(data, AcidSequencer.getTrackLength(_selected_track)*-1, AcidSequencer.getTrackLength(_selected_track), AcidSequencer.getShiftPos(_selected_track));
      AcidSequencer.setShiftPos(_selected_track, data);
    }
} shiftComponent;

struct RollType : PageComponent {
    String options = "";
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

    void change(int8_t data) {
      // incrementer 1, decrementer -1
      //clearStackNote(_selected_track);
      data = parseData(data, 0, 6, AcidSequencer.getRollType(_selected_track));
      AcidSequencer.setRollType(_selected_track, data);
    }
    
    void pot(uint16_t data) {
      data = parseData(data, 0, 6, AcidSequencer.getRollType(_selected_track));
      AcidSequencer.setRollType(_selected_track, data);
    }
} rollTypeComponent;

// make a seq divider too!

struct VoiceSelect : PageComponent {
    void view() {
      genericOptionView("voice", AcidSequencer.getTrackVoiceName(_selected_track, AcidSequencer.getTrackVoice(_selected_track)), line, col, selected);
    }

    void change(int8_t data) {
      // incrementer 1, decrementer -1
      //clearStackNote(_selected_track);
      // create a getter for track voice size
      data = parseData(data, 0, VOICE_MAX_SIZE_808-1, AcidSequencer.getTrackVoice(_selected_track));
      AcidSequencer.setTrackVoice(_selected_track, data);
    }
    
    void pot(uint16_t data) {
      data = parseData(data, 0, VOICE_MAX_SIZE_808-1, AcidSequencer.getTrackVoice(_selected_track));
      AcidSequencer.setTrackVoice(_selected_track, data);
    }
} voiceSelectComponent;

struct VoiceConfig : PageComponent {
    void view() {
      genericOptionView("note", AcidSequencer.getNoteString(AcidSequencer.getTrackVoiceConfig(_selected_track)), line, col, selected);
    }

    void change(int8_t data) {
      // incrementer 1, decrementer -1
      //clearStackNote(_selected_track);
      data = parseData(data, 0, 127, AcidSequencer.getTrackVoiceConfig(_selected_track));
      AcidSequencer.setTrackVoiceConfig(_selected_track, data);
      // send note for preview while change data
      sendNote(data, AcidSequencer.getTrackChannel(_selected_track));
    }

    void pot(uint16_t data) {
      data = parseData(data, 0, 127, AcidSequencer.getTrackVoiceConfig(_selected_track));
      AcidSequencer.setTrackVoiceConfig(_selected_track, data);
      // send note for preview while change data
      sendNote(data, AcidSequencer.getTrackChannel(_selected_track));
    }
} voiceConfigComponent;

struct TrackTune : PageComponent {
    void view() {
      genericOptionView("tune", AcidSequencer.getTune(_selected_track) == 0 ? "off" : AcidSequencer.getNoteString(AcidSequencer.getTune(_selected_track)-1), line, col, selected);
    }

    void change(int8_t data) {
      // incrementer 1, decrementer -1
      //clearStackNote(_selected_track);
      // off, C, C#, D, D#... B
      // 0 = harmonizer off
      // 1 = C..., 12 = B
      data = parseData(data, 0, 12, AcidSequencer.getTune(_selected_track));
      AcidSequencer.setTune(_selected_track, data);
    }
    
    void pot(uint16_t data) {
      data = parseData(data, 0, 12, AcidSequencer.getTune(_selected_track));
      AcidSequencer.setTune(_selected_track, data);
    }

} tuneComponent;

struct TonesNumber : PageComponent {

    TonesNumber() {
      grid_size = 2;
    }
    
    void view() {
      genericOptionView("tones", _number_of_tones, line, col, selected, true);
    }

    void change(int8_t data) {
      data = parseData(data, 1, 7, _number_of_tones);
      _number_of_tones = data;
    }
    
    void pot(uint16_t data) {
      data = parseData(data, 1, 7, _number_of_tones);
      _number_of_tones = data;
    }
} tonesNumberComponent;

struct LowRange : PageComponent {
    void view() {
      genericOptionView("low", AcidSequencer.getNoteString(_lower_note), line, col, selected);
    }

    void change(int8_t data) {
      data = parseData(data, 0, 127, _lower_note);
      _lower_note = data;
    }
    
    void pot(uint16_t data) {
      data = parseData(data, 0, 127, _lower_note);
      _lower_note = data;
    }
} lowRangeComponent;

struct HighRange : PageComponent {
    void view() {
      genericOptionView("high", _range_note, line, col, selected);
    }

    void change(int8_t data) {
      data = parseData(data, 0, 127, _range_note);
      _range_note = data;
    }
    
    void pot(uint16_t data) {
      data = parseData(data, 0, 127, _range_note);
      _range_note = data;
    }
} highRangeComponent;

struct AccentAmount : PageComponent {
    void view() {
      genericOptionView("accent", _accent_probability, line, col, selected);
    }

    void change(int8_t data) {
      data = parseData(data, 0, 100, _accent_probability);
      _accent_probability = data;
    }
    
    void pot(uint16_t data) {
      data = parseData(data, 0, 100, _accent_probability);
      _accent_probability = data;
    }
} accentAmountComponent;

struct SlideAmount : PageComponent {
    void view() {
      genericOptionView("slide", _slide_probability, line, col, selected);
    }

    void change(int8_t data) {
      data = parseData(data, 0, 100, _slide_probability);
      _slide_probability = data;
    }
    
    void pot(uint16_t data) {
      data = parseData(data, 0, 100, _slide_probability);
      _slide_probability = data;
    }
} slideAmountComponent;

struct RollAmount : PageComponent {
    void view() {
      genericOptionView("roll", _roll_probability, line, col, selected);
    }

    void change(int8_t data) {
      data = parseData(data, 0, 100, _roll_probability);
      _roll_probability = data;
    }
    
    void pot(uint16_t data) {
      data = parseData(data, 0, 100, _roll_probability);
      _roll_probability = data;
    }
} rollAmountComponent;

struct TrackScale : PageComponent {

    TrackScale() {
      // we want this component to be 2 grids navigable object
      grid_size = 2;
    }
    
    void view() {
      genericOptionView("scale", AcidSequencer.getTemperamentName(AcidSequencer.getTemperamentId()), line, col, selected, true);
    }

    void change(int8_t data) {
      // incrementer 1, decrementer -1
      //clearStackNote(_selected_track);
      // off, C, C#, D, D#... B
      // 0 = harmonizer off
      // 1 = C..., 12 = B
      data = parseData(data, 0, 13, AcidSequencer.getTemperamentId());
      AcidSequencer.setTemperament(data);
    }
    
    void pot(uint16_t data) {
      data = parseData(data, 0, 13, AcidSequencer.getTemperamentId());
      AcidSequencer.setTemperament(data);
    }

} scaleComponent;

struct TrackFill : PageComponent {

    TrackFill() {
      // we want this component to be 2 grids navigable object
      grid_size = 2;
    }
    
    void view() {
      genericOptionView("fill", _generative_fill, line, col, selected, true);
    }

    void change(int8_t data) {
      // incrementer 1, decrementer -1
      //clearStackNote(_selected_track);
      data = parseData(data, 1, 100, _generative_fill);
      _generative_fill = data;
    }
    
    void pot(uint16_t data) {
      data = parseData(data, 1, 100, _generative_fill);
      _generative_fill = data;
    }
} fillComponent;


/*
void note_view()
{
  uint8_t steps_line = 3;
  uint8_t curr_step = AcidSequencer.getCurrentStep(0);
  uint8_t step_size = AcidSequencer.getTrackLength(0);
  uint8_t idx = 0;

  uCtrl.oled->print("  16  ", steps_line, 1, false, false); 
  uCtrl.oled->display->drawBox(0, ((steps_line-1)*8)+1, 29, 6);  
  uCtrl.oled->print("  32  ", steps_line, 7, false, false); 
  uCtrl.oled->display->drawBox(30, ((steps_line-1)*8)+1, 29, 6);
  uCtrl.oled->print("  48  ", steps_line, 13, false, false); 
  uCtrl.oled->display->drawBox(60, ((steps_line-1)*8)+1, 29, 6);
  uCtrl.oled->print("  64  ", steps_line, 19, false, false); 
  uCtrl.oled->display->drawBox(90, ((steps_line-1)*8)+1, 30, 6);
  steps_line++;
  
  for (uint8_t i=0; i < 16; i++) {
    idx = i % 4;
    if (idx == 0 && i != 0) {
      steps_line++;
    }
    uCtrl.oled->print("  C3  ", steps_line, (idx*6)+1, false, false); 
    if (curr_step == i) {
      //uCtrl.oled->print("> C3  ", steps_line, (idx*6)+1, false, false); 
      //uCtrl.oled->display->drawLine(0, (idx*5*6), 29, 6);  
      uCtrl.oled->display->drawBox(idx*5*6, (steps_line-1)*8, 29, 8);  
    }
  } 

  //uCtrl.oled->print("  16  ", steps_line+1, 9, false, false); 
  //uCtrl.oled->display->drawBox(0, 49, 120, 6);

  //uCtrl.oled->print("  16  ", steps_line+1, 3, false, false); 
  //uCtrl.oled->display->drawBox(0, 49, 59, 6);  
  //uCtrl.oled->print("  32  ", steps_line+1, 16, false, false); 
  //uCtrl.oled->display->drawBox(61, 49, 59, 6);

  //uCtrl.oled->print("  16  ", steps_line+1, 2, false, false); 
  //uCtrl.oled->display->drawBox(0, 49, 39, 6);  
  //uCtrl.oled->print("  32  ", steps_line+1, 10, false, false); 
  //uCtrl.oled->display->drawBox(40, 49, 39, 6);
  //uCtrl.oled->print("  48  ", steps_line+1, 18, false, false); 
  //uCtrl.oled->display->drawBox(80, 49, 40, 6);
  
  
  //uCtrl.oled->print("    ", steps_line+1, 1, false, true); 
  //uCtrl.oled->print("    ", steps_line+1, 5, false, step_size > 16 ? true : false); 
  //uCtrl.oled->print("    ", steps_line+1, 9, false, step_size > 32 ? true : false); 
  //uCtrl.oled->print("    ", steps_line+1, 13, false, step_size > 48 ? true : false);   
}
*/
