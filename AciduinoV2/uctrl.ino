// Display device
//U8X8 * u8x8 = new U8X8_SSD1306_128X64_NONAME_HW_I2C(U8X8_PIN_NONE);
//U8X8 * u8x8 = new U8X8_SH1106_128X64_NONAME_HW_I2C(U8X8_PIN_NONE);
U8G2 * u8g2 = new U8G2_SH1106_128X64_NONAME_F_HW_I2C(U8G2_R0, U8X8_PIN_NONE);
// UART MIDI port 1
MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI1);

void uCtrlInit() {
// process midi at 250 microseconds speed
  uCtrl.setOn250usCallback(midiHandle);
  // process sequencer at 1 milisecond speed
  //uCtrl.setOn1msCallback(sequencerHandle);

  //
  // MIDI setup
  //
  uCtrl.initMidi();
  uCtrl.midi->plug(&usbMIDI);
  uCtrl.midi->plug(&MIDI1);
  uCtrl.midi->setMidiInputCallback(midiInputHandler);
  //uCtrl.midi->setMidiOutputCallback(midiOutputHandler);

  //
  // OLED setup
  // Please check you oled model to correctly init him
  //
  uCtrl.initOled(u8g2);
  uCtrl.oled->flipDisplay(1); 
  uCtrl.oled->print("booting", 4, 1);
  uCtrl.oled->print("please wait...", 5, 1); 
  
  //
  // DIN Module
  //
  uCtrl.initDin();
  // those are the detent encoder pins. see above how to setup they as detent
  // previous
  uCtrl.din->plug(16);
  // next
  uCtrl.din->plug(15);
  // up
  uCtrl.din->plug(17);
  // down
  uCtrl.din->plug(9);
  // page button 1
  uCtrl.din->plug(6);
  // page button 2
  uCtrl.din->plug(7);
  // decrementer button
  uCtrl.din->plug(3);
  // incrementer button
  uCtrl.din->plug(5); 
  // shift button
  uCtrl.din->plug(4);
  // little hack to make the shift protoboard work, ground our gnd button pin 2 to avoid floating noises around...
  pinMode(2, OUTPUT);
  digitalWrite(2, LOW);
  // encoders setup
  // in pair order always! and pairs starting with odd ids
  //uCtrl.din->encoder(PREVIOUS_BUTTON, NEXT_BUTTON);

  //
  // DOUT Module
  //
  uCtrl.initDout();
  uCtrl.dout->plug(LED_BUILTIN);

  //
  // AIN Module
  //
  uCtrl.initAin();
  uCtrl.ain->plug(A9);
  // our aciduino v2 protoboard can only connect with vcc and gnd swaped, lets inform that to uctrl ain driver
  uCtrl.ain->invertRead(true);
  // little hack to make the pot on aciduino protoboard work, ground our gnd pot pin 22 to avoid floating noises around...
  pinMode(22, OUTPUT);
  digitalWrite(22, LOW);
  
  //
  // Page Module for UI
  //
  uCtrl.initPage(4);
  // Track config page
  // it will be registred in order call
  welcome_page_init();
  step_sequencer_page_init();
  generative_page_init();
  live_page_init();
  // use component UI
  uCtrl.page->setNavComponentCtrl(SHIFT_BUTTON, UP_BUTTON, DOWN_BUTTON, PREVIOUS_BUTTON, NEXT_BUTTON, PAGE_BUTTON_1, PAGE_BUTTON_2, GENERIC_BUTTON_1, GENERIC_BUTTON_2, 1);
  uCtrl.page->setFunctionDrawCallback(functionDrawCallback);
  uCtrl.page->setNavPot(true);
  uCtrl.page->setPage(1);

  //
  uCtrl.init();
}

void functionDrawCallback(const char * f1, const char * f2, uint8_t f1_state, uint8_t f2_state)
{
  // menu action
  uCtrl.oled->print(f1, 8, 1+((14-strlen(f1))/2)); 
  uCtrl.oled->print(f2, 8, 14+((14-strlen(f2))/2)); 
  // state variation
  if (f1_state == 1) {
    uCtrl.oled->display->drawBox(0, 57, 63, 7);
  }
  if (f2_state == 1) {
    uCtrl.oled->display->drawBox(64, 57, 64, 7);
  }
  // horizontal line
  uCtrl.oled->display->drawBox(0, 56, 128, 1);
  // vertical separator
  //uCtrl.oled->display->drawBox(64, 59, 1, 5);
}

// All midi interface registred thru uCtrl get incomming data thru callback
void midiInputHandler(uctrl::protocol::midi::MIDI_MESSAGE * msg, uint8_t port, uint8_t interrupted) 
{
  if (uClock.getMode() == uClock.EXTERNAL_CLOCK) {
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
}

void midiHandle() {
  while (uCtrl.midi->read(1)) {
  }
}
