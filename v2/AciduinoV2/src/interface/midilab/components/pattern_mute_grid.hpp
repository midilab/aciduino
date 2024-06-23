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
    }
    
    void view() {
      // update selected mute block based on selected track(fast nav using track change)
      uint8_t selected_track = aciduino.getSelectedTrack();
      if (selected_track < TRACK_NUMBER_303) {
        selected_mute_chn = selected_track;
        element_index = 0;
      } else {
        if (selected_track_ref != selected_track) {
          selected_mute_chn = 0;
          element_index = ((selected_track - TRACK_NUMBER_303) * VOICE_MAX_SIZE_808) + TRACK_NUMBER_303;
          selected_track_ref = selected_track;
        }
      }
      
      // mute pattern
      for (uint8_t i=0; i < 4; i++) {
        
        // mute grid
        for (uint8_t j=0; j < max_elements_per_grid; j++) {
          // default block base
          uCtrl.oled->drawBox(y+(i*8), x+(j*block_size), 6, block_size-2, i+1 == selected_line && j == selected_mute_chn ? true : false);
          
          // 303
          if (aciduino.seq.is303(j+element_index)) {
            if (!aciduino.getMuteGridState(i, j+element_index)) {
              uCtrl.oled->drawBox(y+(i*8)+1, x+(j*block_size)+1, 4, block_size-4, i+1 == selected_line && j == selected_mute_chn ? true : false);
            }
          // 808
          } else {
            uint8_t map_ref = (j+element_index-TRACK_NUMBER_303);
            uint8_t track = (map_ref/VOICE_MAX_SIZE_808)+TRACK_NUMBER_303;
            uint8_t voice = map_ref%VOICE_MAX_SIZE_808;
            if (!aciduino.getMuteGridState(i, track, voice)) {
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
      //uCtrl.oled->print(String(aciduino.getSelectedTrack()+1), line+4, 1);
      if (aciduino.seq.is303(aciduino.getSelectedTrack())) {
        //if (aciduino.seq.stepOn(aciduino.getSelectedTrack(), aciduino.seq.getCurrentStep(aciduino.getSelectedTrack())) && aciduino.isPlaying()) {
        //  uCtrl.oled->print(aciduino.seq.getNoteString(aciduino.seq.getStepData(aciduino.getSelectedTrack(), aciduino.seq.getCurrentStep(aciduino.getSelectedTrack()))), line+4, 1);
        //}
        //uCtrl.oled->print("303", line+4, 1);
        //uCtrl.oled->print(String(aciduino.getSelectedTrack()+1), line+4, 7);
      // 808
      } else {
        uCtrl.oled->print(aciduino.seq.getTrackVoiceName(aciduino.getSelectedTrack(), aciduino.seq.getTrackVoice(aciduino.getSelectedTrack())), line+4, 1);
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
                //selected_mute_chn = aciduino.seq.is303(aciduino.getSelectedTrack()) ? 0 : VOICE_MAX_SIZE_808-1;
              }
            } else {
              --selected_mute_chn;
            }      
            
            map_ref = selected_mute_chn + element_index;
            if (map_ref < TRACK_NUMBER_303) {
              aciduino.setSelectedTrack(map_ref);
            } else {
              aciduino.setSelectedTrack(((map_ref - TRACK_NUMBER_303) / VOICE_MAX_SIZE_808) + TRACK_NUMBER_303);
              // selected_mute_channel_rel = map_ref%VOICE_MAX_SIZE_808;
              aciduino.seq.setTrackVoice(aciduino.getSelectedTrack(), (map_ref-TRACK_NUMBER_303)%VOICE_MAX_SIZE_808);
              //aciduino.seq.setTrackVoice(aciduino.getSelectedTrack(), selected_mute_chn);
            }
            selected_track_ref = aciduino.getSelectedTrack();
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
              aciduino.setSelectedTrack(map_ref);
            } else {
              aciduino.setSelectedTrack(((map_ref - TRACK_NUMBER_303) / VOICE_MAX_SIZE_808) + TRACK_NUMBER_303);
              // selected_mute_channel_rel = map_ref%VOICE_MAX_SIZE_808;
              aciduino.seq.setTrackVoice(aciduino.getSelectedTrack(), (map_ref-TRACK_NUMBER_303)%VOICE_MAX_SIZE_808);
              //aciduino.seq.setTrackVoice(aciduino.getSelectedTrack(), selected_mute_chn);
            }
            selected_track_ref = aciduino.getSelectedTrack();
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
        if (aciduino.seq.is303(aciduino.getSelectedTrack())) {
          aciduino.setMuteGridState(pattern, 1, aciduino.getSelectedTrack());
        } else {
          aciduino.setMuteGridState(pattern, 1, aciduino.getSelectedTrack(), (selected_mute_chn+element_index-TRACK_NUMBER_303)%VOICE_MAX_SIZE_808);
        }
        // selected is same as current? update tracks mute state
        if (pattern == current_pattern)
          changePattern(current_pattern);
      } else if (data == DECREMENT) {
        if (aciduino.seq.is303(aciduino.getSelectedTrack())) {
          aciduino.setMuteGridState(pattern, 0, aciduino.getSelectedTrack());
        } else {
          aciduino.setMuteGridState(pattern, 0, aciduino.getSelectedTrack(), (selected_mute_chn+element_index-TRACK_NUMBER_303)%VOICE_MAX_SIZE_808);
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
        aciduino.seq.setMute(i, !aciduino.getMuteGridState(pattern, i));
      }

      // apply mute schema to 808s
      for (uint8_t i=0; i < TRACK_NUMBER_808; i++) {
        for (uint8_t j=0; j < VOICE_MAX_SIZE_808; j++) {
          aciduino.seq.setMute(i+TRACK_NUMBER_303, j, !aciduino.getMuteGridState(pattern, i+TRACK_NUMBER_303, j));
        }
      }
    }

    void updateCurrentMuteState(uint8_t track, uint8_t voice = 0)
    {
      if (aciduino.seq.is303(track)) {
        if (aciduino.seq.getMute(track)) {
          aciduino.setMuteGridState(current_pattern, 0, track);
        } else {
          aciduino.setMuteGridState(current_pattern, 1, track);
        }
      } else {
        if (aciduino.seq.getMute(track, aciduino.seq.getTrackVoice(track))) {
          aciduino.setMuteGridState(current_pattern, 0, track, voice);
        } else {
          aciduino.setMuteGridState(current_pattern, 1, track, voice);
        }
        // update to selected voice too
        selected_mute_chn = voice;
      }
    }
    
} mutePatternComponent;