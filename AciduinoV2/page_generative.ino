const char * f1_page = "undo";
const char * f2_page = "generate";

//GeneralDataControl fillComponent("fill", &_generative_fill, 0, 100)

void generativeFunction1()
{
  // undo new generated sequence
  
}

void generativeFunction2()
{
  // generate new sequence
  if (AcidSequencer.is303(_selected_track)) {
    AcidSequencer.acidRandomize(_selected_track, _generative_fill, _accent_probability, _slide_probability, /* tie_probability */ _tie_probability, _number_of_tones, _lower_note, _range_note);
  } else {
    AcidSequencer.acidRandomize(_selected_track, _generative_fill, _accent_probability, _roll_probability);
  }
}

void generative_page_init()
{
  uCtrl.page->set("gen", generative_page_create, generative_page_destroy, generative_page_refresh, generative_page_digital_input, generative_page_analog_input, 2);     
}

void generative_page_create()
{
  // lets hook the components f1 anf f2 functions for generative page only
  //
  uCtrl.page->setFunctionHook(f1_page, f2_page, generativeFunction1, generativeFunction2);
}

void generative_page_destroy()
{
  // clear our hook
  uCtrl.page->clearFunctionHook();
}

// called each cycle interaction of interface object for UI refresh
void generative_page_refresh(uint8_t subpage)
{
  uCtrl.page->component(topBarComponent, 1, 1);

  if (subpage == 0) {
    uCtrl.page->component(lengthComponent, 3, 1, true);
    uCtrl.page->component(shiftComponent, 3, 2);
    // time signature: off(step lenght ate classic mode), 1/3(length goes to size of signature block)

    // sequence variation A/B
    //uCtrl.page->component(variationComponent, 4, 1);
    
    // for 808(fill): with common options. 4/4, tripplets...
    uCtrl.page->component(fillComponent, 6, 1);
      
    if (AcidSequencer.is303(_selected_track)) {
      // randomizer type: classic, bjulrk
      uCtrl.page->component(tuneComponent, 4, 2);
    } else {
      // 808
      // voice init point.
      // voice config note for midi, port for cv
      // note(selected voice)/port(eurorack)
      uCtrl.page->component(voiceSelectComponent, 4, 2);

    }
  } else if (subpage == 1) {
    // generative
    // use classic algo random or euclidian for 303 generator

    // classical vs euclidian
    // 

    uCtrl.page->component(accentAmountComponent, 3, 1, true);
    
    if (AcidSequencer.is303(_selected_track)) {
      uCtrl.page->component(slideAmountComponent, 3, 2);
      uCtrl.page->component(lowRangeComponent, 4, 1);
      uCtrl.page->component(highRangeComponent, 4, 2);
      uCtrl.page->component(tonesNumberComponent, 5, 1);
      // for 303(scale)
      uCtrl.page->component(scaleComponent, 6, 1);
    } else {
      uCtrl.page->component(rollAmountComponent, 3, 2);
    }

  }

}

void generative_page_digital_input(uint8_t control_id, uint16_t value, uint8_t subpage)
{  
  // step array buttons handling?
}

void generative_page_analog_input(uint8_t control_id, uint16_t value, uint8_t subpage)
{
  // special fixed pots for interaction like generative ones
}
