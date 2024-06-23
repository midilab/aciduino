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
      selected_track = aciduino.getSelectedTrack();
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
          if (aciduino.getPatternGrid(j+track_index) != i) {
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
                //selected_track = aciduino.seq.is303(selected_track) ? 0 : VOICE_MAX_SIZE_808-1;
              }
            } else {
              --selected_track;
            }      
            
            aciduino.setSelectedTrack(selected_track + track_index);
            selected_track_ref = aciduino.getSelectedTrack();
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

            aciduino.setSelectedTrack(selected_track + track_index);
            selected_track_ref = aciduino.getSelectedTrack();
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
              if (pattern_index < aciduino.getNumOfPatterns() - 4) {
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
        changePattern(selected_pattern, selected_track);
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
            aciduino.copyPattern(selected_pattern);
          } else {
            // copy track
            aciduino.copyPattern(selected_pattern, selected_track);
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
          aciduino.savePattern(aciduino.getPatternGrid(i), i);
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
            if (aciduino.pastePattern(selected_pattern, selected_track)) {
              
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
      if (track == -1) {
        // load all tracks pattern
        aciduino.loadPattern(pattern);
        for (uint8_t i=0; i < max_elements; i++) {
          aciduino.setPatternGrid(i, pattern);
        }
      } else {
        // loads only a track pattern
        aciduino.loadPattern(pattern, track);
        aciduino.setPatternGrid(track, pattern);
      }
      
      // update current pattern reference
      current_pattern = pattern;
    }
    
} patternComponent;