// the control knob will handle midi cc for the last selected midi cc of selected track inside this component
struct PatternControl : PageComponent {
    
    // layout:
    // mute automation pattern style driven
    // 4x16 grid style:
    uint8_t current_pattern = 0;
    uint8_t selected_pattern = 0;
    uint8_t selected_track = 0;     // index inside the active track list
    // active slots in slot order: the UI never assumes a track type layout
    uint8_t slots[SEQ_MAX_TRACKS];
    uint8_t slot_count = 0;
    uint8_t max_elements_per_grid = 0;
    uint8_t block_size = 0;
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

    // rebuild the live track list(dynamic create/delete leaves holes in slots)
    void buildSlots()
    {
      slot_count = 0;
      for (uint8_t t=0; t < SEQ_MAX_TRACKS; t++) {
        if (aciduino.seq.trackType(t) != SEQ_TRACK_NONE) {
          slots[slot_count++] = t;
        }
      }
      max_elements_per_grid = slot_count < 11 ? slot_count : 11;
      block_size = max_elements_per_grid > 0 ? (uint8_t)ceil(86.00 / max_elements_per_grid) : 0;
      if (selected_track >= slot_count) {
        selected_track = slot_count > 0 ? slot_count - 1 : 0;
      }
    }

    // first visible element, keeps the selection inside the 11 cell window
    uint8_t windowStart()
    {
      return (max_elements_per_grid > 0 && selected_track >= max_elements_per_grid) ? selected_track - (max_elements_per_grid - 1) : 0;
    }

    // last selectable pattern: the pending empty slot on sdcard, last saved on eeprom
    uint16_t lastPattern()
    {
      uint16_t saved_patterns = aciduino.getNumOfPatterns();
      return aciduino.hasPendingPatternSlot() ? saved_patterns : (saved_patterns ? saved_patterns - 1 : 0);
    }

    void view() {
      buildSlots();
      // map the selected slot to the live track list
      uint8_t selected_slot = aciduino.getSelectedTrack();
      for (uint8_t i=0; i < slot_count; i++) {
        if (slots[i] == selected_slot) {
          selected_track = i;
          break;
        }
      }
      selected_pattern = selected_line-2+pattern_index;
      uint8_t line_count = 1;
      uint8_t start = windowStart();
      uint16_t saved_patterns = aciduino.getNumOfPatterns();
      uint16_t last_pattern = lastPattern();

    
      if (selected_line == 1)
        selected_line = 2;
        
      // ui for all vs single
      // for all blink twice fast as selected one for the pattern line
      // patterns
      for (uint16_t i=pattern_index; i < (uint16_t)pattern_index+4; i++) {

        // no pattern space here: leave the row blank
        if (i > last_pattern) {
          line_count++;
          continue;
        }
        
        // pattern grid
        for (uint8_t j=0; j < max_elements_per_grid; j++) {
          uint8_t idx = start + j;
          if (idx >= slot_count) {
            continue;
          }
          uint8_t slot = slots[idx];
          bool selected = (line_count+1 == selected_line && idx == selected_track) || (select_all == true && line_count+1 == selected_line);
          // default block base
          uCtrl.oled->drawBox(y+(line_count*8), x+(j*block_size), 6, block_size-2, selected);
          if (aciduino.getPatternGrid(slot) != i) {
            // playing pattern box
            uCtrl.oled->drawBox(y+(line_count*8)+1, x+(j*block_size)+1, 4, block_size-4, selected);
          }
        }

        // pattern name: the pending empty slot waits to be saved to become a pattern
        if (aciduino.hasPendingPatternSlot() && i == saved_patterns) {
          uCtrl.oled->print("new", line+line_count, 20, line_count+1 == selected_line ? true : false);
        } else {
          uCtrl.oled->print("ptrn", line+line_count, 20, line_count+1 == selected_line ? true : false);
          uCtrl.oled->print(String(i+1), line+line_count, 25, line_count+1 == selected_line ? true : false);
        }

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
      buildSlots();

      switch (dir) {
        case LEFT:
            if (selected_track > 0) {
              --selected_track;
            }
            if (slot_count > 0) {
              aciduino.setSelectedTrack(slots[selected_track]);
            }
          break;
        case RIGHT:
            if (selected_track + 1 < slot_count) {
              ++selected_track;
            }
            if (slot_count > 0) {
              aciduino.setSelectedTrack(slots[selected_track]);
            }
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
              // +1 for the pending empty slot on sdcard
              uint16_t pattern_slots = aciduino.getNumOfPatterns() + (aciduino.hasPendingPatternSlot() ? 1 : 0);
              if (pattern_slots > 4 && pattern_index < (uint16_t)(pattern_slots - 4)) {
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
      buildSlots();
      if (data == INCREMENT) {
        changePattern(selected_pattern);
      } else if (data == DECREMENT) {
        if (slot_count > 0) {
          changePattern(selected_pattern, slots[selected_track]);
        }
      }
    }
    
    void function1() {
      buildSlots();
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
            aciduino.copyPattern(selected_pattern);
          } else {
            // copy track
            if (slot_count > 0) {
              aciduino.copyPattern(selected_pattern, slots[selected_track]);
            }
          }
          copy_state = true;
        }
      }
    }

    void function2() {
      buildSlots();
      // save: memory and pattern selected per track as it is on grid
      if (uCtrl.page->isShiftPressed() && !(copy_state || save_as_state)) {
        // save the grid as it is selected per track
        for (uint8_t i=0; i < slot_count; i++) {
          aciduino.savePattern(aciduino.getPatternGrid(slots[i]), slots[i]);
        }
        // save _mute_grid for current_pattern only
        aciduino.saveMuteGrid(current_pattern);
      // paste/all, save as selection
      } else {
        if (copy_state) {
          // paste all
          if(select_all) {
            if (aciduino.pastePattern(selected_pattern)) {
              
            }
          // paste track
          } else {
            if (slot_count > 0 && aciduino.pastePattern(selected_pattern, slots[selected_track])) {
              
            }
          }
          copy_state = false;
        } else if (save_as_state) {
          // save as: gets all current memory and saves on selected pattern(all tracks)
          // save all memory current into selected pattern line
          aciduino.savePattern(selected_line-2+pattern_index);
          save_as_state = false;
        } else {
          // copy all selection
          select_all = !select_all;
        }
      }
    }

    void changePattern(uint8_t pattern, int8_t track = -1) {
      // beyond the pending empty slot? nothing to change to
      if (pattern > aciduino.getNumOfPatterns()) {
        return;
      }

      if (track == -1) {
        // load all tracks pattern
        aciduino.loadPattern(pattern);
        // the loaded image can bring a different track layout
        buildSlots();
        for (uint8_t i=0; i < slot_count; i++) {
          aciduino.setPatternGrid(slots[i], pattern);
        }
      } else {
        // loads only a track pattern
        aciduino.loadPattern(pattern, track);
        aciduino.setPatternGrid(track, pattern);
      }
      
      // update current pattern reference
      current_pattern = pattern;
    }

#if defined(USE_TRACK_STRIP)
    // step pads 1-16: patterns 1-16. tap loads on the selected track,
    // shift loads the pattern on every track (scene).
    void pad(uint8_t number, bool all)
    {
      buildSlots();
      uint8_t pattern = number;
      if (pattern > lastPattern()) {
        return;
      }

      // keep the oled window on the tapped pattern
      pattern_index = (pattern / 4) * 4;
      selected_line = (pattern % 4) + 2;
      selected_pattern = pattern;

      for (uint8_t i=0; i < slot_count; i++) {
        if (slots[i] == aciduino.getSelectedTrack()) {
          selected_track = i;
          break;
        }
      }

      if (all) {
        changePattern(pattern);
      } else if (slot_count > 0) {
        changePattern(pattern, slots[selected_track]);
      }
    }

    void padView()
    {
#if defined(USE_LED_24)
      uint8_t track = aciduino.getSelectedTrack();
      if (aciduino.seq.trackType(track) == SEQ_TRACK_NONE) {
        return;
      }
      uint8_t current = aciduino.getPatternGrid(track);
      uint16_t last = lastPattern();
      for (uint8_t i=0; i < 16; i++) {
        uCtrl.dout->write(STEP_LED_1 + i, (i == current && i <= last) ? HIGH : LOW);
      }
#endif
    }
#endif
    
} patternComponent;
