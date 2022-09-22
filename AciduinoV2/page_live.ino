void live_page_init()
{
  uCtrl.page->set("live", live_page_create, live_page_destroy, live_page_refresh, live_page_digital_input, live_page_analog_input, 2);     
}

void live_page_create()
{
  //uCtrl.dout->writeAll(LOW); 
}

void live_page_destroy()
{
  //uCtrl.dout->writeAll(LOW);
}

// +++ live page
// patterns 
// controllers

// +++ track config
// Module: MIDI|CV
// MIDI Module
// Port: 1 (hardware port interface)
// Channel: 1-16
// CV Module
// Note: 1(CV port)
// Trigger: 1(triger port)
// Accent: 2(triger port)
// Slide: 3(triger port)

// called each cycle interaction of interface object for UI refresh
void live_page_refresh(uint8_t subpage)
{
  uCtrl.page->component(topBarComponent, 1, 1);

  if (subpage == 0) {
    // add div control? 32, 16, 8
    //uCtrl.page->component(typeComponent, 4, 1);
    //uCtrl.page->component(scaleComponent, 4, 2);
  } else if (subpage == 1) {
    
  }
  
}

void live_page_digital_input(uint8_t control_id, uint16_t value, uint8_t subpage)
{  
  // step array buttons handling?
}

void live_page_analog_input(uint8_t control_id, uint16_t value, uint8_t subpage)
{
  // special fixed pots for interaction like generative ones
}
