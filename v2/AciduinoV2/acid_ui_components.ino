//
// uCtrl PageComponent 
// all UI components are programmed as PageComponent to be reused on different pages
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

// generic controler for 303 and 808 devices
typedef struct
{
    const char * control_name;
    uint8_t control_cc = 0;
    uint8_t control_data[TRACK_NUMBER_303];
    int8_t pot_map[TRACK_NUMBER_303] = {-1};
} MIDI_CTRL_DATA_303;

typedef struct
{
    const char * control_name;
    uint8_t control_cc = 0;
    uint8_t control_data[TRACK_NUMBER_808];
    int8_t pot_map[TRACK_NUMBER_808] = {-1};
} MIDI_CTRL_DATA_808;

struct MidiCCControl : PageComponent {

    uint8_t selected_map = 0;
    uint8_t ctrl_selected = 0;

    uint8_t last_selected_track = 0;

#if defined(LEARN_ENABLED)
    const bool learn_enabled = true; 
#else
    const bool learn_enabled = false; 
#endif
    bool learn = false;

    // layout:
    // max 16 controls per track
    // (4 x 2) x 2
    // f1 = A (select A map)
    // f2 = B (select B map)
    // shift changes f1 and f2 to clear  |  learn/POTID
    MIDI_CTRL_DATA_303 control_map_303[16];
    uint8_t ctrl_size_303 = 0;

    MIDI_CTRL_DATA_808 control_map_808[16];
    uint8_t ctrl_size_808 = 0;
    
    MidiCCControl()
    {
      // we want this component to be 4 lines max and 2 grids navigable object
      line_size = 4;
      grid_size = 2;
      // enable more complex nav by updating selector pointer of page component
      update_selector_view = true;
      // initing pot_map for learn refrence
      for (uint8_t i=0; i < 16; i++) {
        for (uint8_t j=0; j < TRACK_NUMBER_303; j++) {
          control_map_303[i].pot_map[j] = -1;
        }
        for (uint8_t j=0; j < TRACK_NUMBER_808; j++) {
          control_map_808[i].pot_map[j] = -1;
        }
      }
    }

    void view() {
      uint8_t data_idx = AcidSequencer.is303(_selected_track) ? _selected_track : _selected_track - TRACK_NUMBER_303;
      uint8_t ctrl_map_init = selected_map == 0 ? 0 : 8;
      uint8_t ctrl_counter = 0;
      ctrl_selected = selected_line-1 + (selected_grid == 2 ? 4 : 0);
      ctrl_selected += selected_map > 0 ? 8 : 0;
      last_selected_track = _selected_track;

      // print controls
      for (uint8_t i=ctrl_map_init; i < ctrl_map_init+8; i++) {
        // process 303 controlelrs?
        if (AcidSequencer.is303(_selected_track)) {
          if (i >= ctrl_size_303) {
            // breaks with no selected element? fix it!
            if (ctrl_selected >= ctrl_size_303) {
              selected_grid = floor((selected_map > 0 ? ctrl_size_303-1-8 : ctrl_size_303-1)/4)+1;
              selected_line = ((selected_map > 0 ? ctrl_size_303-1-8 : ctrl_size_303-1)%4)+1;
            }
            break;
          }
          genericOptionView(control_map_303[i].control_name, String(control_map_303[i].control_data[data_idx]), line+(i%4), ctrl_counter >= 4 ? 14 : 1, i==ctrl_selected);
        // process 808 controlelrs?
        } else {
          if (i >= ctrl_size_808) {
            // breaks with no selected element? fix it!
            if (ctrl_selected >= ctrl_size_808) {
              selected_grid = floor((selected_map > 0 ? ctrl_size_808-1-8 : ctrl_size_808-1)/4)+1;
              selected_line = ((selected_map > 0 ? ctrl_size_808-1-8 : ctrl_size_808-1)%4)+1;
            }
            break;
          }
          genericOptionView(control_map_808[i].control_name, String(control_map_808[i].control_data[data_idx]), line+(i%4), ctrl_counter >= 4 ? 14 : 1, i==ctrl_selected);
        }
        ++ctrl_counter;
      }

      // shift only if learn enabled
      if ((uCtrl.page->isShiftPressed() || learn == true) && learn_enabled == true) {
        updateControlMap();
        int8_t pot_map = AcidSequencer.is303(_selected_track) ? control_map_303[ctrl_selected].pot_map[data_idx] : control_map_808[ctrl_selected].pot_map[data_idx];
        if (pot_map == -1) {
          setF2("learn", learn == true ? true : false);
        } else {
          // clear pot_map
          //setF1(String(String("clear") + String(pot_map)).c_str());
          setF2("clear");
        }
        if (learn == true) {
          setF1("cancel");
        } else {
          setF1("");
        }
      // no shift 
      } else {
        setF1("A", selected_map == 0 ? true : false);
        setF2("B", selected_map == 1 ? true : false);
      }
    }

    void change(int16_t data) {
      sendCCData(data, ctrl_selected, _selected_track);
    }
    
    void function1() {
      // shift 
      if ((uCtrl.page->isShiftPressed() || learn == true) && learn_enabled == true) {
        // cancel learn
        learn = false;
      } else {
        selected_map = 0;
      }
    }

    void function2() {
      // shift 
      if (uCtrl.page->isShiftPressed() && learn_enabled == true) {
        // learn!
        // lock all other pots
        // set waiting learn flag
        uint8_t data_idx = AcidSequencer.is303(_selected_track) ? _selected_track : _selected_track - TRACK_NUMBER_303;
        int8_t pot_map = AcidSequencer.is303(_selected_track) ? control_map_303[ctrl_selected].pot_map[data_idx] : control_map_808[ctrl_selected].pot_map[data_idx];
        if (pot_map == -1) {
          learn = true;
        } else {
          // clear pot_map
          clearCtrl(ctrl_selected);
        }
      } else {
        selected_map = 1;
      }
    }

    void sendCCData(int16_t data, uint8_t ctrl, uint8_t track, uint8_t interrupted = 0) {
      uint8_t data_idx = AcidSequencer.is303(track) ? track : track - TRACK_NUMBER_303;
      // process 303 controlelrs?
      if (AcidSequencer.is303(track)) {
        data = parseData(data, 0, 127, control_map_303[ctrl].control_data[data_idx]);
        control_map_303[ctrl].control_data[data_idx] = data;
        // send data
        sendMidiCC(control_map_303[ctrl].control_cc, data, _track_output_setup[track].channel, _track_output_setup[track].port, interrupted);
      } else {
        data = parseData(data, 0, 127, control_map_808[ctrl].control_data[data_idx]);
        control_map_808[ctrl].control_data[data_idx] = data;
        // send data
        sendMidiCC(control_map_808[ctrl].control_cc, data, _track_output_setup[track].channel, _track_output_setup[track].port, interrupted);
      }
    }

    void sendLastCCData(int16_t data) {
      sendCCData(data, ctrl_selected, last_selected_track);
    }
    
    void set303Control(const char * name, uint8_t cc, uint8_t initial_value = 0) {
      if (ctrl_size_303 >= 16)
        return;
      control_map_303[ctrl_size_303].control_name = name;
      control_map_303[ctrl_size_303].control_cc = cc;
      for (uint8_t i=0; i < TRACK_NUMBER_303; i++) {
        control_map_303[ctrl_size_303].control_data[i] = initial_value;
      }
      ++ctrl_size_303;
    }

    void set808Control(const char * name, uint8_t cc, uint8_t initial_value = 0) {
      if (ctrl_size_808 >= 16)
        return;
      control_map_808[ctrl_size_808].control_name = name;
      control_map_808[ctrl_size_808].control_cc = cc;
      for (uint8_t i=0; i < TRACK_NUMBER_808; i++) {
        control_map_808[ctrl_size_808].control_data[i] = initial_value;
      }
      ++ctrl_size_808;
    }

    void learnCtrl(uint8_t port)
    {
      uint8_t data_idx = AcidSequencer.is303(_selected_track) ? _selected_track : _selected_track - TRACK_NUMBER_303;
      if (learn == true) {
          _control_map_global[port].ctrl = ctrl_selected;
          _control_map_global[port].track = _selected_track;
          if (AcidSequencer.is303(_selected_track)) {
            control_map_303[ctrl_selected].pot_map[data_idx] = port;
          } else {
            control_map_808[ctrl_selected].pot_map[data_idx] = port;
          }
          learn = false;
      }
    }

    void clearCtrl(uint8_t port)
    {
      if (AcidSequencer.is303(_selected_track)) {
        _control_map_global[control_map_303[port].pot_map[_selected_track]].ctrl = -1;  
        control_map_303[port].pot_map[_selected_track] = -1;
      } else {
        _control_map_global[control_map_808[port].pot_map[_selected_track]].ctrl = -1;  
        control_map_808[port].pot_map[_selected_track-TRACK_NUMBER_303] = -1;
      }
    }

    void updateControlMap() {
      // update track control reference for pot map global
      for (uint8_t i=0; i < 16; i++) {
        if (_control_map_global[i].ctrl != -1) {
          if (_control_map_global[i].track < TRACK_NUMBER_303) {
            control_map_303[_control_map_global[i].ctrl].pot_map[_control_map_global[i].track] = i;
          } else {
            control_map_808[_control_map_global[i].ctrl].pot_map[_control_map_global[i].track-TRACK_NUMBER_303] = i;
          }
        }
      }
    }
    
} midiControllerComponent;

// used by ain in case any POT_X registred
void midiControllerHandle(uint8_t port, uint16_t value, uint8_t interrupted) {
  // check for learn request
  midiControllerComponent.learnCtrl(port);
  
  // anything into global learn map table?
  if (_control_map_global[port].ctrl != -1) {
    midiControllerComponent.sendCCData((int16_t)value, _control_map_global[port].ctrl, _control_map_global[port].track, interrupted);
  } else {
    midiControllerComponent.sendCCData((int16_t)value, port, _selected_track, interrupted);
  }
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

// the control knob will handle midi cc for the last selected midi cc of selected track inside this component
struct MutePatternControl : PageComponent {
    
    // layout:
    // mute automation pattern style driven
    // 4x16 grid style:
    // last 4 grids are mute automation clip change/status
    // 12 grids are for mute elements(on 808 max 12, for 303 1 big mute slot)
    // f1 = << (...)
    // f2 = >> (...)
    // this will be extended in the future to allow more patterns to be used, 4 for now!

    uint8_t current_pattern = 0;
    uint8_t selected_pattern = 0;
    uint8_t selected_mute_chn = 0;
    uint8_t element_index = 0;
    uint8_t selected_track_ref = 0;
    const uint8_t max_elements = TRACK_NUMBER_303 + (TRACK_NUMBER_808 * VOICE_MAX_SIZE_808);
    const uint8_t max_elements_per_grid = 11;
    const uint8_t block_size = ceil(86.00 / max_elements_per_grid);
    
    MutePatternControl()
    {
      // we want this component to be 4 lines max and 2 grids navigable object
      line_size = 4;
      grid_size = 2;
      // enable more complex nav by updating selector pointer of page component
      update_selector = true;

      // force initializatiron of mute pattern data
      for (uint8_t i=0; i < 4; i++) {
        for (uint8_t j=0; j < TRACK_NUMBER_808; j++) {
          _mute_pattern[i].map_808[j] = 0xFFFF;
        }
      }
    }
    
    void view() {
      // update selected mute block based on selected track(fast nav using track change)
      if (_selected_track < TRACK_NUMBER_303) {
        selected_mute_chn = _selected_track;
        element_index = 0;
      } else {
        if (selected_track_ref != _selected_track) {
          selected_mute_chn = 0;
          element_index = ((_selected_track - TRACK_NUMBER_303) * VOICE_MAX_SIZE_808) + TRACK_NUMBER_303;
          selected_track_ref = _selected_track;
        }
      }
      
      // mute pattern
      for (uint8_t i=0; i < 4; i++) {
        
        // mute grid
        for (uint8_t j=0; j < max_elements_per_grid; j++) {
          // default block base
          uCtrl.oled->drawBox(y+(i*8), x+(j*block_size), 6, block_size-2, i+1 == selected_line && j == selected_mute_chn ? true : false);
          
          // 303
          if (AcidSequencer.is303(j+element_index)) {
            if (!GET_BIT(_mute_pattern[i].map_303, j+element_index)) {
              uCtrl.oled->drawBox(y+(i*8)+1, x+(j*block_size)+1, 4, block_size-4, i+1 == selected_line && j == selected_mute_chn ? true : false);
            }
          // 808
          } else {
            uint8_t map_ref = (j+element_index-TRACK_NUMBER_303);
            if (!GET_BIT(_mute_pattern[i].map_808[map_ref/VOICE_MAX_SIZE_808], map_ref%VOICE_MAX_SIZE_808)) {
              uCtrl.oled->drawBox(y+(i*8)+1, x+(j*block_size)+1, 4, block_size-4, i+1 == selected_line && j == selected_mute_chn ? true : false);
            }
          }
        }

        // pattern selector status
        uCtrl.oled->print(String(i+1), line+i, 22, i+1 == selected_line ? true : false);
        if (current_pattern == i)
          uCtrl.oled->print("<", line+i, 25);
          
      }

      // track info
      //uCtrl.oled->print(String(_selected_track+1), line+4, 1);
      if (AcidSequencer.is303(_selected_track)) {
        //if (AcidSequencer.stepOn(_selected_track, AcidSequencer.getCurrentStep(_selected_track)) && _playing) {
        //  uCtrl.oled->print(AcidSequencer.getNoteString(AcidSequencer.getStepData(_selected_track, AcidSequencer.getCurrentStep(_selected_track))), line+4, 1);
        //}
        //uCtrl.oled->print("303", line+4, 1);
        //uCtrl.oled->print(String(_selected_track+1), line+4, 7);
      // 808
      } else {
        uCtrl.oled->print(AcidSequencer.getTrackVoiceName(_selected_track, AcidSequencer.getTrackVoice(_selected_track)), line+4, 1);
      }
      
      setF1("<<");
      setF2(">>");
    }

    void nav(uint8_t dir) {
      uint8_t map_ref;
      switch (dir) {
        case LEFT:
            if (selected_mute_chn == 0) {
              if (element_index != 0) {
                --element_index;
              } else {
                //selected_mute_chn = AcidSequencer.is303(_selected_track) ? 0 : VOICE_MAX_SIZE_808-1;
              }
            } else {
              --selected_mute_chn;
            }      
            
            map_ref = selected_mute_chn + element_index;
            if (map_ref < TRACK_NUMBER_303) {
              _selected_track = map_ref;
            } else {
              _selected_track = ((map_ref - TRACK_NUMBER_303) / VOICE_MAX_SIZE_808) + TRACK_NUMBER_303;
              // selected_mute_channel_rel = map_ref%VOICE_MAX_SIZE_808;
              AcidSequencer.setTrackVoice(_selected_track, (map_ref-TRACK_NUMBER_303)%VOICE_MAX_SIZE_808);
              //AcidSequencer.setTrackVoice(_selected_track, selected_mute_chn);
            }
            selected_track_ref = _selected_track;
          break;
        case RIGHT:
            if (selected_mute_chn == VOICE_MAX_SIZE_808-1) {
              //selected_mute_chn = 0;
              if (max_elements_per_grid + element_index < max_elements) {
                ++element_index;
              }
            } else {
              ++selected_mute_chn;
            }

            map_ref = selected_mute_chn + element_index;
            if (map_ref < TRACK_NUMBER_303) {
              _selected_track = map_ref;
            } else {
              _selected_track = ((map_ref - TRACK_NUMBER_303) / VOICE_MAX_SIZE_808) + TRACK_NUMBER_303;
              // selected_mute_channel_rel = map_ref%VOICE_MAX_SIZE_808;
              AcidSequencer.setTrackVoice(_selected_track, (map_ref-TRACK_NUMBER_303)%VOICE_MAX_SIZE_808);
              //AcidSequencer.setTrackVoice(_selected_track, selected_mute_chn);
            }
            selected_track_ref = _selected_track;
          break;
      }
      
    }
    
    void change(int16_t data) {
      // INCREMENT -1
      // DECREMENT -2
      // INCREMENT_SECONDARY -3
      // DECREMENT_SECONDARY -4
      uint8_t pattern = selected_line-1;
      if (data == INCREMENT) {
        if (AcidSequencer.is303(_selected_track)) {
          SET_BIT(_mute_pattern[pattern].map_303, _selected_track);
        } else {
          SET_BIT(_mute_pattern[pattern].map_808[_selected_track-TRACK_NUMBER_303], (selected_mute_chn+element_index-TRACK_NUMBER_303)%VOICE_MAX_SIZE_808);
        }
        // selected is same as current? update tracks mute state
        if (pattern == current_pattern)
          changePattern(current_pattern);
      } else if (data == DECREMENT) {
        if (AcidSequencer.is303(_selected_track)) {
          CLR_BIT(_mute_pattern[pattern].map_303, _selected_track);
        } else {
          CLR_BIT(_mute_pattern[pattern].map_808[_selected_track-TRACK_NUMBER_303], (selected_mute_chn+element_index-TRACK_NUMBER_303)%VOICE_MAX_SIZE_808);
        }
        if (pattern == current_pattern)
          changePattern(current_pattern);
      // secondary inc/dec or pot nav action
      } else {
        // redirect to the last used control on midi controller
        midiControllerComponent.sendLastCCData(data);
      }
    }
    
    void function1() {
      current_pattern = --current_pattern % 4;
      changePattern(current_pattern);
    }

    void function2() {
      current_pattern = ++current_pattern % 4;
      changePattern(current_pattern);
    }

    void changePattern(uint8_t pattern) {
      // apply mute schema to 303s
      for (uint8_t i=0; i < TRACK_NUMBER_303; i++) {
        AcidSequencer.setMute(i, !GET_BIT(_mute_pattern[pattern].map_303, i));
      }

      // apply mute schema to 808s
      for (uint8_t i=0; i < TRACK_NUMBER_808; i++) {
        for (uint8_t j=0; j < VOICE_MAX_SIZE_808; j++) {
          AcidSequencer.setMute(i+TRACK_NUMBER_303, j, !GET_BIT(_mute_pattern[pattern].map_808[i], j));
        }
      }
    }

    void updateCurrentMuteState(uint8_t track, uint8_t voice = 0)
    {
      if (AcidSequencer.is303(track)) {
        if (AcidSequencer.getMute(track)) {
          CLR_BIT(_mute_pattern[current_pattern].map_303, track);
        } else {
          SET_BIT(_mute_pattern[current_pattern].map_303, track);
        }
      } else {
        if (AcidSequencer.getMute(track, AcidSequencer.getTrackVoice(track))) {
          CLR_BIT(_mute_pattern[current_pattern].map_808[track-TRACK_NUMBER_303], voice);
        } else {
          SET_BIT(_mute_pattern[current_pattern].map_808[track-TRACK_NUMBER_303], voice);
        }
        // update to selected voice too
        selected_mute_chn = voice;
      }
    }
    
} mutePatternComponent;

// the control knob will handle midi cc for the last selected midi cc of selected track inside this component
struct PatternControl : PageComponent {
    
    // layout:
    // mute automation pattern style driven
    // 4x16 grid style:
    uint8_t current_pattern = 0;
    uint8_t selected_pattern = 0;
    uint8_t selected_track = 0;
    uint8_t selected_track_ref = 0;
    const uint8_t max_elements = TRACK_NUMBER_303 + TRACK_NUMBER_808;
    const uint8_t max_elements_per_grid = max_elements < 11 ? max_elements : 11;
    const uint8_t block_size = ceil(86.00 / max_elements_per_grid);
    uint8_t track_index = 0;
    uint8_t pattern_index = 0;
    bool copy_state = false;
    bool select_all = false;
    bool save_as_state = false;
    
    PatternControl()
    {
      // we want this component to be 4 lines max and 2 grids navigable object
      // the +2 lines are for scroll up/down usage
      line_size = 6;
      grid_size = 2;
      // enable more complex nav by updating selector pointer of page component
      update_selector = true;
    }

    void view() {
      // update selected mute block based on selected track(fast nav using track change)
      selected_track = _selected_track;
      selected_pattern = selected_line-2+pattern_index;
      uint8_t line_count = 1;

    
      if (selected_line == 1)
        selected_line = 2;
        
      // ui for all vs single
      // for all blink twice fast as selected one for the pattern line
      // patterns
      for (uint8_t i=pattern_index; i < pattern_index+4; i++) {
        
        // pattern grid
        for (uint8_t j=0; j < max_elements_per_grid; j++) {
          // default block base
          uCtrl.oled->drawBox(y+(line_count*8), x+(j*block_size), 6, block_size-2, (line_count+1 == selected_line && j == selected_track) || (select_all == true && line_count+1 == selected_line) ? true : false);
          if (_pattern_grid[j+track_index] != i) {
            // playing pattern box
            uCtrl.oled->drawBox(y+(line_count*8)+1, x+(j*block_size)+1, 4, block_size-4, (line_count+1 == selected_line && j == selected_track) || (select_all == true && line_count+1 == selected_line) ? true : false);
          }
        }

        // pattern name
        uCtrl.oled->print("ptrn", line+line_count, 20, line_count+1 == selected_line ? true : false);
        uCtrl.oled->print(String(i+1), line+line_count, 25, line_count+1 == selected_line ? true : false);

        line_count++;
      }

      // shift goes to copy/paste function
      if (uCtrl.page->isShiftPressed() && !(copy_state || save_as_state)) {
        setF1("save as");
        setF2("save");
      } else {
        // shift f1 goes copy, if copy state then no shift f1 goes paste instead of clear
        if (copy_state) {
          setF1("cancel");
          setF2("paste"); 
        } else if (save_as_state) {
          setF1("cancel");
          setF2("save as"); 
        } else {
          setF1("copy");
          setF2("all", select_all ? true : false);
        }
        
      }
    }

    void nav(uint8_t dir) {

      switch (dir) {
        case LEFT:
            if (selected_track == 0) {
              if (track_index != 0) {
                --track_index;
              } else {
                //selected_track = AcidSequencer.is303(_selected_track) ? 0 : VOICE_MAX_SIZE_808-1;
              }
            } else {
              --selected_track;
            }      
            
            _selected_track = selected_track + track_index;
            selected_track_ref = _selected_track;
          break;
        case RIGHT:
            if (selected_track < max_elements-1) {
              ++selected_track;
            }
            /*
            if (max_elements_per_grid + track_index < max_elements) {
              ++track_index;
            } else {
              ++selected_track;
            }*/

            _selected_track = selected_track + track_index;
            selected_track_ref = _selected_track;
          break;
        case UP:
            if (selected_line == 1) {
              if (pattern_index > 0) {
                pattern_index--;
              }
              selected_line = 2;
            }
          break;
        case DOWN:
            if (selected_line == 6) {
              if (pattern_index < EPRROM_PATTERN_AVAILABLE - 4) {
                pattern_index++;
              }
              selected_line = 5;
            }
          break;
      }
    }

    // changes here
    // use all for pattern grid changes too
    // and take inc for change as press and dec as change on pattern start(quantized pattern change)
    void change(int16_t data) {
      // INCREMENT -1 // changes whole pattern tracks
      // DECREMENT -2 // changes only a pattern track
      // INCREMENT_SECONDARY -3 // changes selected track
      // DECREMENT_SECONDARY -4 // changes selected track
      // POT // last used midi control
      if (data == INCREMENT) {
        changePattern(selected_pattern);
      } else if (data == DECREMENT) {
        changePattern(selected_pattern, _selected_track);
      }
    }
    
    void function1() {
      if (uCtrl.page->isShiftPressed() && !(copy_state || save_as_state)) {
        save_as_state = true;
      } else {
        if (copy_state || save_as_state) {
          // cancel
          copy_state = false;
          save_as_state = false;
        } else {
          // copy
          if (select_all) {
            // copy all
            copyPattern(selected_pattern);
          } else {
            // copy track
            copyPattern(selected_pattern, _selected_track);
          }
          copy_state = true;
        }
      }
    }

    void function2() {
      // save: memory and pattern selected per track as it is on grid
      if (uCtrl.page->isShiftPressed() && !(copy_state || save_as_state)) {
        // save the grid as it is selected per track
        for (uint8_t i=0; i < max_elements; i++) {
          savePattern(_pattern_grid[i], i);
        }
        // save _mute_grid for current_pattern only
        saveMuteGrid(current_pattern);
      // paste/all, save as selection
      } else {
        if (copy_state) {
          // paste all
          if(select_all) {
            if (pastePattern(selected_pattern)) {
              
            }
          // paste track
          } else {
            if (pastePattern(selected_pattern, _selected_track)) {
              
            }
          }
          copy_state = false;
        } else if (save_as_state) {
          // save as: gets all current memory and saves on selected pattern(all tracks)
          // save all memory current into selected pattern line
          savePattern(selected_line-2+pattern_index);
          save_as_state = false;
        } else {
          // copy all selection
          select_all = !select_all;
        }
      }
    }

    void changePattern(uint8_t pattern, int8_t track = -1) {
      if (track == -1) {
        // load all tracks pattern
        loadPattern(pattern);
        for (uint8_t i=0; i < max_elements; i++) {
          _pattern_grid[i] = pattern;
        }
      } else {
        // loads only a track pattern
        loadPattern(pattern, track);
        _pattern_grid[track] = pattern;
      }
      
      // update current pattern reference
      current_pattern = pattern;
    }
    
} patternComponent;

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
    uint16_t step_size = 0;
    uint8_t pot_last_note = 36;
    bool full_size_view = false;
    // rec and preview function helpers
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
        selected_locator = selected_step / (full_size_view ? 64 : 16); 
        step_size = AcidSequencer.getTrackLength(_selected_track);
        line_size = full_size_view ? (selected_locator == locator_length-1 ? ceil((step_size - (selected_locator * 64)) / 16) : 4) + 1 : 2;
      }
    }

    void view() {
      uint8_t steps_line = line + 1;
      uint8_t line_idx = 0, step_on = 0;
      uint8_t max_steps_per_view = full_size_view ? 64 : 16;

      curr_step = AcidSequencer.getCurrentStep(_selected_track);
      step_size = AcidSequencer.getTrackLength(_selected_track);
      locator_current = curr_step == 0 ? 0 : curr_step / max_steps_per_view;
      locator_length = ceil((float)step_size/max_steps_per_view);
      // fix selected step changes on tracks with different track length
      selected_step = selected_step >= step_size ? step_size - 1 : selected_step;
      selected_locator = selected_locator >= locator_length ? locator_current : selected_locator;
      // force it again in case track length change
      line_size = full_size_view ? (selected_locator == locator_length-1 ? ceil((step_size - (selected_locator * 64)) / 16) : 4) + 1 : 2;

#if defined(USE_LED_8) || defined(USE_LED_16) || defined(USE_LED_24)
      // locator leds
      uCtrl.dout->writeAll(LOW);
      if (locator_current !=  selected_locator || !selected) {
        uCtrl.dout->write(locator_current+SELECTOR_LED_1, HIGH); 
      }
      // selected locator and selected componet?
      if ((selected_line == 1 || selected_line == 2) && selected) {
        uCtrl.dout->write(selected_locator+SELECTOR_LED_1, uCtrl.dout->blink() ? LOW : HIGH);
      }
#endif
      
      // step locators
      uint8_t bar_size = 128 / locator_length;
      for (uint8_t i=0; i < locator_length; i++) {
        uCtrl.oled->drawBox(y+(locator_current == i ? 3 : 4), x+(bar_size*i)+2, (locator_current == i ? 3 : 1), bar_size-4, (selected_locator == i && (selected_line == 1 || selected_line >= 2) && selected));
      }

      // steps
      uint16_t first_step = selected ? selected_locator * max_steps_per_view : locator_current * max_steps_per_view;
      uint16_t last_step = first_step + max_steps_per_view > step_size ? step_size : first_step + max_steps_per_view;
      for (uint16_t step_idx=first_step; step_idx < last_step; step_idx++) {
        step_on = AcidSequencer.stepOn(_selected_track, step_idx);
        line_idx = step_idx % 16;

        // only use this in full sized mode
        if (line_idx == 0 && step_idx%64 != 0 && full_size_view) {
          steps_line++;
        }
    
        // reset step asset memory 8 bytes array
        memset(step_asset, 0x00, sizeof(step_asset));
    
        // step on/off?
        if (step_on) {
          uCtrl.oled->mergeBitmap(step_asset, (uint8_t*)STEP_ON);
#if defined(USE_LED_16) || defined(USE_LED_24)
          uCtrl.dout->write(line_idx+STEP_LED_1, HIGH);
#endif
        } else {
          uCtrl.oled->mergeBitmap(step_asset, (uint8_t*)STEP_OFF);
        }
    
        // step time signature mark?
        if (step_idx % 4 == 0) {
          uCtrl.oled->mergeBitmap(step_asset, (uint8_t*)STEP_TIME_SIGNATURE_MARK);
        }
    
        // step current?
        if (curr_step == step_idx && _playing) {
          uCtrl.oled->mergeBitmap(step_asset, (uint8_t*)STEP_SELECTED);
#if defined(USE_LED_16) || defined(USE_LED_24)
          uCtrl.dout->write(line_idx+STEP_LED_1, step_on ? LOW : HIGH);
#endif
        }
    
        // step selected?
        if (selected_step == step_idx && selected && selected_line >= 2) {
           uCtrl.oled->mergeBitmap(step_asset, (uint8_t*)STEP_SELECTED, true);
#if defined(USE_LED_16) || defined(USE_LED_24)
           uCtrl.dout->write(line_idx+STEP_LED_1, uCtrl.dout->blink() ? LOW : HIGH);
#endif
        }
    
        uCtrl.oled->print(step_asset, steps_line, line_idx+1);
      }

      // step info
      uint8_t info_line_idx = full_size_view ? 5 : 2;
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
          setF1("clear");
          setF2("mute", AcidSequencer.is303(_selected_track) ? AcidSequencer.getMute(_selected_track) : AcidSequencer.getMute(_selected_track, AcidSequencer.getTrackVoice(_selected_track)));
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
          if (full_size_view && selected_line > 1) {
            if (selected_line == 5) {
              selected_step = (selected_step % 16) + (16*3) + (selected_locator*64);
            } else {
              selected_step = selected_step >= 16 ? selected_step - 16 : step_size - (16 - selected_step);
            }
            selected_locator = selected_step / 64;
            selected_line = ((selected_step % 64) / 16) + 2;
          }
          break;
        case DOWN:
          if (full_size_view && selected_line > 1) {
            if (selected_line == 2) {
              selected_step = (selected_step % 16) + (selected_locator*64);
            } else {
              selected_step = (selected_step + 16) % step_size;
            }
            selected_locator = selected_step / 64;
            selected_line = ((selected_step % 64) / 16) + 2;
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
              selected_locator = selected_step / (full_size_view ? 64 : 16);
            }
            // update the selected line if is full_size_view
            if (full_size_view && selected_line != 1) {
              selected_line = ((selected_step % 64) / 16) + 2;
            }
          }
          break;
        case RIGHT:
          // step locator
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
          // steps
          } else if(selected_line > 1) {
            if (selected_step == step_size-1) {
              selected_step = 0;
              selected_locator = 0;
            } else {
              ++selected_step;
              selected_locator = selected_step / (full_size_view ? 64 : 16);
            }
            // update the selected line if is full_size_view
            if (full_size_view && selected_line != 1) {
              selected_line = ((selected_step % 64) / 16) + 2;
            }
          }
          break;
      }

      // keep grid nav aligned for user best ux experience
      //selected_grid = (float)(selected_locator+1) / locator_length > 0.5 ? 2 : 1;
      //selected_grid = selected_step % 16 >= 8 ? 2 : 1;
      
    }
    
    void change(int16_t data) {
      
      if (data == DECREMENT || data == INCREMENT) {

        // toggle on/off step with generic button 2(INCREMENT)
        if (data == INCREMENT && selected_line != 1) {
          AcidSequencer.rest(_selected_track, selected_step, AcidSequencer.stepOn(_selected_track, selected_step));
        // tap button for preview and realtime record with generic button 1(DECREMENT)
        } else {
          // step or realtime record
          // keep track to avoid changes on note while note on state
          rec_track = _selected_track;
          rec_note = AcidSequencer.is303(_selected_track) ? pot_last_note : AcidSequencer.getTrackVoiceConfig(_selected_track);
          AcidSequencer.input(rec_track, NOTE_ON, rec_note, AcidSequencer.is303(rec_track) ? NOTE_VELOCITY_303 : NOTE_VELOCITY_808);
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
            sendNote(data, _track_output_setup[_selected_track].channel, _track_output_setup[_selected_track].port, NOTE_VELOCITY_808);
            sendNote(data, _track_output_setup[_selected_track].channel, _track_output_setup[_selected_track].port, 0);
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
          AcidSequencer.input(rec_track, NOTE_OFF, rec_note, AcidSequencer.is303(rec_track) ? NOTE_VELOCITY_303 : NOTE_VELOCITY_808);
        }
      }
    }
    
    void function1() {
      if (selected_line == 1) {
        // clear track
        AcidSequencer.clearTrack(_selected_track);
      } else if (selected_line >= 2) {
        // 303 and 808 uses the same accent button f1
        AcidSequencer.setAccent(_selected_track, selected_step, !AcidSequencer.accentOn(_selected_track, selected_step));
      }
    }

    void function2() {
      if (selected_line == 1) {
        // mute track(voice for 808)
        if (AcidSequencer.is303(_selected_track)) {
          AcidSequencer.setMute(_selected_track, !AcidSequencer.getMute(_selected_track));
          // keep mutePatternComponent updated for current mute state
          mutePatternComponent.updateCurrentMuteState(_selected_track);
        } else {
          AcidSequencer.setMute(_selected_track, AcidSequencer.getTrackVoice(_selected_track), !AcidSequencer.getMute(_selected_track, AcidSequencer.getTrackVoice(_selected_track)));
          mutePatternComponent.updateCurrentMuteState(_selected_track, AcidSequencer.getTrackVoice(_selected_track));
        }
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

struct MidiChannelConfig : PageComponent {

    void view() {
      genericOptionView("channel", String(_track_output_setup[_selected_track].channel+1), line, col, selected);
    }
    
    void change(int16_t data) {
      data = parseData(data, 0, 15, _track_output_setup[_selected_track].channel);
      ATOMIC(_track_output_setup[_selected_track].channel = data)
    }

} midiChannelConfigComponent;

struct TrackOutputSelector : PageComponent {

    void view() {
      genericOptionView("out", String(String("midi") + (_track_output_setup[_selected_track].port+1)), line, col, selected);
    }

    void change(int16_t data) {
      data = parseData(data, 0, uCtrl.midi->sizeOf()-1, _track_output_setup[_selected_track].port);
      ATOMIC(_track_output_setup[_selected_track].port = data)
    }

} trackOutputSelectorComponent;

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

struct SystemResources : PageComponent {

    uint8_t resource_id = 0;

    SystemResources() {
      grid_size = 2;    
    }

    void view() {

      switch(resource_id) {
        case 0:
          genericOptionView("free ram", String(freeRam()), line, col, selected, true);
          break;
        case 1:
          genericOptionView("epprom", String(EPPROM_SIZE), line, col, selected, true);
          break;
        case 2:
          //genericOptionView("session", String(String((EPPROM_SESSION_SIZE/(float)EEPROM.length())*100) + String("%")), line, col, selected, true);
          genericOptionView("session", String(EPPROM_SESSION_SIZE), line, col, selected, true);
          break;
        case 3:
          genericOptionView("pattern", String(EPRROM_PATTERN_AVAILABLE), line, col, selected, true);
          break;
      }
      
    }
    
    void change(int16_t data) {
      data = parseData(data, 0, 3, resource_id);
      resource_id = data;
    }
    
} systemResourcesComponent;

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

// make a seq divider too!

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

struct TonesNumber : PageComponent {

    void view() {
      genericOptionView("tones", String(_generative_303[_selected_track].number_of_tones), line, col, selected);
    }

    void change(int16_t data) {
      data = parseData(data, 1, 7, _generative_303[_selected_track].number_of_tones);
      _generative_303[_selected_track].number_of_tones = data;
    }
    
} tonesNumberComponent;

struct LowOctave : PageComponent {
  
    void view() {
      genericOptionView("octave", String(_generative_303[_selected_track].lower_octave), line, col, selected);
    }

    void change(int16_t data) {
      data = parseData(data, 1, 11, _generative_303[_selected_track].lower_octave);
      _generative_303[_selected_track].lower_octave = data;
    }
    
} lowOctaveComponent;

struct RangeOctave : PageComponent {
  
    void view() {
      genericOptionView("octaves", String(_generative_303[_selected_track].range_octave), line, col, selected);
    }

    void change(int16_t data) {
      data = parseData(data, 1, 11, _generative_303[_selected_track].range_octave);
      _generative_303[_selected_track].range_octave = data;
    }
    
} rangeOctaveComponent;

struct AccentAmount : PageComponent {
  
    void view() {
      uint8_t accent_probability = AcidSequencer.is303(_selected_track) ? _generative_303[_selected_track].accent_probability : _generative_808[_selected_track-TRACK_NUMBER_303].accent_probability; 
      genericOptionView("accent", String(accent_probability), line, col, selected);
    }

    void change(int16_t data) {
      uint8_t * accent_probability = AcidSequencer.is303(_selected_track) ? &_generative_303[_selected_track].accent_probability : &_generative_808[_selected_track-TRACK_NUMBER_303].accent_probability; 
      data = parseData(data, 0, 100, *accent_probability);
      *accent_probability = data;
    }

} accentAmountComponent;

struct SlideAmount : PageComponent {
  
    void view() {
      genericOptionView("slide", String(_generative_303[_selected_track].slide_probability), line, col, selected);
    }

    void change(int16_t data) {
      data = parseData(data, 0, 100, _generative_303[_selected_track].slide_probability);
      _generative_303[_selected_track].slide_probability = data;
    }
    
} slideAmountComponent;

struct TieAmount : PageComponent {
  
    void view() {
      genericOptionView("tie", String(_generative_303[_selected_track].tie_probability), line, col, selected);
    }

    void change(int16_t data) {
      data = parseData(data, 0, 100, _generative_303[_selected_track].tie_probability);
      _generative_303[_selected_track].tie_probability = data;
    }
    
} tieAmountComponent;

struct RollAmount : PageComponent {
  
    void view() {
      genericOptionView("roll", String(_generative_808[_selected_track-TRACK_NUMBER_303].roll_probability), line, col, selected);
    }

    void change(int16_t data) {
      data = parseData(data, 0, 100, _generative_808[_selected_track-TRACK_NUMBER_303].roll_probability);
      _generative_808[_selected_track-TRACK_NUMBER_303].roll_probability = data;
    }
    
} rollAmountComponent;

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

struct TrackFill : PageComponent {

    TrackFill() {
      // we want this component to be 2 grids navigable object
      grid_size = 2;
    }
    
    void view() {
      uint8_t generative_fill = AcidSequencer.is303(_selected_track) ? _generative_303[_selected_track].generative_fill : _generative_808[_selected_track-TRACK_NUMBER_303].generative_fill; 
      genericOptionView("fill", String(generative_fill), line, col, selected, true);
    }

    void change(int16_t data) {
      //clearStackNote(_selected_track);
      uint8_t * generative_fill = AcidSequencer.is303(_selected_track) ? &_generative_303[_selected_track].generative_fill : &_generative_808[_selected_track-TRACK_NUMBER_303].generative_fill; 
      data = parseData(data, 1, 100, *generative_fill);
      *generative_fill = data;
    }
    
} fillComponent;

// general functions

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

void playStop()
{
  if (_playing)
    uClock.stop();
  else
    uClock.start();
}

void recToggle()
{
  AcidSequencer.setRecStatus(!AcidSequencer.getRecStatus());
}

void previousTrack()
{
  if (_selected_track == 0) {
    _selected_track = AcidSequencer.getTrackNumber() - 1;
  } else {
    --_selected_track;
  }
}

void nextTrack()
{
  if (_selected_track == AcidSequencer.getTrackNumber() - 1) {
    _selected_track = 0;
  } else {
    ++_selected_track;
  }
}
