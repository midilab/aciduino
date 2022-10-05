void step_sequencer_page_init()
{
  uCtrl.page->set("seq", step_sequencer_page_create, step_sequencer_page_destroy, step_sequencer_page_refresh, step_sequencer_page_digital_input, step_sequencer_page_analog_input, 2);     
}

void step_sequencer_page_create()
{
  //uCtrl.dout->writeAll(LOW); 
}

void step_sequencer_page_destroy()
{
  //uCtrl.dout->writeAll(LOW);
}

// called each cycle interaction of interface object for UI refresh
void step_sequencer_page_refresh(uint8_t subpage)
{
  uCtrl.page->component(topBarComponent, 1, 1);

  if (subpage == 0) {
    // line 3. 2 grids
    uCtrl.page->component(lengthComponent, 3, 1);
    uCtrl.page->component(shiftComponent, 3, 2);
    // time signature: off(step lenght ate classic mode), 1/3(length goes to size of signature block)
    
    // add div control? 32, 16, 8
    //uCtrl.page->component(typeComponent, 4, 1);

    // sequence variation A/B
    uCtrl.page->component(variationComponent, 4, 1);
    
    if (AcidSequencer.is303(_selected_track)) {
      // randomizer type: classic, bjulrk
      uCtrl.page->component(tuneComponent, 4, 2);
    } else {
      // 808
      // voice init point.
      // voice config note for midi, port for cv
      // note(selected voice)/port(eurorack)
      //uCtrl.page->component(voiceConfigComponent, 4, 2);
      //uCtrl.page->component(voiceSelectComponent, 4, 1);
      uCtrl.page->component(voiceConfigComponent, 4, 2);
    }

    // seq divider!
    
    // step edit
    uCtrl.page->component(stepSequencerComponent, 5, 1);
  } else if (subpage == 1) {
    // generative
    // ...
    // 303
    // low range, high range
    // tones num
    // notes num

    // 808
    
    // _bjorklund size(length?), _bjorklund (fill)
    // 

    uCtrl.page->component(stepSequencerComponent, 3, 1);
  }
  
}

void step_sequencer_page_digital_input(uint8_t control_id, uint16_t value, uint8_t subpage)
{  
  // step array buttons handling?
}

void step_sequencer_page_analog_input(uint8_t control_id, uint16_t value, uint8_t subpage)
{
  // special fixed pots for interaction like generative ones
}
