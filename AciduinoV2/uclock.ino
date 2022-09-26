void handle_bpm_led(uint32_t tick)
{
  // BPM led indicator
  if ( !(tick % (96)) || (tick == 1) ) {  // first compass step will flash longer
    bpm_blink_timer = 8;
    uCtrl.dout->write(1, HIGH, 1);
  } else if ( !(tick % (24)) ) {   // each quarter led on
    uCtrl.dout->write(1, HIGH, 1);
  } else if ( !(tick % bpm_blink_timer) ) { // get led off
    uCtrl.dout->write(1, LOW, 1);
    bpm_blink_timer = 1;
  }
}

void uClockSetup()
{
  // Inits the clock
  uClock.init();
  
  // Set the callback function for the clock output to send MIDI Sync message.
  uClock.setClock96PPQNOutput(ClockOut96PPQN);
  
  // Set the callback function for the step sequencer on 16ppqn
  uClock.setClock16PPQNOutput(ClockOut16PPQN);  
  
  // Set the callback function for MIDI Start and Stop messages.
  uClock.setOnClockStartOutput(onClockStart);  
  uClock.setOnClockStopOutput(onClockStop);
  
  // Set the clock BPM to 126 BPM
  uClock.setTempo(126);
  //uClock.setMode(uClock.EXTERNAL_CLOCK);
}

// The callback function wich will be called by uClock each Pulse of 16PPQN clock resolution. Each call represents exactly one step.
void ClockOut16PPQN(uint32_t tick) 
{
  // sequencer tick
  AcidSequencer.on16PPQN(tick);
}

// The callback function wich will be called by uClock each Pulse of 96PPQN clock resolution.
void ClockOut96PPQN(uint32_t tick) 
{
  // sequencer tick
  AcidSequencer.on96PPQN(tick);
  // led clock monitor
  handle_bpm_led(tick);
}

// The callback function wich will be called when clock starts by using Clock.start() method.
void onClockStart() 
{
  msg.type = uctrl::protocol::midi::Start;
  uCtrl.midi->writeAllPorts(&msg);
  _playing = true;
}

// The callback function wich will be called when clock stops by using Clock.stop() method.
void onClockStop() 
{
  msg.type = uctrl::protocol::midi::Stop;
  uCtrl.midi->writeAllPorts(&msg);
  uCtrl.dout->write(1, LOW);
  // clear all tracks stack note
  AcidSequencer.clearStackNote();
  _playing = false;
}
