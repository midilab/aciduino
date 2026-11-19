// generic controler for 303 and 808 devices
// pot_map is indexed by absolute track slot(dynamic layout)
typedef struct
{
    const char * control_name;
    uint8_t control_cc = 0;
    int8_t pot_map[SEQ_MAX_TRACKS] = {-1};
} MIDI_CTRL_DATA_303;

typedef struct
{
    const char * control_name;
    uint8_t control_cc = 0;
    int8_t pot_map[SEQ_MAX_TRACKS] = {-1};
} MIDI_CTRL_DATA_808;

typedef struct
{
    const char * control_name;
    uint8_t control_cc = 0;
    int8_t pot_map[SEQ_MAX_TRACKS] = {-1};
} MIDI_CTRL_DATA_POLY;

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

    MIDI_CTRL_DATA_POLY control_map_poly[16];
    uint8_t ctrl_size_poly = 0;

    // control value per absolute track slot: shared by the three type maps
    uint8_t control_data[16][SEQ_MAX_TRACKS];
    
    MidiCCControl()
    {
      // we want this component to be 4 lines max and 2 grids navigable object
      line_size = 4;
      grid_size = 2;
      // enable more complex nav by updating selector pointer of page component
      update_selector_view = true;
      // initing pot_map for learn refrence
      for (uint8_t i=0; i < 16; i++) {
        for (uint8_t j=0; j < SEQ_MAX_TRACKS; j++) {
          control_map_303[i].pot_map[j] = -1;
          control_map_808[i].pot_map[j] = -1;
          control_map_poly[i].pot_map[j] = -1;
          control_data[i][j] = 0;
        }
      }
    }

    // pot learn map of a track/control pair(absolute slot, dynamic layout)
    int8_t & potMap(uint8_t track, uint8_t ctrl) {
      if (aciduino.seq.is303(track)) return control_map_303[ctrl].pot_map[track];
      if (aciduino.seq.is808(track)) return control_map_808[ctrl].pot_map[track];
      return control_map_poly[ctrl].pot_map[track];
    }

    void view() {
      uint8_t data_idx = aciduino.getSelectedTrack();
      uint8_t ctrl_map_init = selected_map == 0 ? 0 : 8;
      uint8_t ctrl_counter = 0;
      ctrl_selected = selected_line-1 + (selected_grid == 2 ? 4 : 0);
      ctrl_selected += selected_map > 0 ? 8 : 0;
      last_selected_track = aciduino.getSelectedTrack();

      // print controls
      for (uint8_t i=ctrl_map_init; i < ctrl_map_init+8; i++) {
        // process 303 controlelrs?
        if (aciduino.seq.is303(aciduino.getSelectedTrack())) {
          if (i >= ctrl_size_303) {
            // breaks with no selected element? fix it!
            if (ctrl_selected >= ctrl_size_303) {
              selected_grid = floor((selected_map > 0 ? ctrl_size_303-1-8 : ctrl_size_303-1)/4)+1;
              selected_line = ((selected_map > 0 ? ctrl_size_303-1-8 : ctrl_size_303-1)%4)+1;
            }
            break;
          }
          genericOptionView(control_map_303[i].control_name, String(control_data[i][data_idx]), line+(i%4), ctrl_counter >= 4 ? 14 : 1, i==ctrl_selected);
        // process 808 controlelrs?
        } else if (aciduino.seq.is808(aciduino.getSelectedTrack())) {
          if (i >= ctrl_size_808) {
            // breaks with no selected element? fix it!
            if (ctrl_selected >= ctrl_size_808) {
              selected_grid = floor((selected_map > 0 ? ctrl_size_808-1-8 : ctrl_size_808-1)/4)+1;
              selected_line = ((selected_map > 0 ? ctrl_size_808-1-8 : ctrl_size_808-1)%4)+1;
            }
            break;
          }
          genericOptionView(control_map_808[i].control_name, String(control_data[i][data_idx]), line+(i%4), ctrl_counter >= 4 ? 14 : 1, i==ctrl_selected);
        // process poly controlelrs?
        } else {
          if (i >= ctrl_size_poly) {
            // breaks with no selected element? fix it!
            if (ctrl_selected >= ctrl_size_poly) {
              selected_grid = floor((selected_map > 0 ? ctrl_size_poly-1-8 : ctrl_size_poly-1)/4)+1;
              selected_line = ((selected_map > 0 ? ctrl_size_poly-1-8 : ctrl_size_poly-1)%4)+1;
            }
            break;
          }
          genericOptionView(control_map_poly[i].control_name, String(control_data[i][data_idx]), line+(i%4), ctrl_counter >= 4 ? 14 : 1, i==ctrl_selected);
        }
        ++ctrl_counter;
      }

      // shift only if learn enabled
      if ((uCtrl.page->isShiftPressed() || learn == true) && learn_enabled == true) {
        updateControlMap();
        int8_t pot_map = potMap(aciduino.getSelectedTrack(), ctrl_selected);
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
      sendCCData(data, ctrl_selected, aciduino.getSelectedTrack());
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
        int8_t pot_map = potMap(aciduino.getSelectedTrack(), ctrl_selected);
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
      // process 303 controlelrs?
      if (aciduino.seq.is303(track)) {
        data = parseData(data, 0, 127, control_data[ctrl][track]);
        control_data[ctrl][track] = data;
        // send data
        aciduino.sendMidiCC(control_map_303[ctrl].control_cc, data, aciduino.getTrackOutputParam(TRACK_CHANNEL, track), aciduino.getTrackOutputParam(TRACK_PORT, track), interrupted);
      // process 808 controlelrs?
      } else if (aciduino.seq.is808(track)) {
        data = parseData(data, 0, 127, control_data[ctrl][track]);
        control_data[ctrl][track] = data;
        // send data
        aciduino.sendMidiCC(control_map_808[ctrl].control_cc, data, aciduino.getTrackOutputParam(TRACK_CHANNEL, track), aciduino.getTrackOutputParam(TRACK_PORT, track), interrupted);
      // process poly controlelrs?
      } else {
        data = parseData(data, 0, 127, control_data[ctrl][track]);
        control_data[ctrl][track] = data;
        // send data
        aciduino.sendMidiCC(control_map_poly[ctrl].control_cc, data, aciduino.getTrackOutputParam(TRACK_CHANNEL, track), aciduino.getTrackOutputParam(TRACK_PORT, track), interrupted);
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
      for (uint8_t i=0; i < SEQ_MAX_TRACKS; i++) {
        control_data[ctrl_size_303][i] = initial_value;
      }
      ++ctrl_size_303;
    }

    void set808Control(const char * name, uint8_t cc, uint8_t initial_value = 0) {
      if (ctrl_size_808 >= 16)
        return;
      control_map_808[ctrl_size_808].control_name = name;
      control_map_808[ctrl_size_808].control_cc = cc;
      for (uint8_t i=0; i < SEQ_MAX_TRACKS; i++) {
        control_data[ctrl_size_808][i] = initial_value;
      }
      ++ctrl_size_808;
    }

    void setPolyControl(const char * name, uint8_t cc, uint8_t initial_value = 0) {
      if (ctrl_size_poly >= 16)
        return;
      control_map_poly[ctrl_size_poly].control_name = name;
      control_map_poly[ctrl_size_poly].control_cc = cc;
      for (uint8_t i=0; i < SEQ_MAX_TRACKS; i++) {
        control_data[ctrl_size_poly][i] = initial_value;
      }
      ++ctrl_size_poly;
    }

    void learnCtrl(uint8_t port)
    {
      if (learn == true) {
          aciduino.setMidiControlParam(MIDI_CTRL, ctrl_selected, port);
          aciduino.setMidiControlParam(MIDI_TRACK, aciduino.getSelectedTrack(), port);
          potMap(aciduino.getSelectedTrack(), ctrl_selected) = port;
          learn = false;
      }
    }

    void clearCtrl(uint8_t port)
    {
      int8_t &map = potMap(aciduino.getSelectedTrack(), port);
      int8_t idx = map;
      aciduino.setMidiControlParam(MIDI_CTRL, -1, idx);
      map = -1;
    }

    void updateControlMap() {
      // update track control reference for pot map global
      for (uint8_t i=0; i < 16; i++) {
        int8_t ctrl = aciduino.getMidiControlParam(MIDI_CTRL, i);
        if (ctrl != -1) {
          uint8_t track = aciduino.getMidiControlParam(MIDI_TRACK, i);
          if (track < SEQ_MAX_TRACKS && aciduino.seq.trackType(track) != SEQ_TRACK_NONE) {
            potMap(track, ctrl) = i;
          }
        }
      }
    }
    
} midiControllerComponent;

// used by ain in case any POT_X registred
void midiControllerHandle(uint8_t port, uint16_t value) {

#if defined(USE_CHANGER_POT)
  // let pot changer out of this controller
  if (port == 0)
    return;
  --port;
#endif

  // check for learn request
  midiControllerComponent.learnCtrl(port);
  
  // anything into global learn map table?
  int8_t ctrl = aciduino.getMidiControlParam(MIDI_CTRL, port);
  uint8_t track = aciduino.getMidiControlParam(MIDI_TRACK, port);

  if (ctrl != -1) {
    midiControllerComponent.sendCCData((int16_t)value, ctrl, track, 0);
  } else {
    midiControllerComponent.sendCCData((int16_t)value, port, aciduino.getSelectedTrack(), 0);
  }
}
