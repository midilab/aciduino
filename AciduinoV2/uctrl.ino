// Display device
//U8X8 * u8x8 = new U8X8_SSD1306_128X64_NONAME_HW_I2C(U8X8_PIN_NONE);
//U8X8 * u8x8 = new U8X8_SH1106_128X64_NONAME_HW_I2C(U8X8_PIN_NONE);
//U8G2 * u8g2 = new U8G2_SH1106_128X64_NONAME_F_HW_I2C(U8G2_R0, U8X8_PIN_NONE);

U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

void playStop()
{
  if (_playing)
    uClock.stop();
  else
    uClock.start();
}

void clockMode()
{
  // so instead of clock mode lets get call a screen(create a dialog call screen with f1/2 options and a control over something) with tempo setup... pot controls tempo, inc/dec controls fine tempo tune
  // F1 and F@ for the new screen will be internal, external with state to choose from
  // or we just move the selector to the tempo at top bar and leave it as non selectable via user interface nav buttons
  uClock.setMode(uClock.getMode() == uClock.INTERNAL_CLOCK ? uClock.EXTERNAL_CLOCK : uClock.INTERNAL_CLOCK);
}

void previousTrack()
{
  if (_selected_track == 0) {
    _selected_track = AcidSequencer.getTrackNumber() - 1;
  } else {
    --_selected_track;
  }
}

void nextTrack()
{
  if (_selected_track == AcidSequencer.getTrackNumber() - 1) {
    _selected_track = 0;
  } else {
    ++_selected_track;
  }
}

void uCtrlSetup() {
  // process midi at 250 microseconds speed
  uCtrl.setOn250usCallback(midiInputHandle);

  //
  // OLED setup
  // Please check you oled model to correctly init him
  //
  uCtrl.initOled(&u8g2);
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
  uCtrl.initPage();
  welcome_page_init();
  step_sequencer_page_init();
  generative_page_init();
  live_page_init();
  // use component UI
  uCtrl.page->setNavComponentCtrl(SHIFT_BUTTON, UP_BUTTON, DOWN_BUTTON, PREVIOUS_BUTTON, NEXT_BUTTON, PAGE_BUTTON_1, PAGE_BUTTON_2, GENERIC_BUTTON_1, GENERIC_BUTTON_2, 1);
  // use shift button callback?
  // slave/master tempo set
  uCtrl.page->setShiftCtrlAction(PAGE_BUTTON_1, clockMode);
  // play/stop tempo
  uCtrl.page->setShiftCtrlAction(PAGE_BUTTON_2, playStop);
  // previous track
  uCtrl.page->setShiftCtrlAction(GENERIC_BUTTON_1, previousTrack);
  // next track
  uCtrl.page->setShiftCtrlAction(GENERIC_BUTTON_2, nextTrack);
  
  uCtrl.page->setFunctionDrawCallback(functionDrawCallback);
  uCtrl.page->setNavPot(true);

  //
  uCtrl.init();

  // 
  uCtrl.page->setPage(1);
}
