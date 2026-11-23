
void pattern_page_create()
{
  uCtrl.dout->writeAll(LOW); 
}

void pattern_page_destroy()
{
  uCtrl.dout->writeAll(LOW);
}

// called each cycle interaction of interface object for UI refresh
void pattern_page_refresh(uint8_t subpage)
{
  uCtrl.page->component(topBarComponent, 1, 1);

  if (subpage == 0) {
    // pattern page
    /*
    uCtrl.oled->print("# pattern 1", 3, 1);
    uCtrl.oled->print("# pattern 2", 4, 1);
    uCtrl.oled->print("# pattern 3", 5, 1);
    uCtrl.oled->print("# pattern 4", 6, 1);
    */
    uCtrl.page->component(patternComponent, 2, 1, true);
  } else if (subpage == 1) {
    uCtrl.page->component(mutePatternComponent, 3, 1, true);
  }

#if defined(USE_TRACK_STRIP)
  if (subpage == 0) {
    patternComponent.padView();
  } else if (subpage == 1) {
    mutePatternComponent.padView();
  }
#endif
  trackStripView();
}

void pattern_page_digital_input(uint8_t control_id, uint16_t value, uint8_t subpage)
{  
  if (trackStripInput(control_id, value)) return;

#if defined(USE_TRACK_STRIP)
  if (value == LOW) return;

  if (control_id >= STEP_BUTTON_1 && control_id <= STEP_BUTTON_16) {
    uint8_t number = control_id - STEP_BUTTON_1;
    if (subpage == 0) {
      // patterns: tap = selected track, shift = all tracks(scene)
      patternComponent.pad(number, uCtrl.page->isShiftPressed());
    } else if (subpage == 1) {
      // parts of the selected 808 track + mute automation patterns
      mutePatternComponent.pad(number);
    }
  }
#endif
}

void pattern_page_analog_input(uint8_t control_id, uint16_t value, uint8_t subpage)
{
  // special fixed pots for interaction like generative ones
}

void pattern_page_init()
{
  //uCtrl.page->set("ptrn", pattern_page_create, pattern_page_destroy, pattern_page_refresh, pattern_page_digital_input, pattern_page_analog_input, 2);
  uCtrl.page->set("ptrn", pattern_page_create, pattern_page_destroy, pattern_page_refresh, pattern_page_digital_input, nullptr, 2);
}
