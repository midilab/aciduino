const char * f1_page = "clear";
const char * f2_page = "generate";

void generativeFunction1()
{
  // clear track
  AcidSequencer.clearTrack(_selected_track);
}

void generativeFunction2()
{ 
  // generate new sequence
  if (AcidSequencer.is303(_selected_track)) {
    AcidSequencer.acidRandomize(_selected_track, _generative_303[_selected_track].generative_fill, _generative_303[_selected_track].accent_probability, _generative_303[_selected_track].slide_probability, _generative_303[_selected_track].tie_probability, _generative_303[_selected_track].number_of_tones, _generative_303[_selected_track].lower_octave*12, _generative_303[_selected_track].range_octave*12);
  } else {
    AcidSequencer.acidRandomize(_selected_track, _generative_808[_selected_track-TRACK_NUMBER_303].generative_fill, _generative_808[_selected_track-TRACK_NUMBER_303].accent_probability, _generative_808[_selected_track-TRACK_NUMBER_303].roll_probability);
  }
}

void generative_page_init()
{
  //uCtrl.page->set("gene", generative_page_create, generative_page_destroy, generative_page_refresh, generative_page_digital_input, generative_page_analog_input, 2);   
  uCtrl.page->set("gene", generative_page_create, generative_page_destroy, generative_page_refresh, generative_page_digital_input, nullptr, 2);     
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


struct TonesNumber : PageComponent {

    void view() {
      genericOptionView("tones", String(_generative_303[_selected_track].number_of_tones), line, col, selected);
    }

    void change(int16_t data) {
      data = parseData(data, 1, 7, _generative_303[_selected_track].number_of_tones);
      _generative_303[_selected_track].number_of_tones = data;
    }
    
} tonesNumberComponent;

struct LowOctave : PageComponent {
  
    void view() {
      genericOptionView("octave", String(_generative_303[_selected_track].lower_octave), line, col, selected);
    }

    void change(int16_t data) {
      data = parseData(data, 1, 11, _generative_303[_selected_track].lower_octave);
      _generative_303[_selected_track].lower_octave = data;
    }
    
} lowOctaveComponent;

struct RangeOctave : PageComponent {
  
    void view() {
      genericOptionView("octaves", String(_generative_303[_selected_track].range_octave), line, col, selected);
    }

    void change(int16_t data) {
      data = parseData(data, 1, 11, _generative_303[_selected_track].range_octave);
      _generative_303[_selected_track].range_octave = data;
    }
    
} rangeOctaveComponent;

struct AccentAmount : PageComponent {
  
    void view() {
      uint8_t accent_probability = AcidSequencer.is303(_selected_track) ? _generative_303[_selected_track].accent_probability : _generative_808[_selected_track-TRACK_NUMBER_303].accent_probability; 
      genericOptionView("accent", String(accent_probability), line, col, selected);
    }

    void change(int16_t data) {
      uint8_t * accent_probability = AcidSequencer.is303(_selected_track) ? &_generative_303[_selected_track].accent_probability : &_generative_808[_selected_track-TRACK_NUMBER_303].accent_probability; 
      data = parseData(data, 0, 100, *accent_probability);
      *accent_probability = data;
    }

} accentAmountComponent;

struct SlideAmount : PageComponent {
  
    void view() {
      genericOptionView("slide", String(_generative_303[_selected_track].slide_probability), line, col, selected);
    }

    void change(int16_t data) {
      data = parseData(data, 0, 100, _generative_303[_selected_track].slide_probability);
      _generative_303[_selected_track].slide_probability = data;
    }
    
} slideAmountComponent;

struct TieAmount : PageComponent {
  
    void view() {
      genericOptionView("tie", String(_generative_303[_selected_track].tie_probability), line, col, selected);
    }

    void change(int16_t data) {
      data = parseData(data, 0, 100, _generative_303[_selected_track].tie_probability);
      _generative_303[_selected_track].tie_probability = data;
    }
    
} tieAmountComponent;

struct RollAmount : PageComponent {
  
    void view() {
      genericOptionView("roll", String(_generative_808[_selected_track-TRACK_NUMBER_303].roll_probability), line, col, selected);
    }

    void change(int16_t data) {
      data = parseData(data, 0, 100, _generative_808[_selected_track-TRACK_NUMBER_303].roll_probability);
      _generative_808[_selected_track-TRACK_NUMBER_303].roll_probability = data;
    }
    
} rollAmountComponent;

struct TrackScale : PageComponent {

    TrackScale() {
      // we want this component to be 2 grids navigable object
      grid_size = 2;
    }
    
    void view() {
      genericOptionView("scale", String(AcidSequencer.getTemperamentName(AcidSequencer.getTemperamentId())), line, col, selected, true);
    }

    void change(int16_t data) {
      data = parseData(data, 0, 13, AcidSequencer.getTemperamentId());
      AcidSequencer.setTemperament(data);
    }

} scaleComponent;

struct TrackFill : PageComponent {

    TrackFill() {
      // we want this component to be 2 grids navigable object
      grid_size = 2;
    }
    
    void view() {
      uint8_t generative_fill = AcidSequencer.is303(_selected_track) ? _generative_303[_selected_track].generative_fill : _generative_808[_selected_track-TRACK_NUMBER_303].generative_fill; 
      genericOptionView("fill", String(generative_fill), line, col, selected, true);
    }

    void change(int16_t data) {
      //clearStackNote(_selected_track);
      uint8_t * generative_fill = AcidSequencer.is303(_selected_track) ? &_generative_303[_selected_track].generative_fill : &_generative_808[_selected_track-TRACK_NUMBER_303].generative_fill; 
      data = parseData(data, 1, 100, *generative_fill);
      *generative_fill = data;
    }
    
} fillComponent;

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
      uCtrl.page->component(tuneComponent, 4, 1);
      uCtrl.page->component(transposeComponent, 4, 2);
    } else {
      // 808
      // voice init point.
      // voice config note for midi, port for cv
      // note(selected voice)/port(eurorack)
      uCtrl.page->component(voiceSelectComponent, 4, 1);
      uCtrl.page->component(rollTypeComponent, 4, 2);
    }
  } else if (subpage == 1) {
    // generative
    // use classic algo random or euclidian for 303 generator

    // classical vs euclidian
    // 

    uCtrl.page->component(accentAmountComponent, 3, 1, true);
    
    if (AcidSequencer.is303(_selected_track)) {
      uCtrl.page->component(slideAmountComponent, 3, 2);
      
      uCtrl.page->component(tieAmountComponent, 4, 1);
      uCtrl.page->component(tonesNumberComponent, 4, 2);
      
      uCtrl.page->component(lowOctaveComponent, 5, 1);
      uCtrl.page->component(rangeOctaveComponent, 5, 2);
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
