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
        step_size = aciduino.seq.getTrackLength(aciduino.getSelectedTrack());
        line_size = full_size_view ? (selected_locator == locator_length-1 ? ceil((step_size - (selected_locator * 64)) / 16) : 4) + 1 : 2;
      }
    }

    void view() {
      uint8_t steps_line = line + 1;
      uint8_t line_idx = 0, step_on = 0;
      uint8_t max_steps_per_view = full_size_view ? 64 : 16;

      curr_step = aciduino.seq.getCurrentStep(aciduino.getSelectedTrack());
      step_size = aciduino.seq.getTrackLength(aciduino.getSelectedTrack());
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
        step_on = aciduino.seq.stepOn(aciduino.getSelectedTrack(), step_idx);
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
        if (curr_step == step_idx && aciduino.isPlaying()) {
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
      if (aciduino.seq.is303(aciduino.getSelectedTrack())) {
        if (selected_line >= 2) {
          uCtrl.oled->print(aciduino.seq.getNoteString(aciduino.seq.getStepData(aciduino.getSelectedTrack(), selected_step)), line+info_line_idx, 1);
        } else{
          if (aciduino.seq.stepOn(aciduino.getSelectedTrack(), curr_step) && aciduino.isPlaying()) {
            uCtrl.oled->print(aciduino.seq.getNoteString(aciduino.seq.getStepData(aciduino.getSelectedTrack(), curr_step)), line+info_line_idx, 1);
          }
        }
      // 808
      } else {
        uCtrl.oled->print(aciduino.seq.getTrackVoiceName(aciduino.getSelectedTrack(), aciduino.seq.getTrackVoice(aciduino.getSelectedTrack())), line+info_line_idx, 1);
        //uCtrl.oled->print(aciduino.seq.getNoteString(aciduino.seq.getTrackVoiceConfig(aciduino.getSelectedTrack())), line+info_line_idx, 1);
      }

      // f1 and f2
      // Selectors
      if (selected_line == 1) {
          setF1("clear");
          setF2("mute", aciduino.seq.is303(aciduino.getSelectedTrack()) ? aciduino.seq.getMute(aciduino.getSelectedTrack()) : aciduino.seq.getMute(aciduino.getSelectedTrack(), aciduino.seq.getTrackVoice(aciduino.getSelectedTrack())));
      // Steps
      } else if (selected_line >= 2) {
        setF1("accent", aciduino.seq.accentOn(aciduino.getSelectedTrack(), selected_step));
        // 303
        if (aciduino.seq.is303(aciduino.getSelectedTrack())) {
          if ((aciduino.seq.stepOn(aciduino.getSelectedTrack(), selected_step-1) || aciduino.seq.tieOn(aciduino.getSelectedTrack(), selected_step-1)) && !aciduino.seq.stepOn(aciduino.getSelectedTrack(), selected_step) && selected_step != 0) {
            setF2("tie", aciduino.seq.tieOn(aciduino.getSelectedTrack(), selected_step));
          } else {
            setF2("slide", aciduino.seq.slideOn(aciduino.getSelectedTrack(), selected_step));
          }
        // 808
        } else {
          setF2("roll", aciduino.seq.rollOn(aciduino.getSelectedTrack(), selected_step));
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
          aciduino.seq.rest(aciduino.getSelectedTrack(), selected_step, aciduino.seq.stepOn(aciduino.getSelectedTrack(), selected_step));
        // tap button for preview and realtime record with generic button 1(DECREMENT)
        } else {
          // step or realtime record
          // keep track to avoid changes on note while note on state
          rec_track = aciduino.getSelectedTrack();
          rec_note = aciduino.seq.is303(aciduino.getSelectedTrack()) ? pot_last_note : aciduino.seq.getTrackVoiceConfig(aciduino.getSelectedTrack());
          aciduino.seq.input(rec_track, NOTE_ON, rec_note, aciduino.seq.is303(rec_track) ? NOTE_VELOCITY_303 : NOTE_VELOCITY_808);
        }

      // secondary inc/dec or pot nav action
      } else {
        
        // 303?
        if (aciduino.seq.is303(aciduino.getSelectedTrack())) {

          uint8_t octave, note;
        
          octave = floor(aciduino.seq.getStepData(aciduino.getSelectedTrack(), selected_step) / 12);
          note = aciduino.seq.getStepData(aciduino.getSelectedTrack(), selected_step) - (octave * 12);
  
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
            aciduino.seq.setStepData(aciduino.getSelectedTrack(), selected_step, pot_last_note);
          }
        // 808
        } else {

          // shift sets voice config
          if (uCtrl.page->isShiftPressed()) {
            // select voice note[midi] or port[cv]
            data = parseData(data, 0, 127, aciduino.seq.getTrackVoiceConfig(aciduino.getSelectedTrack()));
            aciduino.seq.setTrackVoiceConfig(aciduino.getSelectedTrack(), data);
            // send note for preview while change data
            aciduino.sendNote(data, aciduino.getTrackOutputParam(TRACK_CHANNEL), aciduino.getTrackOutputParam(TRACK_PORT), NOTE_VELOCITY_808);
            aciduino.sendNote(data, aciduino.getTrackOutputParam(TRACK_CHANNEL), aciduino.getTrackOutputParam(TRACK_PORT), 0);
          // no shift select voice
          } else {
            // select voice
            data = parseData(data, 0, VOICE_MAX_SIZE_808-1, aciduino.seq.getTrackVoice(aciduino.getSelectedTrack()));
            aciduino.seq.setTrackVoice(aciduino.getSelectedTrack(), data);
          }

        }
        
      }
      
    }

    void changeRelease(int16_t data) {
      if(selected_line >= 2) {
        // preview note release for note off
        if (data == DECREMENT) {
          aciduino.seq.input(rec_track, NOTE_OFF, rec_note, aciduino.seq.is303(rec_track) ? NOTE_VELOCITY_303 : NOTE_VELOCITY_808);
        }
      }
    }
    
    void function1() {
      if (selected_line == 1) {
        // clear track
        aciduino.seq.clearTrack(aciduino.getSelectedTrack());
      } else if (selected_line >= 2) {
        // 303 and 808 uses the same accent button f1
        aciduino.seq.setAccent(aciduino.getSelectedTrack(), selected_step, !aciduino.seq.accentOn(aciduino.getSelectedTrack(), selected_step));
      }
    }

    void function2() {
      if (selected_line == 1) {
        // mute track(voice for 808)
        if (aciduino.seq.is303(aciduino.getSelectedTrack())) {
          aciduino.seq.setMute(aciduino.getSelectedTrack(), !aciduino.seq.getMute(aciduino.getSelectedTrack()));
          // keep mutePatternComponent updated for current mute state
          mutePatternComponent.updateCurrentMuteState(aciduino.getSelectedTrack());
        } else {
          aciduino.seq.setMute(aciduino.getSelectedTrack(), aciduino.seq.getTrackVoice(aciduino.getSelectedTrack()), !aciduino.seq.getMute(aciduino.getSelectedTrack(), aciduino.seq.getTrackVoice(aciduino.getSelectedTrack())));
          mutePatternComponent.updateCurrentMuteState(aciduino.getSelectedTrack(), aciduino.seq.getTrackVoice(aciduino.getSelectedTrack()));
        }
      } else if (selected_line >= 2) {
        // 303
        if (aciduino.seq.is303(aciduino.getSelectedTrack())) {
          if ((aciduino.seq.stepOn(aciduino.getSelectedTrack(), selected_step-1) || aciduino.seq.tieOn(aciduino.getSelectedTrack(), selected_step-1)) && !aciduino.seq.stepOn(aciduino.getSelectedTrack(), selected_step) && selected_step != 0) {
            aciduino.seq.setTie(aciduino.getSelectedTrack(), selected_step, !aciduino.seq.tieOn(aciduino.getSelectedTrack(), selected_step));
          } else {
            aciduino.seq.setSlide(aciduino.getSelectedTrack(), selected_step, !aciduino.seq.slideOn(aciduino.getSelectedTrack(), selected_step));
          }
        // 808
        } else {
          aciduino.seq.setRoll(aciduino.getSelectedTrack(), selected_step, !aciduino.seq.rollOn(aciduino.getSelectedTrack(), selected_step));
        }
      }
    }

    void selectorButton(uint8_t nunmber) {
      selected_locator = nunmber;
    }

    void stepButton(uint8_t number) {
      uint8_t step = number + ((full_size_view && selected_line != 1 ? selected_line-2 : selected_locator) * 16);
      aciduino.seq.rest(aciduino.getSelectedTrack(), step, aciduino.seq.stepOn(aciduino.getSelectedTrack(), step));
    }
    
} stepSequencerComponent;
