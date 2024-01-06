uint8_t _select_page_menu = 1;
bool _powersave = false;

void system_page_init()
{ 
  //uCtrl.page->set("syst", system_page_create, system_page_destroy, system_page_refresh, system_page_digital_input, system_page_analog_input, 2); 
  uCtrl.page->set("syst", system_page_create, system_page_destroy, system_page_refresh, system_page_digital_input, nullptr, 2);    
}

void system_page_create()
{
  uCtrl.dout->writeAll(LOW);
}

void system_page_destroy()
{
  uCtrl.dout->writeAll(LOW);
}

// GUI Components
struct TempoTransport : PageComponent {

    void view() {
      genericOptionView("tempo", String(uClock.getTempo(), 1), line, col, selected);

      setF1("rec", AcidSequencer.getRecStatus());
      setF2("play", _playing ? true : false);
    }

    void change(int16_t data) {
      // primary incrementer or decrementer
      if (data == DECREMENT || data == INCREMENT) {
        // inc and dec will fine update to 0.1 bpm
        uClock.setTempo(uClock.getTempo()+((data == DECREMENT ? -1 : 1) * 0.1));
      // secondary inc/dec or pot nav action
      } else {
        data = parseData(data, 30, 180, (uint16_t)uClock.getTempo());
        uClock.setTempo(data);
      }
    }

    void function1() {
      // rec mode
      AcidSequencer.setRecStatus(!AcidSequencer.getRecStatus());
    }

    void function2() {
      // play/stop
      if (_playing)
        uClock.stop();
      else
        uClock.start();
    }

} tempoTransportComponent;

struct TempoClockSource : PageComponent {

    void view() {
      genericOptionView("clock", _midi_clock_port == 0 ? String("int") : String(String("midi") + _midi_clock_port), line, col, selected);
    }

    void change(int16_t data) {
      data = parseData(data, 0, uCtrl.midi->sizeOf(), _midi_clock_port);
      ATOMIC(_midi_clock_port = data)
      if (_midi_clock_port > 0) {
        uClock.setMode(uClock.EXTERNAL_CLOCK);
      } else {
        uClock.setMode(uClock.INTERNAL_CLOCK);
      }
    }
    
} tempoClockSourceComponent;

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

struct RecInputSource : PageComponent {
    
    void view() {
      genericOptionView("rec", String(String("midi") + _midi_rec_port), line, col, selected);
    }

    void change(int16_t data) {
      data = parseData(data, 1, uCtrl.midi->sizeOf(), _midi_rec_port);
      ATOMIC(_midi_rec_port = data)
    }

} recInputComponent;

struct RecMode : PageComponent {

    uint8_t rec_mode = 0;

    void view() {
      genericOptionView("mode", rec_mode == 0 ? String("real") : String("step"), line, col, selected);
    }

    void change(int16_t data) {
      data = parseData(data, 0, 1, rec_mode);
      rec_mode = data;
    }

} recModeComponent;

struct ShuffleControl : PageComponent {

  void view() {
    genericOptionView("shuffle", uClock.isShuffled() ? String("on") : String("off"), line, col, selected);
  }

  void change(int16_t data) {
    data = parseData(data, 0, 1, uClock.isShuffled());
    uClock.setShuffle(data);
  }

} shuffleControlComponent;

struct ShuffleSignature : PageComponent {

  // MPC60 groove signatures?
  uint8_t current_shuffle = 0;
  uint8_t template_size = 2;
  int8_t shuffle_50[2] = {0, 0};
  int8_t shuffle_54[2] = {0, 2};
  int8_t shuffle_58[2] = {0, 4};
  int8_t shuffle_62[2] = {0, 6};
  int8_t shuffle_66[2] = {0, 8};
  int8_t shuffle_71[2] = {0, 10};
  int8_t shuffle_75[2] = {0, 12};

  String shuffle_name[7] = {"50%", "54%", "58%", "62%", "66%", "71%", "75%"};
  
  int8_t* shuffle_templates[7] = {shuffle_50, shuffle_54, shuffle_58, shuffle_62, shuffle_66, shuffle_71, shuffle_75};

  void view() {
    genericOptionView("sig.", shuffle_name[current_shuffle], line, col, selected);
  }

  void change(int16_t data) {
    current_shuffle = parseData(data, 0, 6, current_shuffle);
    // uClock shuffle
    uClock.setShuffleTemplate(shuffle_templates[current_shuffle], template_size);
  }

} shuffleSignatureComponent;

// called each cycle interaction of interface object for GUI refresh
void system_page_refresh(uint8_t subpage)
{ 
  // https://javl.github.io/image2cpp/
  //uCtrl.oled->display->drawXBMP(0, 20, 124, 36, logo_bpm);

  uCtrl.page->component(topBarComponent, 1, 1);

  if (subpage == 0) {

    // tempo setup    
    uCtrl.page->component(tempoTransportComponent, 3, 1, true);
    uCtrl.page->component(tempoClockSourceComponent, 4, 1);
    
    // rec setup
    uCtrl.page->component(recInputComponent, 3, 2);
    uCtrl.page->component(recModeComponent, 4, 2);

    // shuffle setup
    uCtrl.page->component(shuffleControlComponent, 5, 1);
    uCtrl.page->component(shuffleSignatureComponent, 5, 2);
    
    // session setup
    uCtrl.page->component(sessionConfigComponent, 6, 1);
    
  } else if (subpage == 1) {

    uCtrl.oled->print("track config", 3, 1);
    uCtrl.page->component(trackOutputSelectorComponent, 4, 1, true);
    uCtrl.page->component(midiChannelConfigComponent, 4, 2);
    
    uCtrl.oled->print("system resources", 6, 1);
    uCtrl.page->component(systemResourcesComponent, 7, 1);
    
  }
}

void system_page_digital_input(uint8_t control_id, uint16_t value, uint8_t subpage)
{ 
  // direct access to digital inputs(DIN and TOUCH)
  switch (control_id) {
    
  }   
}

void system_page_analog_input(uint8_t control_id, uint16_t value, uint8_t subpage)
{
  // direct access to analog inputs
}
