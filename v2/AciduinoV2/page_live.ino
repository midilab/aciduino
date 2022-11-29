
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

// midi controls as parameters to select

/* 
roland cloud tb303 midi cc messages
CC  Address Parameter
1   0x01    VCF BEND DOWN
11  0x0B    MASTER LEVEL
12  0x0C    ENV MOD
16  0x10    ACCENT
17  0x11    DRIVE DEPTH
18  0x12    DELAY TIME
19  0x13    DELAY LEVEL
64  0x40    HOLD PEDAL
71  0x47    RESONANCE
74  0x4A    CUT OFF FREQ
75  0x4B    DECAY
104 0x68 TUNING

roland cloud tr808 midi cc messages
CC# Parameter
9   SHUFFLE
20  BD TUNE
21  BD TONE
22  BD COMP
23  BD DECAY
24  BD LEVEL
25  SD TONE
26  SD SNAPPY
27  SD COMP
28  SD DECAY
29  SD LEVEL
46  LT TUNE
47  LT DECAY
48  LT LEVEL
49  MT TUNE
50  MT DECAY
51  MT LEVEL
52  HT TUNE
53  HT DECAY
54  HT LEVEL
55  RS TUNE
56  RS DECAY
57  RS LEVEL
58  CP TUNE
59  CP DECAY
60  CP LEVEL
61  CH TUNE
62  CH DECAY
63  CH LEVEL
71  TOTAL ACCENT
80  OH TUNE
81  OH DECAY
82  OH LEVEL
83  CY TONE
84  CY DECAY
85  CY LEVEL
86  CB TUNE
87  CB DECAY
88  SCB LEVEL

roland cloud tr909 midi cc messages
CC  Parameter
9   SHUFFLE
20  BD TUNE
21  BD TONE
22  BD COMP
23  BD DECAY
24  BD LEVEL
25  SD TONE
26  SD SNAPPY
27  SD COMP
28  SD DECAY
29  SD LEVEL
46  LT TUNE
47  LT DECAY
48  LT LEVEL
49  MT TUNE
50  MT DECAY
51  MT LEVEL
52  HT TUNE
53  HT DECAY
54  HT LEVEL
55  RS TUNE
56  RS DECAY
57  RS LEVEL
58  CP TUNE
59  CP DECAY
60  CP LEVEL
61  CH TUNE
62  CH DECAY
63  CH LEVEL
71  TOTAL ACCENT
80  OH TUNE
81  OH DECAY
82  OH LEVEL
83  CC TONE
84  CC DECAY
85  CC LEVEL
86  RC TUNE
87  RC DECAY
88  RC LEVEL
*/

void live_page_init()
{
#ifdef MIDI_CONTROLER
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
#else
  // one subpage for now until we get the pattern feature setup. ableton grid like patterns select screen     
  uCtrl.page->set("live", live_page_create, live_page_destroy, live_page_refresh, live_page_digital_input, live_page_analog_input, 1);  
#endif   
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

  // all elements hooked to change sequence A/B

  if (subpage == 0) {
    

  } else if (subpage == 1) {
    // pattern control subpage
#ifdef MIDI_CONTROLER
    uCtrl.page->component(midiControllerComponent, 3, 1, true);
#endif
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
