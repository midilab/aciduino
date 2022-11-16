//
// uCtrl PageComponent 
// all UI components are programmed as PageComponent to be reused on different pages
//

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

// for a 1x grid 1x line size
// large == true ? 2x grid size
// used byu a lot of simple components
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
    const char * control_name;
    uint8_t control_cc = 0;
    uint8_t * control_data;

    MidiCCControl(const char * name, uint8_t cc, uint8_t data_slot = 1, uint8_t initial_value = 0)
    {
      control_name = name;
      control_cc = cc;
      control_data = (uint8_t*) malloc(sizeof(uint8_t) * data_slot);
      for (uint8_t i=0; i < data_slot; i++) {
        control_data[i] = initial_value;
      }
    }
    
    void view() {
      uint8_t data_idx = AcidSequencer.is303(_selected_track) ? _selected_track : _selected_track - TRACK_NUMBER_303;
      genericOptionView(control_name, control_data[data_idx], line, col, selected);
    }

    void change(int16_t data) {
      // incrementer 1, decrementer -1
      uint8_t data_idx = AcidSequencer.is303(_selected_track) ? _selected_track : _selected_track - TRACK_NUMBER_303;
      data = parseData(data, 0, 127, control_data[data_idx]);
      control_data[data_idx] = data;
      // send data
      sendMidiCC(control_cc, data, AcidSequencer.getTrackChannel(_selected_track));
    }
    
};

struct TopBar : PageComponent {

    // http://dotmatrixtool.com/#
    // 8px by 8px, row major, little endian
    const uint8_t SUBPAGE[8] = {0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00};
    const uint8_t SUBPAGE_SELECTED[8] = {0x00, 0x00, 0x3c, 0x3c, 0x3c, 0x3c, 0x00, 0x00};
    
    TopBar() {
      // not overried by hook callback of a page
      no_hook = true;
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
      uCtrl.oled->print(_selected_track+1, 1, 2); 
      uCtrl.oled->display->drawBox(0, 0, 10, 8);
      uCtrl.oled->print(AcidSequencer.is303(_selected_track) ? "303" : "808", 1, 4); 

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
      uCtrl.oled->print(uClock.getMode() == uClock.INTERNAL_CLOCK ? "i" : "e", 1, 19);    
      uCtrl.oled->display->drawBox(88, 0, 8, 8);
      uCtrl.oled->print(String(uClock.getTempo(), 1), 1, 21, selected);

      // top bar big line 
      uCtrl.oled->display->drawBox(0, 9, 128, 1);
      
      // f1 and f2
      setF1(uClock.getMode() == uClock.INTERNAL_CLOCK ? "external" : "internal");
      setF2(_playing ? "stop" : "play");
    }
    
    void change(int16_t data) {
      // primary incrementer or decrementer
      if (data == DECREMENT || data == INCREMENT) {
        // inc and dec will fine update to 0.1 bpm
        uClock.setTempo(uClock.getTempo()+((data == DECREMENT ? -1 : 1) * 0.1));
      // secondary inc/dec or pot nav action
      } else {
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
} topBarComponent;

struct StepSequencer : PageComponent {

    // step sequencer theme editor
    // http://dotmatrixtool.com/#
    // 8px by 8px, row major, little endian
    // made your self a nice theme? share it with aciduino community via pull request!
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
    uint8_t pot_last_note = 36;
    bool full_size_view = false;
    // rec and preview function helpers
    uint8_t rec_step_in = 0;
    uint8_t rec_track = 0;
    uint8_t rec_note = 36;

    StepSequencer() {
      // we want this component to be 2 lines for compact view(5 for full sized view) and 2 grids navigable object
      line_size = 2;
      grid_size = 2;
      // enables changeRelease() for release state of generic 1/2 buttons
      change_full_state = true;
      // enable more complex nav by updating selector pointer of page component
      update_selector = true;
    }

    void setViewMode(bool full_size) {
      if (full_size != full_size_view) {
        full_size_view = full_size;
        line_size = full_size_view ? ceil((float)step_size/16) + 1 : 2;
        selected_locator = selected_step / (full_size_view ? step_size : 16);
      }
    }
    
    void view() {
      uint8_t steps_line = line + 1;
      uint8_t idx = 0, step_on = 0;
      uint8_t max_steps_per_view = full_size_view ? 64 : 16;

      curr_step = AcidSequencer.getCurrentStep(_selected_track);
      step_size = AcidSequencer.getTrackLength(_selected_track);
      locator_current = curr_step == 0 ? 0 : ceil((float)curr_step/max_steps_per_view) - 1;
      locator_length = ceil((float)step_size/max_steps_per_view);
      // fix selected step changes on tracks with different track length
      selected_step = selected_step >= step_size ? step_size - 1 : selected_step;
      selected_locator = selected_locator >= locator_length ? locator_current : selected_locator;
      // force it again in case track length change
      line_size = full_size_view ? ceil((float)step_size/16) + 1 : 2;

#if defined(USE_UONE_BOARD)
      // locator leds
      uCtrl.dout->writeAll(LOW);
      if (locator_current !=  selected_locator || !selected) {
        uCtrl.dout->write(locator_current+1, HIGH); 
      }
      // selected locator and selected componet?
      if ((selected_line == 1 || selected_line == 2) && selected) {
        uCtrl.dout->write(selected_locator+1, uCtrl.dout->blink() ? LOW : HIGH);
      }
#endif
      
      // step locators
      uint8_t bar_size = locator_length > 1 ? 128 / locator_length : 0;
      for (uint8_t i=0; i < locator_length; i++) {
        uCtrl.oled->drawBox(y+(locator_current == i ? 2 : 3), x+(bar_size*i)+2, (locator_current == i ? 3 : 1), bar_size-4, (selected_locator == i && (selected_line == 1 || selected_line >= 2) && selected));
      }

      // steps
      uint8_t first_step = (selected_line == 1 || selected_line >= 2) && selected ? selected_locator * max_steps_per_view : locator_current*max_steps_per_view;
      uint8_t last_step = first_step + max_steps_per_view > step_size ? step_size : first_step + max_steps_per_view;
      for (uint8_t i=first_step; i < last_step; i++) {
        step_on = AcidSequencer.stepOn(_selected_track, i);
        idx = i % 16;

        // only use this in full sized mode
        if (idx == 0 && i != 0 && full_size_view) {
          steps_line++;
        }
    
        // reset step asset memory 8 bytes array
        memset(step_asset, 0x00, sizeof(step_asset));
    
        // step on/off?
        if (step_on) {
          uCtrl.oled->mergeBitmap(step_asset, (uint8_t*)STEP_ON);
#if defined(USE_UONE_BOARD)
          uCtrl.dout->write(idx+9, HIGH);
#endif
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
#if defined(USE_UONE_BOARD)
          uCtrl.dout->write(idx+9, step_on ? LOW : HIGH);
#endif
        }
    
        // step selected?
        if (selected_step == i && selected && selected_line >= 2) {
           uCtrl.oled->mergeBitmap(step_asset, (uint8_t*)STEP_SELECTED, true);
#if defined(USE_UONE_BOARD)
           uCtrl.dout->write(idx+9, uCtrl.dout->blink() ? LOW : HIGH);
#endif
        }
    
        uCtrl.oled->print(step_asset, steps_line, idx+1);
      }

      // step info
      uint8_t info_line_idx = full_size_view ? ceil((float)step_size/16) + 1 : 2;
      // 303
      if (AcidSequencer.is303(_selected_track)) {
        if (selected_line >= 2) {
          uCtrl.oled->print(AcidSequencer.getNoteString(AcidSequencer.getStepData(_selected_track, selected_step)), line+info_line_idx, 1);
        } else{
          if (AcidSequencer.stepOn(_selected_track, curr_step) && _playing) {
            uCtrl.oled->print(AcidSequencer.getNoteString(AcidSequencer.getStepData(_selected_track, curr_step)), line+info_line_idx, 1);
          }
        }
      // 808
      } else {
        uCtrl.oled->print(AcidSequencer.getTrackVoiceName(_selected_track, AcidSequencer.getTrackVoice(_selected_track)), line+info_line_idx, 1);
        //uCtrl.oled->print(AcidSequencer.getNoteString(AcidSequencer.getTrackVoiceConfig(_selected_track)), line+info_line_idx, 1);
      }

      // f1 and f2
      // Selectors
      if (selected_line == 1) {
          setF1("mute", AcidSequencer.getMute(_selected_track));
          // you can only paste if the selected selector is cleared otherwise it will shows copy
          setF2("clear");
      // Steps
      } else if (selected_line >= 2) {
        setF1("accent", AcidSequencer.accentOn(_selected_track, selected_step));
        // 303
        if (AcidSequencer.is303(_selected_track)) {
          if ((AcidSequencer.stepOn(_selected_track, selected_step-1) || AcidSequencer.tieOn(_selected_track, selected_step-1)) && !AcidSequencer.stepOn(_selected_track, selected_step) && selected_step != 0) {
            setF2("tie", AcidSequencer.tieOn(_selected_track, selected_step));
          } else {
            setF2("slide", AcidSequencer.slideOn(_selected_track, selected_step));
          }
        // 808
        } else {
          setF2("roll", AcidSequencer.rollOn(_selected_track, selected_step));
        }
      }

      // debug
      //uCtrl.oled->print(selected_line, line+info_line_idx, 10);
      
    }

    void nav(uint8_t dir) {

      switch (dir) {
        case UP:
          if (full_size_view && selected_line != 1) {
            selected_step = selected_step >= 16 ? selected_step - 16 : step_size - (16 - selected_step);
            selected_locator = selected_step / (full_size_view ? step_size : 16);
            selected_line = ceil((float)selected_step/16) + 1;
          }
          break;
        case DOWN:
          if (full_size_view && selected_line != 1) {
            selected_step = (selected_step + 16) % step_size;
            //selected_step = (selected_step + 16) > step_size ? step_size - 1 : (selected_step + 16) % step_size;
            selected_locator = selected_step / (full_size_view ? step_size : 16);
            selected_line = ceil((float)selected_step/16) + 1;
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
          } else if(selected_line > 1) {
            if (selected_step == 0) {
              selected_step = step_size-1;
              selected_locator = locator_length-1;
            } else {
              --selected_step;
              selected_locator = selected_step / (full_size_view ? step_size : 16);
            }
            // update the selected line if is full_size_view
            if (full_size_view && selected_line != 1) {
              selected_line = (selected_step / 16) + 2;
            }
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
          } else if(selected_line > 1) {
            if (selected_step == step_size-1) {
              selected_step = 0;
              selected_locator = 0;
            } else {
              ++selected_step;
              selected_locator = selected_step / (full_size_view ? step_size : 16);
            }
            // update the selected line if is full_size_view
            if (full_size_view && selected_line != 1) {
              selected_line = (selected_step / 16) + 2;
            }
          }
          break;
      }

      // keep grid nav aligned for user best ux experience
      selected_grid = (float)((float)(selected_locator+1)/(float)locator_length) > 0.5 ? 2 : 1;
      selected_grid = selected_step%16 >= 8 ? 2 : 1;
      
    }
    
    void change(int16_t data) {
      
      if (data == DECREMENT || data == INCREMENT) {

        // toggle on/off step with generic button 2(INCREMENT)
        if (data == INCREMENT && selected_line != 1) {
          AcidSequencer.rest(_selected_track, selected_step, AcidSequencer.stepOn(_selected_track, selected_step));
        // tap button for preview and realtime record with generic button 1(DECREMENT)
        } else {
          // if its playing we record
          // keep track to avoid changes on note while note on state
          rec_step_in = curr_step+1;
          rec_track = _selected_track;
          rec_note = AcidSequencer.is303(_selected_track) ? pot_last_note : AcidSequencer.getTrackVoiceConfig(_selected_track);
          if (_playing) {
            // for 303 we also set the note input via nav pot
            if (AcidSequencer.is303(rec_track)) {
              AcidSequencer.setStepData(rec_track, rec_step_in, rec_note);
            }
            // record note in!
            AcidSequencer.rest(rec_track, rec_step_in, false);
          }
          // send note preview
          sendNote(rec_note, AcidSequencer.getTrackChannel(rec_track), AcidSequencer.is303(rec_track) ? NOTE_VELOCITY_303 : NOTE_VELOCITY_808); 
        }

      // secondary inc/dec or pot nav action
      } else {
        
        // 303?
        if (AcidSequencer.is303(_selected_track)) {

          uint8_t octave, note;
        
          octave = floor(AcidSequencer.getStepData(_selected_track, selected_step) / 12);
          note = AcidSequencer.getStepData(_selected_track, selected_step) - (octave * 12);
  
          // shift sets octave
          if (uCtrl.page->isShiftPressed()) {
            data = parseData(data, 0, 10, octave);
            pot_last_note = note + (data*12);
          // no shift sets note
          } else {
            data = parseData(data, 0, 11, note);
            pot_last_note = data + (octave*12);
          }
          
          if(selected_line >= 2) {
            // select step note
            AcidSequencer.setStepData(_selected_track, selected_step, pot_last_note);
          }
        // 808
        } else {

          // shift sets voice config
          if (uCtrl.page->isShiftPressed()) {
            // select voice note[midi] or port[cv]
            data = parseData(data, 0, 127, AcidSequencer.getTrackVoiceConfig(_selected_track));
            AcidSequencer.setTrackVoiceConfig(_selected_track, data);
            // send note for preview while change data
            sendNote(data, AcidSequencer.getTrackChannel(_selected_track), NOTE_VELOCITY_808);
            sendNote(data, AcidSequencer.getTrackChannel(_selected_track), 0);
          // no shift select voice
          } else {
            // select voice
            data = parseData(data, 0, VOICE_MAX_SIZE_808-1, AcidSequencer.getTrackVoice(_selected_track));
            AcidSequencer.setTrackVoice(_selected_track, data);
          }

        }
        
      }
      
    }

    void changeRelease(int16_t data) {
      if(selected_line >= 2) {
        // preview note release for note off
        if (data == DECREMENT) {
          // if its playing we record
          if (_playing) {
            // for 303 we also set the note input via nav pot
            if (AcidSequencer.is303(rec_track)) {
              // should we set tie?
              uint8_t start_step = rec_step_in+1;
              uint8_t steps_holded = 0;
              // hold longer than track end step
              if (curr_step < rec_step_in) {
                steps_holded = step_size - (rec_step_in - curr_step);
              } else if (curr_step > rec_step_in) {
                steps_holded = curr_step - rec_step_in;
              }

              if (steps_holded > 0) {
                uint8_t end_step = start_step+steps_holded;
                for (uint8_t i=start_step; i < end_step; i++) {
                  uint8_t step_idx = i%step_size;
                  // set step to rest mode and tie it and same note holded!
                  AcidSequencer.rest(rec_track, step_idx, true);
                  AcidSequencer.setTie(rec_track, step_idx, true);
                  AcidSequencer.setStepData(rec_track, step_idx, rec_note);
                }
              }
            }
          }
          // send note off
          sendNote(rec_note, AcidSequencer.getTrackChannel(rec_track), 0);
        }
      }
    }
    
    void function1() {
      if (selected_line == 1) {
        // mute track(subtrack for 808)
        AcidSequencer.setMute(_selected_track, !AcidSequencer.getMute(_selected_track));
      } else if (selected_line >= 2) {
        // 303 and 808 uses the same accent button f1
        AcidSequencer.setAccent(_selected_track, selected_step, !AcidSequencer.accentOn(_selected_track, selected_step));
      }
    }

    void function2() {
      if (selected_line == 1) {
        // clear track
        AcidSequencer.clearTrack(_selected_track);
      } else if (selected_line >= 2) {
        // 303
        if (AcidSequencer.is303(_selected_track)) {
          if ((AcidSequencer.stepOn(_selected_track, selected_step-1) || AcidSequencer.tieOn(_selected_track, selected_step-1)) && !AcidSequencer.stepOn(_selected_track, selected_step) && selected_step != 0) {
            AcidSequencer.setTie(_selected_track, selected_step, !AcidSequencer.tieOn(_selected_track, selected_step));
          } else {
            AcidSequencer.setSlide(_selected_track, selected_step, !AcidSequencer.slideOn(_selected_track, selected_step));
          }
        // 808
        } else {
          AcidSequencer.setRoll(_selected_track, selected_step, !AcidSequencer.rollOn(_selected_track, selected_step));
        }
      }
    }

    void selectorButton(uint8_t nunmber) {
      selected_locator = nunmber;
    }

    void stepButton(uint8_t number) {
      uint8_t step = number + ((full_size_view && selected_line != 1 ? selected_line-2 : selected_locator) * 16);
      AcidSequencer.rest(_selected_track, step, AcidSequencer.stepOn(_selected_track, step));
    }
    
} stepSequencerComponent;

struct TrackLength : PageComponent {

    void view() {
      genericOptionView("lenght", AcidSequencer.getTrackLength(_selected_track), line, col, selected);
    }

    void change(int16_t data) {
      //clearStackNote(_selected_track);
      data = parseData(data, 1, AcidSequencer.getTrackMaxLength(_selected_track), AcidSequencer.getTrackLength(_selected_track));
      AcidSequencer.setTrackLength(_selected_track, data);
    }
    
} lengthComponent;

struct SequenceShift : PageComponent {
  
    void view() {
      genericOptionView("shift", AcidSequencer.getShiftPos(_selected_track), line, col, selected);
    }

    void change(int16_t data) {
      //clearStackNote(_selected_track);
      data = parseData(data, AcidSequencer.getTrackLength(_selected_track)*-1, AcidSequencer.getTrackLength(_selected_track), AcidSequencer.getShiftPos(_selected_track));
      AcidSequencer.setShiftPos(_selected_track, data);
    }
    
} shiftComponent;

struct Transpose : PageComponent {
  
    void view() {
      genericOptionView("transpose", AcidSequencer.getTranspose(_selected_track), line, col, selected);
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

// make a seq divider too!

struct VoiceSelect : PageComponent {
  
    void view() {
      genericOptionView("voice", AcidSequencer.getTrackVoiceName(_selected_track, AcidSequencer.getTrackVoice(_selected_track)), line, col, selected);
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
      genericOptionView("note", AcidSequencer.getNoteString(AcidSequencer.getTrackVoiceConfig(_selected_track)), line, col, selected);
    }

    void change(int16_t data) {
      //clearStackNote(_selected_track);
      data = parseData(data, 0, 127, AcidSequencer.getTrackVoiceConfig(_selected_track));
      AcidSequencer.setTrackVoiceConfig(_selected_track, data);
      // send note for preview while change data
      sendNote(data, AcidSequencer.getTrackChannel(_selected_track), NOTE_VELOCITY_808);
      sendNote(data, AcidSequencer.getTrackChannel(_selected_track), 0);
    }
    
} voiceConfigComponent;

struct TrackTune : PageComponent {
  
    void view() {
      genericOptionView("tune", AcidSequencer.getTune(_selected_track) == 0 ? "off" : AcidSequencer.getNoteString(AcidSequencer.getTune(_selected_track)-1), line, col, selected);
    }

    void change(int16_t data) {
      // 0 = harmonizer off
      // 1 = C..., 12 = B
      //clearStackNote(_selected_track);
      data = parseData(data, 0, 12, AcidSequencer.getTune(_selected_track));
      AcidSequencer.setTune(_selected_track, data);
    }
    
} tuneComponent;

struct TonesNumber : PageComponent {

    void view() {
      genericOptionView("tones", _number_of_tones, line, col, selected);
    }

    void change(int16_t data) {
      data = parseData(data, 1, 7, _number_of_tones);
      _number_of_tones = data;
    }
    
} tonesNumberComponent;

struct LowOctave : PageComponent {
  
    void view() {
      genericOptionView("octave", _lower_octave, line, col, selected);
    }

    void change(int16_t data) {
      data = parseData(data, 1, 11, _lower_octave);
      _lower_octave = data;
    }
    
} lowOctaveComponent;

struct RangeOctave : PageComponent {
  
    void view() {
      genericOptionView("octaves", _range_octave, line, col, selected);
    }

    void change(int16_t data) {
      data = parseData(data, 1, 11, _range_octave);
      _range_octave = data;
    }
    
} rangeOctaveComponent;

struct AccentAmount : PageComponent {
  
    void view() {
      genericOptionView("accent", _accent_probability, line, col, selected);
    }

    void change(int16_t data) {
      data = parseData(data, 0, 100, _accent_probability);
      _accent_probability = data;
    }
    
} accentAmountComponent;

struct SlideAmount : PageComponent {
  
    void view() {
      genericOptionView("slide", _slide_probability, line, col, selected);
    }

    void change(int16_t data) {
      data = parseData(data, 0, 100, _slide_probability);
      _slide_probability = data;
    }
    
} slideAmountComponent;

struct TieAmount : PageComponent {
  
    void view() {
      genericOptionView("tie", _tie_probability, line, col, selected);
    }

    void change(int16_t data) {
      data = parseData(data, 0, 100, _tie_probability);
      _tie_probability = data;
    }
    
} tieAmountComponent;

struct RollAmount : PageComponent {
  
    void view() {
      genericOptionView("roll", _roll_probability, line, col, selected);
    }

    void change(int16_t data) {
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

    void change(int16_t data) {
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

    void change(int16_t data) {
      //clearStackNote(_selected_track);
      data = parseData(data, 1, 100, _generative_fill);
      _generative_fill = data;
    }
    
} fillComponent;
