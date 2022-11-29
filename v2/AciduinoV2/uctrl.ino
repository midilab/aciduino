// Display device
//U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8X8_PIN_NONE);
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

void playStop()
{
  if (_playing)
    uClock.stop();
  else
    uClock.start();
}

void tempoSetup()
{
  uCtrl.page->selectComponent(topBarComponent);
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
  // uCtrl initial setup
  // drivers and handlers
  
  //
  // OLED setup
  // Please check you oled model to correctly init him
  //
  uCtrl.initOled(&u8g2);
#if defined(USE_LITE_BOARD)
  uCtrl.oled->flipDisplay(1); 
#endif
  uCtrl.oled->print("booting", 4, 1);
  uCtrl.oled->print("please wait...", 5, 1); 

  //
  // DIN Module
  //
  uCtrl.oled->print(">init din...", 8, 1);  
  uCtrl.initDin();
#if defined(USE_LITE_BOARD)
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
#elif defined(USE_UONE_BOARD)
  uCtrl.din->plug(24);
  uCtrl.din->plug(25);
  uCtrl.din->plug(23);
  // encoders setup
  // in pair order always! and pairs starting with odd ids
  uCtrl.din->encoder(ENCODER_DEC, ENCODER_INC);
#endif

  //
  // DOUT Module
  //
  uCtrl.oled->print(">init dout...", 8, 1);
#if defined(USE_LITE_BOARD)
  uCtrl.initDout();
  uCtrl.dout->plug(LED_BUILTIN);
#elif defined(USE_UONE_BOARD)
  uCtrl.initDout(&SPI, 17);
  uCtrl.dout->plug(3);
#endif

  //
  // AIN Module
  //
#if defined(USE_LITE_BOARD)
  uCtrl.oled->print(">init ain...", 8, 1);
  uCtrl.initAin();
  uCtrl.ain->plug(A9);
  // our aciduino v2 protoboard can only connect with vcc and gnd swaped, lets inform that to uctrl ain driver
  uCtrl.ain->invertRead(true);
  // little hack to make the pot on aciduino protoboard work, ground our gnd pot pin 22 to avoid floating noises around...
  pinMode(22, OUTPUT);
  digitalWrite(22, LOW);
#endif

  //
  // Capacitive Touch Module
  //
#if defined(USE_UONE_BOARD)
  uCtrl.oled->print(">init ctouch...", 8, 1);
  // 4 and 3 needs a swap, fix it on rev 0.2
  uCtrl.initCapTouch(6, 5, 3, 4);
  //uCtrl.touch->setThreshold(45);
  uCtrl.touch->setThreshold(41);
  //uCtrl.touch->setThreshold(38);
  uCtrl.touch->plug(A6);
  uCtrl.touch->plug(A7);
#endif

  //
  // SdCard Module
  //
  //uCtrl.initSdCard(&SPI, 7); 
  
  //
  // Page Module for UI
  //
  uCtrl.initPage();
  welcome_page_init();
  step_sequencer_page_init();
  generative_page_init();
  live_page_init();
#if defined(USE_LITE_BOARD)
  // use component UI
  uCtrl.page->setNavComponentCtrl(SHIFT_BUTTON, UP_BUTTON, DOWN_BUTTON, PREVIOUS_BUTTON, NEXT_BUTTON, PAGE_BUTTON_1, PAGE_BUTTON_2, GENERIC_BUTTON_1, GENERIC_BUTTON_2);
  uCtrl.page->setNavPot(1);
#elif defined(USE_UONE_BOARD)
  // use component UI
  uCtrl.page->setNavComponentCtrl(SHIFT_BUTTON, UP_BUTTON, DOWN_BUTTON, PREVIOUS_BUTTON, NEXT_BUTTON, PAGE_BUTTON_1, PAGE_BUTTON_2, GENERIC_BUTTON_1, GENERIC_BUTTON_2, ENCODER_DEC, ENCODER_INC);
#endif
  // use shift button callback?
  // slave/master tempo set
  uCtrl.page->setShiftCtrlAction(PAGE_BUTTON_1, tempoSetup);
  // play/stop tempo
  uCtrl.page->setShiftCtrlAction(PAGE_BUTTON_2, playStop);
  // previous track
  uCtrl.page->setShiftCtrlAction(GENERIC_BUTTON_1, previousTrack);
  // next track
  uCtrl.page->setShiftCtrlAction(GENERIC_BUTTON_2, nextTrack);
  uCtrl.page->setFunctionDrawCallback(functionDrawCallback);

  //
  uCtrl.init();

  // get all leds off
  uCtrl.dout->writeAll(LOW);
  
  // 
  uCtrl.page->setPage(1);
}
