
void step_sequencer_page_create()
{
  uCtrl.dout->writeAll(LOW); 
}

void step_sequencer_page_destroy()
{
  uCtrl.dout->writeAll(LOW);
}

// called each cycle interaction of interface object for UI refresh
void step_sequencer_page_refresh(uint8_t subpage)
{
  uCtrl.page->component(topBarComponent, 1, 1);

  if (subpage == 0) {
    // set step sequencer mode view as small
    stepSequencerComponent.setViewMode(false);
    // line 3. 2 grids
    uCtrl.page->component(lengthComponent, 3, 1, true);
    uCtrl.page->component(shiftComponent, 3, 2);
    // time signature: off(step lenght ate classic mode), 1/3(length goes to size of signature block)
    
    // add div control? 32, 16, 8
    //uCtrl.page->component(typeComponent, 4, 1);

    // sequence variation A/B
    //uCtrl.page->component(variationComponent, 4, 1);
    
    if (aciduino.seq.is303(aciduino.getSelectedTrack())) {
      // randomizer type: classic, bjulrk
      uCtrl.page->component(tuneComponent, 4, 1);
      uCtrl.page->component(transposeComponent, 4, 2);
    } else {
      // 808
      // voice init point.
      // voice config note for midi, port for cv
      // note(selected voice)/port(eurorack)
      //uCtrl.page->component(voiceConfigComponent, 4, 2);
      //uCtrl.page->component(voiceSelectComponent, 4, 1);
      uCtrl.page->component(voiceConfigComponent, 4, 1);
      uCtrl.page->component(rollTypeComponent, 4, 2);
    }
    
    // step edit
    uCtrl.page->component(stepSequencerComponent, 5, 1);
  } else if (subpage == 1) {
    // set step sequencer mode view as full size view
    stepSequencerComponent.setViewMode(true);

    // step edit full view mode
    uCtrl.page->component(stepSequencerComponent, 2, 1, true);
  }
  
}

void step_sequencer_page_digital_input(uint8_t control_id, uint16_t value, uint8_t subpage)
{  
#if defined(USE_PUSH_24) || defined(USE_PUSH_32) || defined(USE_TOUCH_32)
  // step array buttons handling
  if (value == LOW) return;
  
  // step array buttons handling?
  if (control_id >= SELECTOR_BUTTON_1 && control_id <= SELECTOR_BUTTON_8) {
    // change selector
    stepSequencerComponent.selectorButton(control_id-SELECTOR_BUTTON_1);
  }
  if (control_id >= STEP_BUTTON_1 && control_id <= STEP_BUTTON_16) {
    // select step, on/off
    stepSequencerComponent.stepButton(control_id-STEP_BUTTON_1);
  }
#endif
}

void step_sequencer_page_analog_input(uint8_t control_id, uint16_t value, uint8_t subpage)
{
  // special fixed pots for interaction like generative ones
}

void step_sequencer_page_init()
{
  //uCtrl.page->set("seqr", step_sequencer_page_create, step_sequencer_page_destroy, step_sequencer_page_refresh, step_sequencer_page_digital_input, step_sequencer_page_analog_input, 2);   
  uCtrl.page->set("seqr", step_sequencer_page_create, step_sequencer_page_destroy, step_sequencer_page_refresh, step_sequencer_page_digital_input, nullptr, 2);     
}
