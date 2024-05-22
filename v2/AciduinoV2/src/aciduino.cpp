#include "aciduino.hpp"

//
// setup and runtime
//
void Aciduino::init() {
  // setup uctrl hardware and control interfaces
  uCtrlSetup();
  // setup clock system
  uClockSetup();
  // init the sequencer
  initSequencer();
}

void Aciduino::run() {
  // let uCtrl do his job
  uCtrl.run();
}

void Aciduino::playStop()
{
  if (aciduino.isPlaying())
    uClock.stop();
  else
    uClock.start();
}

void Aciduino::recToggle()
{
  aciduino.seq.setRecStatus(!aciduino.seq.getRecStatus());
}

void Aciduino::previousTrack()
{
  if (_selected_track == 0) {
    _selected_track = aciduino.seq.getTrackNumber() - 1;
  } else {
    --_selected_track;
  }
}

void Aciduino::nextTrack()
{
  if (_selected_track == aciduino.seq.getTrackNumber() - 1) {
    _selected_track = 0;
  } else {
    ++_selected_track;
  }
}

//
// Sequencer initialization
//
void Aciduino::initSequencer()
{
  // init default track output data
  for(uint8_t track=0; track < TRACK_NUMBER_303+TRACK_NUMBER_808; track++) {
    _track_output_setup[track].output = MIDI_OUTPUT;
    _track_output_setup[track].channel = track;
    _track_output_setup[track].port = 0;
  }
  
  // the acid sequencer main output callback
  aciduino.seq.setOutputCallback(sequencerOutHandler);

  // we're going to init epprom in two cases:
  // if track layout changes || if no aciduino data present at epprom
  // so if you want to keep your pattenrs and change layout consider backup pattenrs into another sequencer
  // because they will be wipeout for the new memory layout to take place
  if (checkEppromDataLayoutChange()) {
    eppromInit();
  }

  // load shuffle form memory pattern?
  int8_t init_template[2] = {0,0};
  uClock.setShuffleTemplate(init_template, 2);

  // load epprom saved session
  loadSession();
  // load first pattern
  loadPattern(0);
}

void Aciduino::uClockSetup()
{
  // Inits the clock
  uClock.init();
  
  // Set the callback function for the step sequencer on 96PPQN and for step sequencer feature
  uClock.setOnPPQN(onPPQNCallback);
  uClock.setOnStep(onStepCallback);
  // Set the callback function for the clock output to send MIDI Sync message.
  uClock.setOnSync24(onSync24Callback);
  // Set the callback function for MIDI Start and Stop messages.
  uClock.setOnClockStart(onClockStart);  
  uClock.setOnClockStop(onClockStop);
  
  // Set the clock BPM to 126 BPM
  uClock.setTempo(126);
  //uClock.setMode(uClock.EXTERNAL_CLOCK);
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
void Aciduino::sendMidiCC(uint8_t cc, uint8_t value, uint8_t channel, uint8_t port, uint8_t interrupted = 0) {
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
  uCtrl.storage->load((void*)_generative_303, sizeof(_generative_303), EPPROM_SESSION_ADDRESS);
  uCtrl.storage->load((void*)_generative_808, sizeof(_generative_808));
  uCtrl.storage->load((void*)_control_map_global, sizeof(_control_map_global));
  uCtrl.storage->load((void*)_track_output_setup, sizeof(_track_output_setup));
}

void Aciduino::saveSession()
{
  // save session data
  uCtrl.storage->save((void*)_generative_303, sizeof(_generative_303), EPPROM_SESSION_ADDRESS);
  uCtrl.storage->save((void*)_generative_808, sizeof(_generative_808));
  uCtrl.storage->save((void*)_control_map_global, sizeof(_control_map_global));
  uCtrl.storage->save((void*)_track_output_setup, sizeof(_track_output_setup));
}

uint16_t Aciduino::getPatternEppromAddress(uint8_t pattern, int8_t track)
{
  uint16_t pattern_address = EPRROM_PATTERN_ADDRESS + (pattern * PATTERN_TOTAL_MEM_SIZE);
  if (track != -1) {
    pattern_address += aciduino.seq.is303(track) ? PATTERN_303_TRACK_SIZE * track : PATTERN_303_MEM_SIZE + (PATTERN_808_TRACK_SIZE * (track-TRACK_NUMBER_303));
  }
  return pattern_address;
}

void Aciduino::loadPattern(uint8_t pattern, int8_t track, int8_t to_track)
{
  uint16_t pattern_address = 0;
  
  if (track != -1) {
    // load only a track patttern
    pattern_address = getPatternEppromAddress(pattern, track);
    
    // to_track request?
    if (to_track != -1)
      track = to_track;
      
    aciduino.seq.setMute(track, true);
    aciduino.seq.clearStackNote(track);
    uCtrl.storage->load((void*)aciduino.seq.getPatternData(track), (aciduino.seq.is303(track) ? PATTERN_303_TRACK_SIZE : PATTERN_808_TRACK_SIZE), pattern_address);
    aciduino.seq.setMute(track, false);

    // load _mute_pattern only for track?
    //...
  } else {
    // load the whole pattern for all tracks
    pattern_address = getPatternEppromAddress(pattern);
    
    // mute all before load into memory(its volatile! here a mute is enongth)
    for (uint8_t i=0; i < (TRACK_NUMBER_303+TRACK_NUMBER_808); i++) {
      aciduino.seq.setMute(i, true);
    }
    // clear all floating notes around
    aciduino.seq.clearStackNote();
    
    // get 303 whole pattern data first
    uCtrl.storage->load((void*)aciduino.seq.getPatternData(0), PATTERN_303_MEM_SIZE, pattern_address);
    // then 808 whole pattern data last
    uCtrl.storage->load((void*)aciduino.seq.getPatternData(TRACK_NUMBER_303), PATTERN_808_MEM_SIZE, pattern_address+PATTERN_303_MEM_SIZE);

    // load _mute_pattern
    uCtrl.storage->load((void*)_mute_pattern, sizeof(_mute_pattern), pattern_address+PATTERN_303_MEM_SIZE+PATTERN_808_MEM_SIZE);
    
    // unmute all loaded fresh and new pattern
    for (uint8_t i=0; i < (TRACK_NUMBER_303+TRACK_NUMBER_808); i++) {
      aciduino.seq.setMute(i, false);
    }
  }
}

void Aciduino::savePattern(uint8_t pattern, int8_t track, int8_t from_track)
{
  uint16_t pattern_address = 0;
  if (track != -1) {
    // save only a track patttern
    pattern_address = getPatternEppromAddress(pattern, track);
    
    // to_track request?
    if (from_track != -1)
      track = from_track;
      
    uCtrl.storage->save((void*)aciduino.seq.getPatternData(track), (aciduino.seq.is303(track) ? PATTERN_303_TRACK_SIZE : PATTERN_808_TRACK_SIZE), pattern_address);

    // save _mute_pattern for a specific patern
    //...
  } else {
    // saves the whole pattern for all tracks
    pattern_address = getPatternEppromAddress(pattern);
    // save 303 whole pattern data first
    uCtrl.storage->save((void*)aciduino.seq.getPatternData(0), PATTERN_303_MEM_SIZE, pattern_address);
    // then 808 whole pattern data last
    uCtrl.storage->save((void*)aciduino.seq.getPatternData(TRACK_NUMBER_303), PATTERN_808_MEM_SIZE, pattern_address+PATTERN_303_MEM_SIZE);
    // _mute_grid data
    uCtrl.storage->save((void *)_mute_pattern, sizeof(_mute_pattern), pattern_address+PATTERN_303_MEM_SIZE+PATTERN_808_MEM_SIZE);
  }
}

void Aciduino::saveMuteGrid(uint8_t pattern)
{
  // _mute_grid data
  uCtrl.storage->save((void*)_mute_pattern, sizeof(_mute_pattern), getPatternEppromAddress(pattern)+PATTERN_303_MEM_SIZE+PATTERN_808_MEM_SIZE);
}

// copy current track data into same or another track pattern memory
void Aciduino::copyPattern(uint8_t pattern, int8_t track)
{
    _copy_pattern = pattern;
    _copy_track = track;
}

bool Aciduino::pastePattern(uint8_t pattern, int8_t track)
{
  bool copy_memory = true;
  
  if (track == -1 || _copy_track == - 1) {
    // paste whole pattern track per track
    for (uint8_t i=0; i < aciduino.seq.getTrackNumber(); i++) {
      // update _copy_track with current track
      _copy_track = i;
      pastePattern(pattern, i);
    }
    // paste _mute_grid too!
    uCtrl.storage->save((void*)_mute_pattern, sizeof(_mute_pattern), getPatternEppromAddress(pattern)+PATTERN_303_MEM_SIZE+PATTERN_808_MEM_SIZE);
    return true;
  }

  // check if track types are different
  if (aciduino.seq.is303(_copy_track) != aciduino.seq.is303(track)) {
    // cant copy.. handle UI
    return false;
  }
  
  // copy from memory or from storage?
  if (_pattern_grid[_copy_track] == _copy_pattern) {
    // we copy from current memory data
    copy_memory = true;
  } else {
    // we copy from storage data
    copy_memory = false;
  }

  // paste to memory or to storage?
  if (_pattern_grid[track] == pattern) {
    // TODO: should we save stuffs copied to memory?
    // we paste to current memory
    if (copy_memory) {
      // if paste is from memory to memory
      aciduino.seq.setMute(track, true);
      aciduino.seq.clearStackNote(track);
      memcpy(aciduino.seq.getPatternData(track), aciduino.seq.getPatternData(_copy_track), (aciduino.seq.is303(track) ? PATTERN_303_TRACK_SIZE : PATTERN_808_TRACK_SIZE));
      aciduino.seq.setMute(track, false);
    } else {
      // if paste is from storage to memory
      loadPattern(_copy_pattern, _copy_track, track);
    }
  } else {
    // we paste to storage data
    if (copy_memory) {
      // if paste is from memory to storage
      savePattern(pattern, track, _copy_track);
    } else {
      // if paste is from storage to storage
      uCtrl.storage->copy(getPatternEppromAddress(_copy_pattern, _copy_track), getPatternEppromAddress(pattern, track), (aciduino.seq.is303(track) ? PATTERN_303_TRACK_SIZE : PATTERN_808_TRACK_SIZE));
    }
  }

  return true;

}

/*
void Engine808::setBufferTrack(uint8_t track)
{
  // copy track into buffer
  memcpy(_track_helper_buffer, _sequencer[track], sizeof(_track_helper_buffer));
}
 */

bool Aciduino::checkEppromDataLayoutChange()
{
  uint16_t pattern_size = 0;
  uCtrl.storage->load((void*)&pattern_size, sizeof(pattern_size), EPPROM_CHECK_DATA_ADDRESS);
  if (pattern_size == PATTERN_TOTAL_MEM_SIZE) {
    return false;
  } else {
    return true;
  }
}

void Aciduino::eppromInit()
{
    uint16_t pattern_size = PATTERN_TOTAL_MEM_SIZE;
    // init epprom session/pattern memory
    // init all epprom slots to defaults
    saveSession();
    for (uint8_t pattern=0; pattern < EPRROM_PATTERN_AVAILABLE; pattern++) {
      savePattern(pattern);
    }
    // mark epprom first bytes with pattern size to use as a checker for layout changes
    uCtrl.storage->save((void*)&pattern_size, sizeof(pattern_size), EPPROM_CHECK_DATA_ADDRESS);
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

// CALLBACKS
// used by aciduino.seq object as callback to spill data out
void sequencerOutHandler(uint8_t msg_type, uint8_t note, uint8_t velocity, uint8_t track)
{
  switch(_track_output_setup[track].output) {
    case MIDI_OUTPUT:
      midiSequencerOutHandler(msg_type, note, velocity, _track_output_setup[track].channel, _track_output_setup[track].port);
      break;
    case CV_OUTPUT:
      // soon boy...
      break;
    case GATE_OUTPUT:
      // soooooooon...
      break;
  }
}

// All midi interface registred thru uCtrl get incomming data thru this callback
void midiInputHandler(uctrl::protocol::midi::MIDI_MESSAGE * msg, uint8_t port, uint8_t interrupted) 
{
  if ( uClock.getMode() == uClock.EXTERNAL_CLOCK ) {
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
      aciduino.seq.input(_selected_track, msg->type == uctrl::protocol::midi::NoteOn ? NOTE_ON : NOTE_OFF, msg->data1, msg->data2, interrupted);
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

// used by aciduino.seq object as callback to spill midi messages out
void midiSequencerOutHandler(uint8_t msg_type, uint8_t byte1, uint8_t byte2, uint8_t channel, uint8_t port)
{
  msg_interrupt.type = msg_type == NOTE_ON ? uctrl::protocol::midi::NoteOn : uctrl::protocol::midi::NoteOff;
  msg_interrupt.data1 = byte1;
  msg_interrupt.data2 = byte2;
  msg_interrupt.channel = channel;
  uCtrl.midi->write(&msg_interrupt, port+1, 1);
}

// a port to read midi notes 1ms
void midiHandle() {
  //while (uCtrl.midi->read(2)) {
  //}
  uCtrl.midi->read(_midi_rec_port, 1);
}

// used by uCtrl at 250us speed to get MIDI sync input messages on time
void midiHandleSync() {
  if (_midi_clock_port > 0 && uClock.getMode() == uClock.EXTERNAL_CLOCK) {
    //while (uCtrl.midi->read(_midi_clock_port)) {
    //}
    uCtrl.midi->read(_midi_clock_port, 1);
  }
}

//
// clock schema setup
//
uint8_t _bpm_blink_timer = 1;

#if defined(USE_BPM_LED)
void handle_bpm_led(uint32_t tick)
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
void onSync24Callback(uint32_t tick) 
{
  // Send MIDI_CLOCK to external gears
  sendMidiClock();
#if defined(USE_BPM_LED)
  // led clock monitor
  handle_bpm_led(tick);
#endif
}

// The callback function wich will be called by uClock each Pulse of 96PPQN clock resolution.
void onPPQNCallback(uint32_t tick) 
{
  // sequencer tick
  aciduino.seq.on96PPQN(tick);
}

// The callback function wich will be called by uClock each new step event time
void onStepCallback(uint32_t step) 
{
  // sequencer tick
  aciduino.seq.onStep(step, uClock.getShuffleLength());
}

// The callback function wich will be called when clock starts by using Clock.start() method.
void onClockStart() 
{
  sendMidiStart();
  aciduino.isPlaying() = true;
}

// The callback function wich will be called when clock stops by using Clock.stop() method.
void onClockStop() 
{
  sendMidiStop();
  // clear all tracks stack note(all floating notes off!)
  aciduino.seq.clearStackNote();
  aciduino.isPlaying() = false;
#if defined(USE_BPM_LED)
  // force to turn bpm led off
  uCtrl.dout->write(BPM_LED, LOW);
#endif
}

// static instantiation
Aciduino aciduino;