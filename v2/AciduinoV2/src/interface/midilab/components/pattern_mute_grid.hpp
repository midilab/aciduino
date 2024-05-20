#include "../../uCtrl.h"

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