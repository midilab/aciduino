#include <stddef.h>

#include "aciduino.hpp"

//
// setup and runtime
//
void Aciduino::init() {
  // setup clock system
  uClockSetup();
  // init the sequencer
  initSequencer();

  // force initializatiron of mute pattern data
  for (uint8_t i=0; i < 4; i++) {
    _mute_grid[i].map = 0xFFFF;
    for (uint8_t j=0; j < SEQ_MAX_TRACKS; j++) {
      _mute_grid[i].map_parts[j] = 0xFFFF;
    }
  }
}

void Aciduino::run() {
  // let uCtrl do his job
  uCtrl.run();
}

//
// Sequencer initialization
//
void Aciduino::initSequencer()
{
  // storage setup epprom/sdcard
  storageSetup();

  // init default track output data
  for(uint8_t track=0; track < SEQ_MAX_TRACKS; track++) {
    _track_output_setup[track].output = MIDI_OUTPUT;
    _track_output_setup[track].channel = track;
    _track_output_setup[track].port = 0;
  }

  // the acid sequencer main output callback
  aciduino.seq.setOutputCallback(sequencerOutHandler);

  // bring up storage: sdcard session select/import/factory init
  // or epprom layout check/factory init when running on epprom
  storageInit();

  // load shuffle form memory pattern?
  int8_t init_template[2] = {0,0};
  uClock.setShuffleTemplate(init_template, 2);

  // load saved session
  // on sdcard this also restores the pattern grid and loads each session pattern
  loadSession();
  // no session pattern grid on epprom: load first pattern
  if (!uCtrl.storage->isSdActive()) {
    loadPattern(0);
  }
  // old session without a layout record: adopt the layout that came from the
  // loaded patterns and persist it once
  if (!_layout_valid) {
    _layout_reset_mask = 0;
    saveSession();
  }
}

// storage bring up: pick the backend, validate data layout and import/init as needed
void Aciduino::storageInit()
{
#if defined(USE_SDCARD_MODULE) && defined(USE_PAGE_MODULE)
  // card is physically there but its filesystem did not mount: ask the user
  // if they want it formatted before falling back to epprom mode
  if (uCtrl.sdcard != nullptr && uCtrl.sdcard->needsFormat()) {
    if (formatSdCardDialog()) {
      // formatted and remounted: boot proceeds into the sdcard flow below
    }
  }
#endif
  if (uCtrl.storage->isSdActive()) {
    // restore last used session(create session file if missing)
    uCtrl.storage->restoreSession();
    // data layout check runs against the active session image
    if (checkEppromDataLayoutChange()) {
      if (uCtrl.storage->hasEppromData(_pattern_total_mem_size)) {
        // epprom data avaliable: migrate it into the sdcard session image
        // (stop before the layout tail: it is not pattern data)
        uCtrl.storage->importFromEpprom(_epprom_track_layout_address);
      } else {
        // fresh card: factory session image from current defaults
        uint16_t pattern_size = _pattern_total_mem_size;
        uCtrl.storage->save((void*)&pattern_size, sizeof(pattern_size), _epprom_check_data_address);
        saveSession();
        savePattern(0);
      }
    }
    return;
  }

  // we're going to init epprom in two cases:
  // if track layout changes || if no aciduino data present at epprom
  // so if you want to keep your pattenrs and change layout consider backup pattenrs into another sequencer
  // because they will be wipeout for the new memory layout to take place
  if (checkEppromDataLayoutChange()) {
    eppromInit();
  }
}

#if defined(USE_SDCARD_MODULE) && defined(USE_PAGE_MODULE) && (defined(USE_DIN_MODULE) || defined(USE_TOUCH_MODULE))
// blocking boot dialog: ask the user if the unmounted sdcard should be formatted
// returns true when the card got formatted and remounted
bool Aciduino::formatSdCardDialog()
{
  bool format_card = false;
  bool answered = false;

  uCtrl.oled->clearDisplay();
  uCtrl.oled->print("sdcard", 1, 1);
  uCtrl.oled->print("needs format!", 3, 2);
  uCtrl.oled->print("f1 yes", 8, 1);
  uCtrl.oled->print("f2 no", 8, 15);
  uCtrl.oled->refreshDisplay();

  // drain the interrupt-driven event queues exactly like uCtrlClass::run() does
  while (!answered) {
#if defined(USE_DIN_MODULE)
    if (uCtrl.din != nullptr) {
      while (uCtrl.din->event_queue.head != uCtrl.din->event_queue.tail) {
        uint8_t port = uCtrl.din->event_queue.event[uCtrl.din->event_queue.head].port;
        uint16_t value = uCtrl.din->event_queue.event[uCtrl.din->event_queue.head].value;
        uCtrl.din->event_queue.head = (uCtrl.din->event_queue.head+1) % (uCtrl.din->event_queue.size);

        // din event value 1 = pressed
        if (value != 0) {
          if (port == uCtrl.page->_nav_ctrl_port.function1) {
            format_card = true;
            answered = true;
          } else if (port == uCtrl.page->_nav_ctrl_port.function2) {
            answered = true;
          }
        }
      }
    }
#endif
#if defined(USE_TOUCH_MODULE)
    if (uCtrl.touch != nullptr) {
      uint8_t port_ref = 0;
#if defined(USE_DIN_MODULE)
      if (uCtrl.din != nullptr) {
        port_ref = uCtrl.din->sizeOf();
      }
#endif
      while (uCtrl.touch->event_queue.head != uCtrl.touch->event_queue.tail) {
        uint8_t port = uCtrl.touch->event_queue.event[uCtrl.touch->event_queue.head].port + port_ref;
        uint16_t value = uCtrl.touch->event_queue.event[uCtrl.touch->event_queue.head].value;
        uCtrl.touch->event_queue.head = (uCtrl.touch->event_queue.head+1) % (uCtrl.touch->event_queue.size);

        // touch event value 1 = pressed
        if (value != 0) {
          if (port == uCtrl.page->_nav_ctrl_port.function1) {
            format_card = true;
            answered = true;
          } else if (port == uCtrl.page->_nav_ctrl_port.function2) {
            answered = true;
          }
        }
      }
    }
#endif
    delay(1);
  }

  if (!format_card) {
    return false;
  }

  uCtrl.oled->clearDisplay();
  uCtrl.oled->print("formatting...", 4, 2);
  uCtrl.oled->refreshDisplay();

  if (!uCtrl.sdcard->formatCard()) {
    uCtrl.oled->clearDisplay();
    uCtrl.oled->print("format failed", 4, 2);
    uCtrl.oled->refreshDisplay();
    delay(1500);
    return false;
  }

  uCtrl.oled->clearDisplay();
  uCtrl.oled->print("format done", 4, 2);
  uCtrl.oled->refreshDisplay();
  delay(500);
  return true;
}
#endif

void Aciduino::uClockSetup()
{
  // set main clock rate for output(sequencer resolution) and input(receiving sync signals)
  uClock.setOutputPPQN(uClock.PPQN_96);
  uClock.setInputPPQN(uClock.PPQN_24);
  // set the callback function for the step sequencer on 96PPQN and for step sequencer feature
  uClock.setOnOutputPPQN(Aciduino::onPPQNCallback);
  // set the callback function to send MIDI Sync message.
  uClock.setOnSync(uClock.PPQN_24, Aciduino::onSync24Callback);
  // support for step sequencer
  uClock.setOnStep(Aciduino::onStepCallback);
  // set the callback function for MIDI Start and Stop messages.
  uClock.setOnClockStart(Aciduino::onClockStart);
  uClock.setOnClockStop(Aciduino::onClockStop);

  // Set the clock BPM to 126 BPM
  uClock.setTempo(126);
  //uClock.setClockMode(uClock.EXTERNAL_CLOCK);

  uClock.setExtIntervalBuffer(32);

  // Inits the clock
  uClock.init();
}

void Aciduino::storageSetup()
{
  //
  // initiate epprom memory layout setup
  //
#if defined(ARDUINO_ARCH_ESP32) || defined(ESP32)
  _epprom_size = 4096;
#else
  _epprom_size = EEPROM.length();
#endif

  _epprom_check_data_address = 0;
  _epprom_session_address = 2;
  _epprom_session_size =(sizeof(_generative_303) + sizeof(_generative_808) + sizeof(_control_map_global) + sizeof(_track_output_setup));

  //
  // common pattern data definitions
  //
  // one pattern = the whole sequencer image + the mute grid
  _pattern_image_size = aciduino.seq.getPatternImageSize();
  _pattern_total_mem_size = (_pattern_image_size + sizeof(_mute_grid));
  _eprrom_pattern_address = (_epprom_session_address + _epprom_session_size);
  _eprrom_pattern_available = (_epprom_size-_epprom_session_size-_epprom_session_address) / (_pattern_total_mem_size);

  // the session track layout record lives in the free tail after the last
  // pattern: no pattern address moves, so existing patterns stay valid
  if (_epprom_size > sizeof(TRACK_LAYOUT)) {
    _epprom_track_layout_address = _epprom_size - sizeof(TRACK_LAYOUT);
  } else {
    _epprom_track_layout_address = 0;
  }
  while (_eprrom_pattern_available > 0 &&
         ((uint32_t)_eprrom_pattern_address + (uint32_t)_eprrom_pattern_available * _pattern_total_mem_size) > _epprom_track_layout_address) {
    _eprrom_pattern_available--;
  }
}

uint16_t Aciduino::getNumOfPatterns() {
  // sdcard patterns are only bounded by the session image file growth
  if (uCtrl.storage->isSdActive()) {
    uint32_t image_size = uCtrl.storage->size();
    if (image_size <= _eprrom_pattern_address) {
      return 0;
    }
    uint16_t count = (image_size - _eprrom_pattern_address) / _pattern_total_mem_size;
    // pattern index is uint8_t across the code: cap the space at 255 saved patterns
    return count > 255 ? 255 : count;
  }
  return _eprrom_pattern_available;
}

bool Aciduino::hasPendingPatternSlot() {
  // only sdcard storage grows: epprom pattern space is fixed
  return uCtrl.storage->isSdActive();
}

uint16_t Aciduino::getSessionSize() {
  return _epprom_session_size;
}

uint16_t Aciduino::getStorageSize() {
  if (uCtrl.storage->isSdActive()) {
    return uCtrl.storage->size();
  }
  return _epprom_size;
}

//
// sdcard sessions
//
uint16_t Aciduino::getNumOfSessions() {
  return uCtrl.storage->getSessionCount();
}

uint16_t Aciduino::getActiveSession() {
  return uCtrl.storage->getActiveSession();
}

bool Aciduino::selectSession(uint16_t index) {
  if (!uCtrl.storage->useSession(index)) {
    return false;
  }
  // session slot has data for our current layout?
  if (checkEppromDataLayoutChange()) {
    return false;
  }
  // switched sessions: unsaved edits are dropped, as before
  _layout_reset_mask = 0;
  loadSession();
  return true;
}

// fork current state into a new session slot: settings copied over, patterns start empty
bool Aciduino::newSession() {
  uint16_t index = uCtrl.storage->getSessionCount();
  if (!uCtrl.storage->useSession(index, true)) {
    return false;
  }
  // mark image layout as valid for this firmware
  uint16_t pattern_size = _pattern_total_mem_size;
  uCtrl.storage->save((void*)&pattern_size, sizeof(pattern_size), _epprom_check_data_address);
  // reset pattern grid: no patterns saved on a new session yet
  for (uint8_t i = 0; i < SEQ_MAX_TRACKS; i++) {
    _pattern_grid[i] = 0;
  }
  // fork session settings from current state
  saveSession();
  // factory pattern state for all tracks(pending slot 0)
  loadPattern(0);
  return true;
}

// shuffle signature accessors(session data driven state)
uint8_t Aciduino::getShuffleSig() {
  return _shuffle_sig;
}

void Aciduino::setShuffleSig(uint8_t sig, int8_t * shuff) {
  if (sig > 6) {
    sig = 0;
  }
  _shuffle_sig = sig;
  _shuffle_template[0] = shuff[0];
  _shuffle_template[1] = shuff[1];
  uClock.setShuffleTemplate(_shuffle_template, 2);
}

//
// Main interface
//
void Aciduino::playStop()
{
  if (aciduino.isPlaying())
    aciduino.stop();
  else
    aciduino.play();
}

void Aciduino::play()
{
  uClock.start();
}

void Aciduino::stop()
{
  uClock.stop();
}

void Aciduino::pause()
{
  uClock.pause();
}

void Aciduino::recToggle()
{
  aciduino.seq.setRecStatus(!aciduino.seq.getRecStatus());
}

void Aciduino::previousTrack()
{
  // walk slots backwards and skip empty ones(dynamic layout leaves holes)
  uint8_t t = aciduino._selected_track;
  for (uint8_t i=0; i < SEQ_MAX_TRACKS; i++) {
    t = (t == 0) ? SEQ_MAX_TRACKS - 1 : t - 1;
    if (aciduino.seq.trackType(t) != SEQ_TRACK_NONE) {
      aciduino._selected_track = t;
      return;
    }
  }
}

void Aciduino::nextTrack()
{
  // walk slots forwards and skip empty ones(dynamic layout leaves holes)
  uint8_t t = aciduino._selected_track;
  for (uint8_t i=0; i < SEQ_MAX_TRACKS; i++) {
    t = (t == SEQ_MAX_TRACKS - 1) ? 0 : t + 1;
    if (aciduino.seq.trackType(t) != SEQ_TRACK_NONE) {
      aciduino._selected_track = t;
      return;
    }
  }
}

//
// Pattern Grids
//
// MUTE GRID
uint8_t Aciduino::getMuteGridState(uint8_t mute_pattern, uint8_t track, uint8_t voice)
{
  if (track >= SEQ_MAX_TRACKS) {
    return 1;
  }
  // track level(303/poly)
  if (!aciduino.seq.is808(track)) {
    return GET_BIT(_mute_grid[mute_pattern].map, track);
  }
  // part level(808)
  return GET_BIT(_mute_grid[mute_pattern].map_parts[track], voice);
}

void Aciduino::setMuteGridState(uint8_t mute_pattern, uint8_t state, uint8_t track, uint8_t voice)
{
  if (track >= SEQ_MAX_TRACKS) {
    return;
  }
  if (state == 0) {
    if (!aciduino.seq.is808(track)) {
      CLR_BIT(_mute_grid[mute_pattern].map, track);
    } else {
      CLR_BIT(_mute_grid[mute_pattern].map_parts[track], voice);
    }
  } else if (state == 1) {
    if (!aciduino.seq.is808(track)) {
      SET_BIT(_mute_grid[mute_pattern].map, track);
    } else {
      SET_BIT(_mute_grid[mute_pattern].map_parts[track], voice);
    }
  }
}
// PATTERN GRID
uint8_t Aciduino::getPatternGrid(uint8_t track)
{
  return _pattern_grid[track];
}

void Aciduino::setPatternGrid(uint8_t track, uint8_t pattern)
{
  _pattern_grid[track] = pattern;
}

//
// Generative
//
void Aciduino::generatePattern(int8_t track)
{
  uint8_t track_gen = track;
  if (track == -1)
    track_gen = _selected_track;

  uint8_t type = aciduino.seq.trackType(track_gen);
  if (type == SEQ_TRACK_303)
    aciduino.seq.acidRandomize(track_gen, _generative_303[track_gen].generative_fill, _generative_303[track_gen].accent_probability, _generative_303[track_gen].slide_probability, _generative_303[track_gen].tie_probability, _generative_303[track_gen].number_of_tones, _generative_303[track_gen].lower_octave*12, _generative_303[track_gen].range_octave*12);
  else if (type == SEQ_TRACK_808)
    aciduino.seq.acidRandomize(track_gen, _generative_808[track_gen].generative_fill, _generative_808[track_gen].accent_probability, _generative_808[track_gen].roll_probability);
}

uint8_t Aciduino::getGenerativeParam(uint8_t param, int8_t track)
{
  uint8_t track_change = track;
  uint8_t value = 0;
  if (track == -1)
    track_change = _selected_track;

  if (track_change >= SEQ_MAX_TRACKS) {
    return 0;
  }
  uint8_t type = seq.trackType(track_change);
  switch (param) {
    case GENERATIVE_FILL:
      if (type == SEQ_TRACK_303)
        value = _generative_303[track_change].generative_fill;
      else if (type == SEQ_TRACK_808)
        value = _generative_808[track_change].generative_fill;
      break;
    case GENERATIVE_ACCENT:
      if (type == SEQ_TRACK_303)
        value = _generative_303[track_change].accent_probability;
      else if (type == SEQ_TRACK_808)
        value = _generative_808[track_change].accent_probability;
      break;
    case GENERATIVE_SLIDE:
      if (type == SEQ_TRACK_303)
        value = _generative_303[track_change].slide_probability;
      break;
    case GENERATIVE_TIE:
      if (type == SEQ_TRACK_303)
        value = _generative_303[track_change].tie_probability;
      break;
    case GENERATIVE_LOWER_OCTAVE:
      if (type == SEQ_TRACK_303)
        value = _generative_303[track_change].lower_octave;
      break;
    case GENERATIVE_RANGE_OCTAVE:
      if (type == SEQ_TRACK_303)
        value = _generative_303[track_change].range_octave;
      break;
    case GENERATIVE_NUM_TONES:
      if (type == SEQ_TRACK_303)
        value = _generative_303[track_change].number_of_tones;
      break;
    case GENERATIVE_ROLL:
      if (type == SEQ_TRACK_808)
        value = _generative_808[track_change].roll_probability;
      break;
    default:
      break;
  }

  return value;
}

void Aciduino::setGenerativeParam(uint8_t param, uint8_t data, int8_t track)
{
  uint8_t track_change = track;
  if (track == -1)
    track_change = _selected_track;

  if (track_change >= SEQ_MAX_TRACKS) {
    return;
  }
  uint8_t type = seq.trackType(track_change);
  switch (param) {
    case GENERATIVE_FILL:
      if (type == SEQ_TRACK_303)
        _generative_303[track_change].generative_fill = data;
      else if (type == SEQ_TRACK_808)
        _generative_808[track_change].generative_fill = data;
      break;
    case GENERATIVE_ACCENT:
      if (type == SEQ_TRACK_303)
        _generative_303[track_change].accent_probability = data;
      else if (type == SEQ_TRACK_808)
        _generative_808[track_change].accent_probability = data;
      break;
    case GENERATIVE_SLIDE:
      if (type == SEQ_TRACK_303)
        _generative_303[track_change].slide_probability = data;
      break;
    case GENERATIVE_TIE:
      if (type == SEQ_TRACK_303)
        _generative_303[track_change].tie_probability = data;
      break;
    case GENERATIVE_LOWER_OCTAVE:
      if (type == SEQ_TRACK_303)
        _generative_303[track_change].lower_octave = data;
      break;
    case GENERATIVE_RANGE_OCTAVE:
      if (type == SEQ_TRACK_303)
        _generative_303[track_change].range_octave = data;
      break;
    case GENERATIVE_NUM_TONES:
      if (type == SEQ_TRACK_303)
        _generative_303[track_change].number_of_tones = data;
      break;
    case GENERATIVE_ROLL:
      if (type == SEQ_TRACK_808)
        _generative_808[track_change].roll_probability = data;
      break;
    default:
      break;
  }
}

//
// Track output setup
//
uint8_t Aciduino::getTrackOutputParam(uint8_t param, int8_t track)
{
  uint8_t track_change = track;
  if (track == -1)
    track_change = _selected_track;

  switch (param) {
    case TRACK_OUTPUT:
      return _track_output_setup[track_change].output;
    case TRACK_CHANNEL:
      return _track_output_setup[track_change].channel;
    case TRACK_PORT:
      return _track_output_setup[track_change].port;
    default:
      break;
  }
  return 0;
}

void Aciduino::setTrackOutputParam(uint8_t param, uint8_t data, int8_t track)
{
  uint8_t track_change = track;
  if (track == -1)
    track_change = _selected_track;

  switch (param) {
    case TRACK_OUTPUT:
      ATOMIC(_track_output_setup[track_change].output = data)
      break;
    case TRACK_CHANNEL:
      ATOMIC(_track_output_setup[track_change].channel = data)
      break;
    case TRACK_PORT:
      ATOMIC(_track_output_setup[track_change].port = data)
      break;
    default:
      break;
  }
}

//
// midi controller data accessors
//
int8_t Aciduino::getMidiControlParam(uint8_t param, uint8_t idx)
{
  switch (param) {
    case MIDI_CTRL:
      return _control_map_global[idx].ctrl;
    case MIDI_TRACK:
      return (int8_t)_control_map_global[idx].track;
    default:
      break;
  }
  return 0;
}

void Aciduino::setMidiControlParam(uint8_t param, int8_t data, uint8_t idx)
{
  switch (param) {
    case MIDI_CTRL:
      _control_map_global[idx].ctrl = data;
      break;
    case MIDI_TRACK:
      _control_map_global[idx].track = (uint8_t)data;
      break;
    default:
      break;
  }
}

//
// Midi utils
//
// 3 realtime messages used by uClock object inside interruption
inline void Aciduino::sendMidiClock() {
  msg_interrupt.type = uctrl::protocol::midi::Clock;
  uCtrl.midi->writeAllPorts(&msg_interrupt, 1);
}

void Aciduino::sendMidiStart() {
  msg.type = uctrl::protocol::midi::Start;
  uCtrl.midi->writeAllPorts(&msg, 0);
}

void Aciduino::sendMidiStop() {
  msg.type = uctrl::protocol::midi::Stop;
  uCtrl.midi->writeAllPorts(&msg, 0);
}

// this is used inside interruption and outside, controlled by interrupted var
void Aciduino::sendMidiCC(uint8_t cc, uint8_t value, uint8_t channel, uint8_t port, uint8_t interrupted) {
  if (interrupted) {
    msg_interrupt_pots.type = uctrl::protocol::midi::ControlChange;
    msg_interrupt_pots.data1 = cc;
    msg_interrupt_pots.data2 = value;
    msg_interrupt_pots.channel = channel;
    uCtrl.midi->write(&msg_interrupt_pots, port+1, 1);
  } else {
    msg.type = uctrl::protocol::midi::ControlChange;
    msg.data1 = cc;
    msg.data2 = value;
    msg.channel = channel;
    uCtrl.midi->write(&msg, port+1, 0);
  }
}

void Aciduino::sendNote(uint8_t note, uint8_t channel, uint8_t port, uint8_t velocity) {
  msg.data1 = note;
  msg.data2 = velocity;
  msg.channel = channel;

  if (velocity == 0) {
    msg.type = uctrl::protocol::midi::NoteOff;
  } else {
    msg.type = uctrl::protocol::midi::NoteOn;
  }

  uCtrl.midi->write(&msg, port+1, 0);
}

//
// storage schema
//
void Aciduino::loadSession()
{
  // check if there is a session saved.. otherwise init one based on the defaults
  // we can use the last byte as version control
  // load session data
  uCtrl.storage->load((void*)_generative_303, sizeof(_generative_303), _epprom_session_address);
  uCtrl.storage->load((void*)_generative_808, sizeof(_generative_808));
  uCtrl.storage->load((void*)_control_map_global, sizeof(_control_map_global));
  uCtrl.storage->load((void*)_track_output_setup, sizeof(_track_output_setup));

  // sdcard sessions carry extras: tempo, shuffle, the pattern grid and the
  // track layout
  if (uCtrl.storage->isSdActive()) {
    if (uCtrl.storage->loadSessionConfig((void*)&_session_extras, sizeof(_session_extras))) {
      applySessionExtras();
    } else {
      // older sidecar without the layout tail: keep its known prefix
      memset((void*)&_session_extras.track_layout, 0, sizeof(TRACK_LAYOUT));
      if (uCtrl.storage->loadSessionConfig((void*)&_session_extras, offsetof(SESSION_EXTRAS, track_layout))) {
        applySessionExtras();
      }
    }
  } else {
    // epprom: the layout record lives in the free tail after the patterns
    uCtrl.storage->load((void*)&_session_extras.track_layout, sizeof(TRACK_LAYOUT), _epprom_track_layout_address);
  }

  _layout_valid = layoutRecordValid();
  if (_layout_valid) {
    aciduino.seq.applyTrackLayout(_session_extras.track_layout.tracks);
  }

  // patterns travel with the session: restore each track pattern from the grid
  if (uCtrl.storage->isSdActive()) {
    loadPattern(_pattern_grid[0]);
    for (uint8_t i = 1; i < SEQ_MAX_TRACKS; i++) {
      if (aciduino.seq.trackType(i) != SEQ_TRACK_NONE && _pattern_grid[i] != _pattern_grid[0]) {
        loadPattern(_pattern_grid[i], i);
      }
    }
  }

  fixSelectedTrack();
}

void Aciduino::applySessionExtras()
{
  uClock.setTempo(_session_extras.tempo);
  uClock.setShuffle(_session_extras.shuffle_on);
  setShuffleSig(_session_extras.shuffle_sig, _session_extras.shuffle_template);
  memcpy((void*)_pattern_grid, (void*)_session_extras.pattern_grid, sizeof(_pattern_grid));
}

// whole pattern load can bring a different track layout: never leave the UI
// pointing at an empty slot
void Aciduino::fixSelectedTrack()
{
  if (aciduino.seq.trackType(_selected_track) != SEQ_TRACK_NONE) {
    return;
  }
  for (uint8_t i=0; i < SEQ_MAX_TRACKS; i++) {
    if (aciduino.seq.trackType(i) != SEQ_TRACK_NONE) {
      _selected_track = i;
      return;
    }
  }
}

void Aciduino::saveSession()
{
  // the layout is captured first: on sdcard it travels with the session extras,
  // on epprom it goes to the tail record
  writeTrackLayout();

  // save session data
  uCtrl.storage->save((void*)_generative_303, sizeof(_generative_303), _epprom_session_address);
  uCtrl.storage->save((void*)_generative_808, sizeof(_generative_808));
  uCtrl.storage->save((void*)_control_map_global, sizeof(_control_map_global));
  uCtrl.storage->save((void*)_track_output_setup, sizeof(_track_output_setup));

  // a changed layout is propagated to every stored pattern so session and
  // pattern data never disagree
  if (_layout_reset_mask) {
    sanitizePatterns();
    _layout_reset_mask = 0;
  }
}

// capture the live track layout and persist it(session record)
void Aciduino::captureTrackLayout()
{
  aciduino.seq.copyTrackConfigs(_session_extras.track_layout.tracks);
  _session_extras.track_layout.magic = TRACK_LAYOUT_MAGIC ^ (uint16_t)sizeof(TRACK_LAYOUT);
  _session_extras.track_layout.version = TRACK_LAYOUT_VERSION;
  _layout_valid = true;
}

// slots whose live type differs from the last captured session layout
uint16_t Aciduino::layoutChanged()
{
  uint16_t mask = 0;
  if (!_layout_valid) {
    return mask;
  }
  for (uint8_t t = 0; t < SEQ_MAX_TRACKS; t++) {
    if (aciduino.seq.trackType(t) != _session_extras.track_layout.tracks[t].type) {
      mask |= (1u << t);
    }
  }
  return mask;
}

// keep the in-memory session layout current before a load/save uses it. This
// also catches a raw seq.createTrack/deleteTrack call that skipped the wrappers.
void Aciduino::syncTrackLayout()
{
  uint16_t changed = layoutChanged();
  if (changed) {
    _layout_reset_mask |= changed;
    captureTrackLayout();
  }
}

// a slot that was just created or deleted starts from clean session data
void Aciduino::resetTrackSessionData(uint8_t track)
{
  if (track >= SEQ_MAX_TRACKS) {
    return;
  }

  _generative_303[track] = GENERATIVE_303_DATA();
  _generative_808[track] = GENERATIVE_808_DATA();
  _track_output_setup[track].output = MIDI_OUTPUT;
  _track_output_setup[track].channel = track;
  _track_output_setup[track].port = 0;
  _pattern_grid[track] = 0;
  // new/deleted slots are muted on every pattern mute grid
  for (uint8_t mp = 0; mp < 4; mp++) {
    _mute_grid[mp].map |= (1 << track);
    _mute_grid[mp].map_parts[track] = 0xFFFF;
  }
  // global controls must not keep pointing at a slot that changed
  for (uint8_t i = 0; i < 16; i++) {
    if (_control_map_global[i].ctrl != -1 && _control_map_global[i].track == track) {
      _control_map_global[i].ctrl = -1;
    }
  }
}

uint8_t Aciduino::addTrack(uint8_t type)
{
  uint8_t slot = aciduino.seq.createTrack(type);
  if (slot == 0xFF) {
    return 0xFF;
  }

  resetTrackSessionData(slot);
  // the new list wins on every pattern load, even before a save
  captureTrackLayout();
  _layout_reset_mask |= (1u << slot);
  fixSelectedTrack();
  return slot;
}

bool Aciduino::deleteTrack(uint8_t track)
{
  if (!aciduino.seq.deleteTrack(track)) {
    return false;
  }

  resetTrackSessionData(track);
  // the new list wins on every pattern load, even before a save
  captureTrackLayout();
  _layout_reset_mask |= (1u << track);
  if (_copy_track == track) {
    _copy_track = -1;
  }

  if (track == _selected_track) {
    // UI neighbor: previous active slot; the next one only when the deleted
    // track was the first
    uint8_t selected = track;
    for (int8_t t = (int8_t)track - 1; t >= 0; t--) {
      if (aciduino.seq.trackType((uint8_t)t) != SEQ_TRACK_NONE) {
        selected = (uint8_t)t;
        break;
      }
    }
    if (selected == track) {
      for (uint8_t t = track + 1; t < SEQ_MAX_TRACKS; t++) {
        if (aciduino.seq.trackType(t) != SEQ_TRACK_NONE) {
          selected = t;
          break;
        }
      }
    }
    _selected_track = selected;
  } else {
    fixSelectedTrack();
  }
  return true;
}

void Aciduino::writeTrackLayout()
{
  // an add/delete/replace was done since the last save: mark the slots so the
  // stored patterns get sanitized to the new session layout
  _layout_reset_mask |= layoutChanged();

  captureTrackLayout();

  if (uCtrl.storage->isSdActive()) {
    // sdcard: the layout travels in the session sidecar
    _session_extras.tempo = uClock.getTempo();
    _session_extras.shuffle_on = uClock.isShuffled();
    _session_extras.shuffle_sig = _shuffle_sig;
    memcpy((void*)_session_extras.shuffle_template, (void*)_shuffle_template, sizeof(_shuffle_template));
    memcpy((void*)_session_extras.pattern_grid, (void*)_pattern_grid, sizeof(_pattern_grid));
    uCtrl.storage->saveSessionConfig((void*)&_session_extras, sizeof(_session_extras));
    return;
  }

  // epprom: tail record, no pattern address moves
  uCtrl.storage->save((void*)&_session_extras.track_layout, sizeof(TRACK_LAYOUT), _epprom_track_layout_address);
}

bool Aciduino::layoutRecordValid()
{
  const TRACK_LAYOUT &layout = _session_extras.track_layout;
  if (layout.magic != (TRACK_LAYOUT_MAGIC ^ (uint16_t)sizeof(TRACK_LAYOUT))) {
    return false;
  }
  if (layout.version != TRACK_LAYOUT_VERSION) {
    return false;
  }

  for (uint8_t t = 0; t < SEQ_MAX_TRACKS; t++) {
    uint8_t type = layout.tracks[t].type;
    if (type != SEQ_TRACK_NONE && type >= SEQ_TRACK_COUNT) {
      return false;
    }
  }
  return true;
}

// rewrite every stored pattern to the session layout: matching tracks keep
// their notes, deleted/changed slots are dropped, new slots are empty
void Aciduino::sanitizePatterns()
{
  uint16_t patterns = uCtrl.storage->isSdActive() ? getNumOfPatterns() : _eprrom_pattern_available;
  uint16_t image_size = aciduino.seq.getPatternImageSize();

  for (uint16_t p = 0; p < patterns; p++) {
    uint32_t address = getPatternEppromAddress((uint8_t)p);
    if (!uCtrl.storage->load((void*)&_pattern_scratch, image_size, address)) {
      continue;
    }

    // slots touched by add/delete plus any slot whose stored type disagrees
    uint16_t changed = _layout_reset_mask;
    for (uint8_t t = 0; t < SEQ_MAX_TRACKS; t++) {
      if (_pattern_scratch.tracks[t].type != _session_extras.track_layout.tracks[t].type) {
        changed |= (1u << t);
      }
    }
    if (changed == 0) {
      continue;
    }

    aciduino.seq.applyLayoutToImage(&_pattern_scratch, _session_extras.track_layout.tracks, changed);
    uCtrl.storage->save((void*)&_pattern_scratch, image_size, address);

    // reset the mute grid entries of the slots whose track changed
    MUTE_PATTERN mute[4];
    uCtrl.storage->load((void*)mute, sizeof(mute), address + image_size);
    for (uint8_t mp = 0; mp < 4; mp++) {
      for (uint8_t t = 0; t < SEQ_MAX_TRACKS; t++) {
        if (changed & (1u << t)) {
          mute[mp].map |= (1 << t);
          mute[mp].map_parts[t] = 0xFFFF;
          _mute_grid[mp].map |= (1 << t);
          _mute_grid[mp].map_parts[t] = 0xFFFF;
        }
      }
    }
    uCtrl.storage->save((void*)mute, sizeof(mute), address + image_size);
  }
}

uint32_t Aciduino::getPatternEppromAddress(uint8_t pattern)
{
  return _eprrom_pattern_address + ((uint32_t)pattern * _pattern_total_mem_size);
}

void Aciduino::loadPattern(uint8_t pattern, int8_t track, int8_t to_track)
{
  uint32_t pattern_address = 0;
  uint16_t saved_patterns = getNumOfPatterns();
  uint16_t image_size = aciduino.seq.getPatternImageSize();

  // an unsaved add/delete must not be reverted by the pattern on storage
  syncTrackLayout();

  // storage bounds: sdcard allows the pending empty slot right past the last saved pattern,
  // epprom pattern space is fixed
  if (uCtrl.storage->isSdActive() ? (pattern > saved_patterns) : (pattern >= saved_patterns)) {
    return;
  }

  // pending empty slot: factory state into memory, waits to be saved to become a pattern
  if (uCtrl.storage->isSdActive() && pattern == saved_patterns) {
    loadDefaultPattern(track);
    return;
  }

  if (track != -1) {
    // load only a track pattern: read the stored image and take its slice
    pattern_address = getPatternEppromAddress(pattern);
    uint8_t src_track = (uint8_t)track;

    // to_track request?
    if (to_track != -1)
      track = to_track;

    if (!uCtrl.storage->load((void*)&_pattern_scratch, image_size, pattern_address)) {
      return;
    }
    if (!aciduino.seq.saveTrackImageFrom(&_pattern_scratch, src_track, aciduino._track_image_buf)) {
      return;
    }

    aciduino.seq.setMute(track, true);
    aciduino.seq.clearStackNote(track);
    if (_layout_reset_mask & (1u << track)) {
      // unsaved add/delete on this slot: the stored data must not come back
      aciduino.seq.initPatternData(track);
    } else if (!aciduino.seq.loadTrackImage(track, aciduino._track_image_buf)) {
      // stale/corrupt blob: back to factory state for this track
      aciduino.seq.initPatternData(track);
    }
    aciduino.seq.setMute(track, false);
    if (_layout_valid) {
      aciduino.seq.applyTrackSetup(track, _session_extras.track_layout.tracks[track]);
    }
  } else {
    // load the whole pattern for all tracks
    pattern_address = getPatternEppromAddress(pattern);
    SEQ_IMAGE *img = (SEQ_IMAGE *)aciduino.seq.getPatternImage();

    // mute all before load into memory(its volatile! here a mute is enongth)
    for (uint8_t i=0; i < SEQ_MAX_TRACKS; i++) {
      aciduino.seq.setMute(i, true);
    }
    // clear all floating notes around
    aciduino.seq.clearStackNote();

    // the whole sequencer image in one block
    uCtrl.storage->load((void*)img, image_size, pattern_address);
    // then the mute grid
    uCtrl.storage->load((void*)_mute_grid, sizeof(_mute_grid), pattern_address + image_size);

    // the session layout always wins over the stored pattern layout
    if (_layout_valid) {
      // stored slots that disagree plus slots touched by an unsaved add/delete
      uint16_t changed = _layout_reset_mask;
      for (uint8_t t = 0; t < SEQ_MAX_TRACKS; t++) {
        if (img->tracks[t].type != _session_extras.track_layout.tracks[t].type) {
          changed |= (1u << t);
        }
      }
      aciduino.seq.applyTrackLayout(_session_extras.track_layout.tracks, changed);

      // a slot whose track changed is muted on every mute pattern
      for (uint8_t mp = 0; mp < 4; mp++) {
        for (uint8_t t = 0; t < SEQ_MAX_TRACKS; t++) {
          if (changed & (1u << t)) {
            _mute_grid[mp].map |= (1 << t);
            _mute_grid[mp].map_parts[t] = 0xFFFF;
          }
        }
      }
    }

    // unmute all loaded fresh and new pattern
    for (uint8_t i=0; i < SEQ_MAX_TRACKS; i++) {
      aciduino.seq.setMute(i, false);
    }
    fixSelectedTrack();
  }
}

// factory state for a not yet saved pattern slot(the pending empty pattern space)
void Aciduino::loadDefaultPattern(int8_t track)
{
  syncTrackLayout();

  if (track == -1) {
    // whole pattern: engine factory state + unmuted mute grid
    aciduino.seq.initPatternData();
    if (_layout_valid) {
      aciduino.seq.applyTrackLayout(_session_extras.track_layout.tracks, _layout_reset_mask);
    }
    for (uint8_t i=0; i < 4; i++) {
      _mute_grid[i].map = 0xFFFF;
      for (uint8_t j=0; j < SEQ_MAX_TRACKS; j++) {
        _mute_grid[i].map_parts[j] = 0xFFFF;
      }
    }
    fixSelectedTrack();
  } else {
    // single track from the pending slot
    aciduino.seq.setMute(track, true);
    aciduino.seq.clearStackNote(track);
    aciduino.seq.initPatternData(track);
    if (_layout_valid) {
      aciduino.seq.applyTrackSetup(track, _session_extras.track_layout.tracks[track]);
    }
    aciduino.seq.setMute(track, false);
  }
}

void Aciduino::savePattern(uint8_t pattern, int8_t track, int8_t from_track)
{
  uint32_t pattern_address = 0;
  uint16_t saved_patterns = getNumOfPatterns();
  uint16_t image_size = aciduino.seq.getPatternImageSize();

  // storage bounds: sdcard grows at the pending slot, epprom pattern space is fixed
  if (uCtrl.storage->isSdActive() ? (pattern > saved_patterns) : (pattern >= saved_patterns)) {
    return;
  }

  // keep the session layout record in sync with the live track list, then
  // propagate a changed layout to every stored pattern
  writeTrackLayout();
  if (_layout_reset_mask) {
    sanitizePatterns();
    _layout_reset_mask = 0;
  }

  if (track != -1) {
    // save only a track pattern
    // to_track request?
    if (from_track != -1)
      track = from_track;

    // read/modify/write the whole pattern block: the stored track array and
    // the pool stay consistent, so other tracks are never clobbered
    pattern_address = getPatternEppromAddress(pattern);
    bool fresh = !uCtrl.storage->load((void*)&_pattern_scratch, image_size, pattern_address);
    if (fresh) {
      // pending/empty slot: start from the session layout with no notes
      aciduino.seq.resetImage(&_pattern_scratch);
    }
    if (!aciduino.seq.exportTrackImage((uint8_t)track, &_pattern_scratch)) {
      return;
    }
    uCtrl.storage->save((void*)&_pattern_scratch, image_size, pattern_address);
    if (fresh) {
      // new block: initialize its mute grid too
      uCtrl.storage->save((void*)_mute_grid, sizeof(_mute_grid), pattern_address + image_size);
    }
  } else {
    // saves the whole pattern for all tracks
    pattern_address = getPatternEppromAddress(pattern);
    // the whole sequencer image in one block
    uCtrl.storage->save((void*)aciduino.seq.getPatternImage(), aciduino.seq.getPatternImageSize(), pattern_address);
    // _mute_grid data
    uCtrl.storage->save((void *)_mute_grid, sizeof(_mute_grid), pattern_address + aciduino.seq.getPatternImageSize());
  }
}

void Aciduino::saveMuteGrid(uint8_t pattern)
{
  uint16_t saved_patterns = getNumOfPatterns();
  if (uCtrl.storage->isSdActive() ? (pattern > saved_patterns) : (pattern >= saved_patterns)) {
    return;
  }
  // _mute_grid data
  uCtrl.storage->save((void*)_mute_grid, sizeof(_mute_grid), getPatternEppromAddress(pattern) + aciduino.seq.getPatternImageSize());
}

// copy current track data into same or another track pattern memory
void Aciduino::copyPattern(uint8_t pattern, int8_t track)
{
    _copy_pattern = pattern;
    _copy_track = track;
}

bool Aciduino::pastePattern(uint8_t pattern, int8_t track)
{
  // an unsaved add/delete must be respected while resolving track types
  syncTrackLayout();

  if (track == -1 || _copy_track == - 1) {
    // paste whole pattern track per track(dynamic layout can leave empty slots)
    for (uint8_t i=0; i < SEQ_MAX_TRACKS; i++) {
      if (aciduino.seq.trackType(i) == SEQ_TRACK_NONE) {
        continue;
      }
      // update _copy_track with current track
      _copy_track = i;
      if (!pastePattern(pattern, i)) {
        return false;
      }
    }
    // paste _mute_grid too!
    uCtrl.storage->save((void*)_mute_grid, sizeof(_mute_grid), getPatternEppromAddress(pattern) + aciduino.seq.getPatternImageSize());
    return true;
  }

  // check if track types are different
  if (aciduino.seq.trackType(_copy_track) != aciduino.seq.trackType(track)) {
    // cant copy.. handle UI
    return false;
  }

  uint16_t image_size = aciduino.seq.getPatternImageSize();

  // copy from memory or from storage?
  bool copy_memory = (_pattern_grid[_copy_track] == _copy_pattern);
  // paste to memory or to storage?
  bool paste_memory = (_pattern_grid[track] == pattern);

  if (copy_memory) {
    // build the single track blob from current memory
    aciduino.seq.saveTrackImage(_copy_track, aciduino._track_image_buf);
  } else {
    // read the source pattern and build the blob from its stored slice
    if (!uCtrl.storage->load((void*)&_pattern_scratch, image_size, getPatternEppromAddress(_copy_pattern))) {
      return false;
    }
    if (!aciduino.seq.saveTrackImageFrom(&_pattern_scratch, _copy_track, aciduino._track_image_buf)) {
      return false;
    }
  }

  if (paste_memory) {
    // TODO: should we save stuffs copied to memory?
    // we paste to current memory
    aciduino.seq.setMute(track, true);
    aciduino.seq.clearStackNote(track);
    bool ok = aciduino.seq.loadTrackImage(track, aciduino._track_image_buf);
    if (!ok) {
      aciduino.seq.initPatternData(track);
    }
    aciduino.seq.setMute(track, false);
    if (_layout_valid) {
      aciduino.seq.applyTrackSetup(track, _session_extras.track_layout.tracks[track]);
    }
    return ok;
  }

  // paste to storage: read/modify/write the target pattern block
  uint32_t pattern_address = getPatternEppromAddress(pattern);
  bool fresh = !uCtrl.storage->load((void*)&_pattern_scratch, image_size, pattern_address);
  if (fresh) {
    aciduino.seq.resetImage(&_pattern_scratch);
  }
  if (!aciduino.seq.loadTrackImageTo(&_pattern_scratch, track, aciduino._track_image_buf)) {
    return false;
  }
  uCtrl.storage->save((void*)&_pattern_scratch, image_size, pattern_address);
  if (fresh) {
    uCtrl.storage->save((void*)_mute_grid, sizeof(_mute_grid), pattern_address + image_size);
  }
  return true;
}

bool Aciduino::checkEppromDataLayoutChange()
{
  uint16_t pattern_size = 0;
  uCtrl.storage->load((void*)&pattern_size, sizeof(pattern_size), _epprom_check_data_address);
  if (pattern_size == _pattern_total_mem_size) {
    return false;
  } else {
    return true;
  }
}

void Aciduino::eppromInit()
{
    uint16_t pattern_size = _pattern_total_mem_size;
    // init epprom session/pattern memory
    // init all epprom slots to defaults
    saveSession();
    for (uint8_t pattern=0; pattern < _eprrom_pattern_available; pattern++) {
      savePattern(pattern);
    }
    // mark epprom first bytes with pattern size to use as a checker for layout changes
    uCtrl.storage->save((void*)&pattern_size, sizeof(pattern_size), _epprom_check_data_address);
}

#if defined(TEENSYDUINO)
extern "C" char* sbrk(int incr);
int Aciduino::freeRam() {
  char top;
  return &top - reinterpret_cast<char*>(sbrk(0));
}
#elif defined(ARDUINO_ARCH_ESP32) || defined(ESP32)
int Aciduino::freeRam ()
{
  return ESP.getFreeHeap();
}
#elif defined(ARDUINO_ARCH_AVR)
int Aciduino::freeRam ()
{
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}
#endif

// used by aciduino.seq object as callback to spill data out
void Aciduino::sequencerOutHandler(uint8_t msg_type, uint8_t note, uint8_t velocity, uint8_t track)
{
  switch(aciduino._track_output_setup[track].output) {
    case MIDI_OUTPUT:
      aciduino.midiSequencerOutHandler(msg_type, note, velocity, aciduino._track_output_setup[track].channel, aciduino._track_output_setup[track].port);
      break;
    case CV_OUTPUT:
      // soon boy...
      break;
    case GATE_OUTPUT:
      // soooooooon...
      break;
  }
}

void Aciduino::midiSequencerOutHandler(uint8_t msg_type, uint8_t byte1, uint8_t byte2, uint8_t channel, uint8_t port)
{
  msg_interrupt.type = msg_type == NOTE_ON ? uctrl::protocol::midi::NoteOn : uctrl::protocol::midi::NoteOff;
  msg_interrupt.data1 = byte1;
  msg_interrupt.data2 = byte2;
  msg_interrupt.channel = channel;
  uCtrl.midi->write(&msg_interrupt, port+1, 1);
}

// All midi interface registred thru uCtrl get incomming data thru this callback
void Aciduino::midiInputHandler(uctrl::protocol::midi::MIDI_MESSAGE * msg, uint8_t port, uint8_t interrupted)
{
  if ( uClock.getClockMode() == uClock.EXTERNAL_CLOCK ) {
    // external tempo control
    switch (msg->type) {
        case uctrl::protocol::midi::Clock:
          uClock.clockMe();
          return;

        case uctrl::protocol::midi::Start:
          uClock.start();
          return;

        case uctrl::protocol::midi::Stop:
          uClock.stop();
          return;
    }
  }

  // control goes thru selected track(no matter what channel)
  switch (msg->type) {

    //
    // Note ON/OFF
    //
    case uctrl::protocol::midi::NoteOn:
    case uctrl::protocol::midi::NoteOff:
      aciduino.seq.input(aciduino._selected_track, msg->type == uctrl::protocol::midi::NoteOn ? NOTE_ON : NOTE_OFF, msg->data1, msg->data2, interrupted);
      break;

    //
    // Control Change
    //
    case uctrl::protocol::midi::ControlChange:

      // CC MAP Control
      switch ( msg->data1 ) {

        // Sustain Controller 64
        case 64:
          //if (_foot_pedal_rec == true) {
          //  if ( msg->data2 == 127 ) {
              //footSwitchHandle(HIGH);
          //  } else if ( msg->data2 == 0 ) {
              //footSwitchHandle(LOW);
          //  }
          //  return;
          //}
          break;
      }
      break;

    default:
      break;

  }
}

// a port to read midi notes 1ms
void Aciduino::midiHandle() {
  //while (uCtrl.midi->read(2)) {
  //}
  uCtrl.midi->read(aciduino._midi_rec_port, 1);
}

// used by uCtrl at 250us speed to get MIDI sync input messages on time
void Aciduino::midiHandleSync() {
  if (aciduino._midi_clock_port > 0) { // && uClock.getClockMode() == uClock.EXTERNAL_CLOCK) {
    //while (uCtrl.midi->read(_midi_clock_port)) {
    //}
    uCtrl.midi->read(aciduino._midi_clock_port, 1);
  }
}

#if defined(USE_BPM_LED)
void Aciduino::handle_bpm_led(uint32_t tick)
{
  // BPM led indicator
  if ( !(tick % (96)) || (tick == 1) ) {  // first compass step will flash longer
    _bpm_blink_timer = 8;
    uCtrl.dout->write(BPM_LED, HIGH, 1);
  } else if ( !(tick % (24)) ) {   // each quarter led on
    uCtrl.dout->write(BPM_LED, HIGH, 1);
  } else if ( !(tick % _bpm_blink_timer) ) { // get led off
    uCtrl.dout->write(BPM_LED, LOW, 1);
    _bpm_blink_timer = 1;
  }
}
#endif

// keep gears synced on 24PPQN resolution
void Aciduino::onSync24Callback(uint32_t tick)
{
  // Send MIDI_CLOCK to external gears
  aciduino.sendMidiClock();
#if defined(USE_BPM_LED)
  // led clock monitor
  handle_bpm_led(tick);
#endif
}

// The callback function wich will be called by uClock each Pulse of 96PPQN clock resolution.
void Aciduino::onPPQNCallback(uint32_t tick)
{
  // sequencer tick
  aciduino.seq.on96PPQN(tick);
}

// The callback function wich will be called by uClock each new step event time
void Aciduino::onStepCallback(uint32_t step)
{
  // sequencer tick
  aciduino.seq.onStep(step, uClock.getShuffleLength());
}

// The callback function wich will be called when clock starts by using Clock.start() method.
void Aciduino::onClockStart()
{
  aciduino.sendMidiStart();
  aciduino._playing = 1;
}

// The callback function wich will be called when clock stops by using Clock.stop() method.
void Aciduino::onClockStop()
{
  aciduino.sendMidiStop();
  // clear all tracks stack note(all floating notes off!)
  aciduino.seq.clearStackNote();
  aciduino._playing = 0;
#if defined(USE_BPM_LED)
  // force to turn bpm led off
  uCtrl.dout->write(BPM_LED, LOW);
#endif
}

// static instantiation
Aciduino aciduino;
