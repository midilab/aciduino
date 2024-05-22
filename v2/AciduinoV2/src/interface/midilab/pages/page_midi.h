
void midi_page_create()
{
  // lets hook the components f1 anf f2 functions for generative page only
  //
  //uCtrl.page->setFunctionHook(f1_page, f2_page, generativeFunction1, generativeFunction2);
}

void midi_page_destroy()
{
  // clear our hook
  //uCtrl.page->clearFunctionHook();
}

// called each cycle interaction of interface object for UI refresh
void midi_page_refresh(uint8_t subpage)
{
  uCtrl.page->component(topBarComponent, 1, 1);

  if (subpage == 0) {
    uCtrl.page->component(midiControllerComponent, 3, 1, true);
  } else if (subpage == 1) {
    // midi efx subpage. LFOs and other goodies. soon...
  }
}

void midi_page_digital_input(uint8_t control_id, uint16_t value, uint8_t subpage)
{  
  // step array buttons handling?
}

void midi_page_analog_input(uint8_t control_id, uint16_t value, uint8_t subpage)
{
  // special fixed pots for interaction like generative ones
}

void midi_page_init()
{
  //uCtrl.page->set("midi", midi_page_create, midi_page_destroy, midi_page_refresh, midi_page_digital_input, midi_page_analog_input, 2);   
  uCtrl.page->set("midi", midi_page_create, midi_page_destroy, midi_page_refresh, midi_page_digital_input, nullptr, 1);       

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
    midiControllerComponent.set303Control("efx 1", 105);
    midiControllerComponent.set303Control("efx 2", 106);
    midiControllerComponent.set303Control("efx 3", 107);
    midiControllerComponent.set303Control("efx 4", 108);
    midiControllerComponent.set303Control("efx 5", 109);
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
