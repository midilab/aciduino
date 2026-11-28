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

    uint8_t selected_pattern = 0;
    uint8_t selected_mute_chn = 0;
    uint8_t element_index = 0;
    // live mute elements: one per non-808 track, one per 808 part
    uint8_t element_track[SEQ_MAX_TRACKS * PARTS_808];
    uint8_t element_voice[SEQ_MAX_TRACKS * PARTS_808];
    uint8_t element_count = 0;
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

    // rebuild the element list from the live track list, in slot order
    void buildElements()
    {
      element_count = 0;
      for (uint8_t t=0; t < SEQ_MAX_TRACKS; t++) {
        if (aciduino.seq.trackType(t) == SEQ_TRACK_NONE) {
          continue;
        }
        if (aciduino.seq.is808(t)) {
          for (uint8_t v=0; v < PARTS_808; v++) {
            element_track[element_count] = t;
            element_voice[element_count] = v;
            ++element_count;
          }
        } else {
          element_track[element_count] = t;
          element_voice[element_count] = 0;
          ++element_count;
        }
      }
      // keep the window valid after a track is deleted
      if (element_count == 0) {
        element_index = 0;
        selected_mute_chn = 0;
      } else if (element_index >= element_count) {
        element_index = element_count > max_elements_per_grid ? element_count - max_elements_per_grid : 0;
        selected_mute_chn = element_count - 1 - element_index;
      }
    }

    void resolveElement(uint8_t idx, uint8_t &track, uint8_t &voice)
    {
      if (idx >= element_count) {
        track = 0;
        voice = 0;
        return;
      }
      track = element_track[idx];
      voice = element_voice[idx];
    }

    // keep the selected track/voice visible inside the 11 cell window
    void focusElement(uint8_t idx)
    {
      if (idx >= max_elements_per_grid) {
        element_index = idx - (max_elements_per_grid - 1);
        selected_mute_chn = max_elements_per_grid - 1;
      } else {
        element_index = 0;
        selected_mute_chn = idx;
      }
    }

    void selectElement()
    {
      uint8_t idx = element_index + selected_mute_chn;
      if (idx >= element_count) {
        return;
      }
      uint8_t track = element_track[idx];
      uint8_t voice = element_voice[idx];
      aciduino.setSelectedTrack(track);
      if (aciduino.seq.is808(track)) {
        aciduino.seq.setTrackVoice(track, voice);
      }
    }
    
    void view() {
      buildElements();
      // follow the selected track(and 808 part), also after a track is deleted
      uint8_t selected_track = aciduino.getSelectedTrack();
      uint8_t selected_voice = aciduino.seq.is808(selected_track) ? aciduino.seq.getTrackVoice(selected_track) : 0;
      for (uint8_t i=0; i < element_count; i++) {
        if (element_track[i] == selected_track && element_voice[i] == selected_voice) {
          focusElement(i);
          break;
        }
      }
      
      // mute pattern
      for (uint8_t i=0; i < 4; i++) {
        
        // mute grid
        for (uint8_t j=0; j < max_elements_per_grid; j++) {
          uint8_t idx = j + element_index;
          if (idx >= element_count) {
            continue;
          }
          uint8_t track, voice;
          resolveElement(idx, track, voice);
          // default block base
          uCtrl.oled->drawBox(y+(i*8), x+(j*block_size), 6, block_size-2, i+1 == selected_line && j == selected_mute_chn ? true : false);
          if (!aciduino.getMuteGridState(i, track, voice)) {
            uCtrl.oled->drawBox(y+(i*8)+1, x+(j*block_size)+1, 4, block_size-4, i+1 == selected_line && j == selected_mute_chn ? true : false);
          }
        }

        // pattern selector status
        uCtrl.oled->print(String(i+1), line+i, 22, i+1 == selected_line ? true : false);
        if (aciduino.getMutePattern() == i)
          uCtrl.oled->print("<", line+i, 25);
          
      }

      // track info
      if (aciduino.seq.is808(aciduino.getSelectedTrack())) {
        uCtrl.oled->print(aciduino.seq.getTrackVoiceName(aciduino.getSelectedTrack(), aciduino.seq.getTrackVoice(aciduino.getSelectedTrack())), line+4, 1);
      } else if (aciduino.seq.isPoly(aciduino.getSelectedTrack())) {
        uCtrl.oled->print(trackTypeLabel(aciduino.getSelectedTrack()), line+4, 1);
      }
      
      setF1("<<");
      setF2(">>");
    }

    void nav(uint8_t dir) {
      buildElements();
      if (element_count == 0) {
        return;
      }

      switch (dir) {
        case LEFT:
            if (selected_mute_chn == 0) {
              if (element_index != 0) {
                --element_index;
              }
            } else {
              --selected_mute_chn;
            }
            selectElement();
          break;
        case RIGHT:
            if (selected_mute_chn == max_elements_per_grid-1) {
              if (element_index + max_elements_per_grid < element_count) {
                ++element_index;
              }
            } else if (element_index + selected_mute_chn + 1 < element_count) {
              ++selected_mute_chn;
            }
            selectElement();
          break;
      }
      
    }
    
    void change(int16_t data) {
      // INCREMENT -1
      // DECREMENT -2
      // INCREMENT_SECONDARY -3
      // DECREMENT_SECONDARY -4
      buildElements();
      uint8_t idx = selected_mute_chn + element_index;
      if (idx >= element_count) {
        return;
      }
      uint8_t pattern = selected_line-1;
      uint8_t track, voice;
      resolveElement(idx, track, voice);
      if (data == INCREMENT) {
        aciduino.setMuteGridState(pattern, 1, track, voice);
        // selected is same as current? update tracks mute state
        if (pattern == aciduino.getMutePattern())
          changePattern(aciduino.getMutePattern());
      } else if (data == DECREMENT) {
        aciduino.setMuteGridState(pattern, 0, track, voice);
        if (pattern == aciduino.getMutePattern())
          changePattern(aciduino.getMutePattern());
      // secondary inc/dec or pot nav action
      } else {
        // redirect to the last used control on midi controller
        midiControllerComponent.sendLastCCData(data);
      }
    }
    
    void function1() {
      // the active mute pattern lives in the core so the track strip shares it
      aciduino.setMutePattern((aciduino.getMutePattern() + 3) % 4);
      changePattern(aciduino.getMutePattern());
    }

    void function2() {
      aciduino.setMutePattern((aciduino.getMutePattern() + 1) % 4);
      changePattern(aciduino.getMutePattern());
    }

    void changePattern(uint8_t pattern) {
      // apply mute schema to every active track(dynamic layout can leave empty slots)
      for (uint8_t i=0; i < SEQ_MAX_TRACKS; i++) {
        if (aciduino.seq.trackType(i) == SEQ_TRACK_NONE) {
          continue;
        }
        if (aciduino.seq.is808(i)) {
          for (uint8_t j=0; j < VOICE_MAX_SIZE_808; j++) {
            aciduino.seq.setMute(i, j, !aciduino.getMuteGridState(pattern, i, j));
          }
        } else {
          aciduino.seq.setMute(i, !aciduino.getMuteGridState(pattern, i));
        }
      }
    }

    void updateCurrentMuteState(uint8_t track, uint8_t voice = 0)
    {
      if (aciduino.seq.is808(track)) {
        if (aciduino.seq.getMute(track, aciduino.seq.getTrackVoice(track))) {
          aciduino.setMuteGridState(aciduino.getMutePattern(), 0, track, voice);
        } else {
          aciduino.setMuteGridState(aciduino.getMutePattern(), 1, track, voice);
        }
      } else {
        if (aciduino.seq.getMute(track)) {
          aciduino.setMuteGridState(aciduino.getMutePattern(), 0, track);
        } else {
          aciduino.setMuteGridState(aciduino.getMutePattern(), 1, track);
        }
      }
    }

    // a live track mute is also captured by the active mute pattern
    void syncTrackMute(uint8_t track)
    {
      // 808 track mute is independent from the per part mute grid
      if (aciduino.seq.is808(track)) {
        return;
      }
      aciduino.setMuteGridState(aciduino.getMutePattern(), !aciduino.seq.getMute(track), track);
    }

} mutePatternComponent;
