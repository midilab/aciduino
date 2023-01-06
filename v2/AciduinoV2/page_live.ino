
void live_page_init()
{
  uCtrl.page->set("live", live_page_create, live_page_destroy, live_page_refresh, live_page_digital_input, live_page_analog_input, 2);
  // register midi controller options per track
  if (TRACK_NUMBER_303 > 0) {
    // max 16 controls!
    midiControllerComponent.set303Control("filter", 74);
    midiControllerComponent.set303Control("reso.", 71);
    midiControllerComponent.set303Control("env mod", 12);
    midiControllerComponent.set303Control("decay", 75);
    midiControllerComponent.set303Control("accent", 16);
    midiControllerComponent.set303Control("vcf bd.", 1);
    midiControllerComponent.set303Control("drive", 17);
    midiControllerComponent.set303Control("volume", 11);

    midiControllerComponent.set303Control("delay t.", 18);
    midiControllerComponent.set303Control("delay l.", 19);
    midiControllerComponent.set303Control("tuning", 104);
    //midiControllerComponent.set303Control("...", 1);
    //midiControllerComponent.set303Control("...", 1);
    //midiControllerComponent.set303Control("...", 1);
    //midiControllerComponent.set303Control("...", 1);
    //midiControllerComponent.set303Control("...", 1);
  }
  if (TRACK_NUMBER_808 > 0) {
    // max 16 controls!
    midiControllerComponent.set808Control("bd decay", 23);
    midiControllerComponent.set808Control("bd tone", 21);
    midiControllerComponent.set808Control("sd snap", 26);
    midiControllerComponent.set808Control("sd tone", 25);
    midiControllerComponent.set808Control("lt tune", 46);
    midiControllerComponent.set808Control("mt tune", 49);
    midiControllerComponent.set808Control("ht tune", 52);
    midiControllerComponent.set808Control("rs tune", 55);

    midiControllerComponent.set808Control("bd level", 24);
    midiControllerComponent.set808Control("sd level", 29);
    midiControllerComponent.set808Control("ch level", 63);
    midiControllerComponent.set808Control("oh level", 82);
    midiControllerComponent.set808Control("lt level", 48);
    midiControllerComponent.set808Control("mt level", 51);
    midiControllerComponent.set808Control("ht level", 54);
    midiControllerComponent.set808Control("rs level", 57);
  }  
}

void live_page_create()
{
  uCtrl.dout->writeAll(LOW); 
}

void live_page_destroy()
{
  uCtrl.dout->writeAll(LOW);
}

// called each cycle interaction of interface object for UI refresh
void live_page_refresh(uint8_t subpage)
{
  uCtrl.page->component(topBarComponent, 1, 1);

  if (subpage == 0) {
    uCtrl.page->component(mutePatternComponent, 3, 1, true);
  } else if (subpage == 1) {
    uCtrl.page->component(midiControllerComponent, 3, 1, true);
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
