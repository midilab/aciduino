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

// called each cycle interaction of interface object for UI refresh
void system_page_refresh(uint8_t subpage)
{ 
  // https://javl.github.io/image2cpp/
  //uCtrl.oled->display->drawXBMP(0, 20, 124, 36, logo_bpm);

  uCtrl.page->component(topBarComponent, 1, 1);

  //uCtrl.oled->print(String(), 2, 1);

  // save options goes into this page for session related
  //...

  if (subpage == 0) {
    
    uCtrl.page->component(tempoTransportComponent, 3, 1, true);
    uCtrl.page->component(tempoClockSourceComponent, 4, 1);
    
    uCtrl.page->component(recInputComponent, 3, 2);
    uCtrl.page->component(recModeComponent, 4, 2);
    
    uCtrl.page->component(sessionConfigComponent, 6, 1);
    
  } else if (subpage == 1) {

    uCtrl.oled->print("track config", 3, 1);
    uCtrl.page->component(trackOutputSelectorComponent, 4, 1, true);
    uCtrl.page->component(midiChannelConfigComponent, 4, 2);
    
    uCtrl.oled->print("system resources", 6, 1);
    uCtrl.page->component(systemResourcesComponent, 7, 1);

    //uCtrl.oled->print("Micro Controlled", 8, 10);
    
  }
}

void system_page_digital_input(uint8_t control_id, uint16_t value, uint8_t subpage)
{ 
  switch (control_id) {
    
  }   
}

void system_page_analog_input(uint8_t control_id, uint16_t value, uint8_t subpage)
{
  // special fixed pots for interaction like generative ones
}
