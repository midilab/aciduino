
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
  
}

void pattern_page_digital_input(uint8_t control_id, uint16_t value, uint8_t subpage)
{  
  // step array buttons handling?
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
