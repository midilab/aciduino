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

void midi_page_init()
{
  //uCtrl.page->set("midi", midi_page_create, midi_page_destroy, midi_page_refresh, midi_page_digital_input, midi_page_analog_input, 2);   
  uCtrl.page->set("midi", midi_page_create, midi_page_destroy, midi_page_refresh, midi_page_digital_input, nullptr, 1);       

  // register midi controller options per track
  if (TRACK_NUMBER_303 > 0) {
    // max 16 controls!
    midiControllerComponent.set303Control("filter", 74);
    midiControllerComponent.set303Control("reso.", 71);
    midiControllerComponent.set303Control("env mod", 12);
    midiControllerComponent.set303Control("decay", 75);
    midiControllerComponent.set303Control("accent", 16);
    midiControllerComponent.set303Control("vcf bd.", 1);
    midiControllerComponent.set303Control("drive", 17);
    midiControllerComponent.set303Control("volume", 11);

    midiControllerComponent.set303Control("delay t.", 18);
    midiControllerComponent.set303Control("delay l.", 19);
    midiControllerComponent.set303Control("tuning", 104);
    midiControllerComponent.set303Control("efx 1", 105);
    midiControllerComponent.set303Control("efx 2", 106);
    midiControllerComponent.set303Control("efx 3", 107);
    midiControllerComponent.set303Control("efx 4", 108);
    midiControllerComponent.set303Control("efx 5", 109);
  }
  if (TRACK_NUMBER_808 > 0) {
    // max 16 controls!
    midiControllerComponent.set808Control("bd decay", 23);
    midiControllerComponent.set808Control("bd tone", 21);
    midiControllerComponent.set808Control("sd snap", 26);
    midiControllerComponent.set808Control("sd tone", 25);
    midiControllerComponent.set808Control("lt tune", 46);
    midiControllerComponent.set808Control("mt tune", 49);
    midiControllerComponent.set808Control("ht tune", 52);
    midiControllerComponent.set808Control("rs tune", 55);

    midiControllerComponent.set808Control("bd level", 24);
    midiControllerComponent.set808Control("sd level", 29);
    midiControllerComponent.set808Control("ch level", 63);
    midiControllerComponent.set808Control("oh level", 82);
    midiControllerComponent.set808Control("lt level", 48);
    midiControllerComponent.set808Control("mt level", 51);
    midiControllerComponent.set808Control("ht level", 54);
    midiControllerComponent.set808Control("rs level", 57);
  }  
}

void midi_page_create()
{
  // lets hook the components f1 anf f2 functions for generative page only
  //
  //uCtrl.page->setFunctionHook(f1_page, f2_page, generativeFunction1, generativeFunction2);
}

void midi_page_destroy()
{
  // clear our hook
  //uCtrl.page->clearFunctionHook();
}

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

// called each cycle interaction of interface object for UI refresh
void midi_page_refresh(uint8_t subpage)
{
  uCtrl.page->component(topBarComponent, 1, 1);

  if (subpage == 0) {
    uCtrl.page->component(midiControllerComponent, 3, 1, true);
  } else if (subpage == 1) {
    // midi efx subpage. LFOs and other goodies. soon...
  }
}

void midi_page_digital_input(uint8_t control_id, uint16_t value, uint8_t subpage)
{  
  // step array buttons handling?
}

void midi_page_analog_input(uint8_t control_id, uint16_t value, uint8_t subpage)
{
  // special fixed pots for interaction like generative ones
}
