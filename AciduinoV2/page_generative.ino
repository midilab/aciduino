const char * f1_page = "undo";
const char * f2_page = "generate";

void generativeFunction1()
{
  // undo new generated sequence
  
}

void generativeFunction2()
{
  // generate new sequence
  AcidSequencer.acidRandomize(_selected_track, _generative_fill);
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
    // line 3. 2 grids
    uCtrl.page->component(lengthComponent, 3, 1);
    uCtrl.page->component(shiftComponent, 3, 2);
    // time signature: off(step lenght ate classic mode), 1/3(length goes to size of signature block)

    // sequence variation A/B
    uCtrl.page->component(variationComponent, 4, 1);
    
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
    // ...
    // 303
    // low range, high range
    // tones num, notes num

    // 808
    
    // _bjorklund size(length?), _bjorklund (fill)
    // 

    // use classic algo random or euclidian for 303 generator

    // classical vs euclidian
    // 

    // for booth or individual
    // 303 on traditional randomizer
    //#define REST_PROBABILITY_GENERATION_303     10
    // 303 on euclidian rest generator
    // fill
    //#define ACCENT_PROBABILITY_GENERATION_303   50
    uCtrl.page->component(accentAmountComponent, 3, 1);
    if (AcidSequencer.is303(_selected_track)) {
      //#define SLIDE_PROBABILITY_GENERATION_303    30
      uCtrl.page->component(slideAmountComponent, 3, 2);
      uCtrl.page->component(tonesNumberComponent, 4, 1);
      uCtrl.page->component(lowRangeComponent, 5, 1);
      uCtrl.page->component(highRangeComponent, 5, 2);
      // for 303(scale)
      uCtrl.page->component(scaleComponent, 6, 1);
    } else {
      //#define ROLL_PROBABILITY_GENERATION_303    30
      uCtrl.page->component(rollAmountComponent, 3, 2);
    }

/*
    if (AcidSequencer.is303(_selected_track)) {
      // randomizer type: classic, bjulrk
      uCtrl.page->component(tuneComponent, 3, 2);
    } else {
      // 808
      // voice init point.
      // voice config note for midi, port for cv
      // note(selected voice)/port(eurorack)
      uCtrl.page->component(voiceConfigComponent, 3, 1);
    }
*/
    //uCtrl.page->component(stepSequencerComponent, 5, 1);
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
