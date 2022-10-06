
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
#ifdef MIDI_CONTROLER
// for 303 roland cloud
MidiCCControl filterComponent("filter", 74, TRACK_NUMBER_303), 
              resonanceComponent("reso.", 71, TRACK_NUMBER_303), 
              envelopeComponent("env mod", 12, TRACK_NUMBER_303), 
              decayComponent("decay", 75, TRACK_NUMBER_303), 
              accentComponent("accent", 16, TRACK_NUMBER_303);

/*
1   0x01    VCF BEND DOWN
11  0x0B    MASTER LEVEL
17  0x11    DRIVE DEPTH
18  0x12    DELAY TIME
19  0x13    DELAY LEVEL
64  0x40    HOLD PEDAL
104 0x68 TUNING
*/

// for 808 roland cloud
MidiCCControl bdDecayComponent("bd decay", 23, TRACK_NUMBER_808), 
              bdToneComponent("bd tone", 21, TRACK_NUMBER_808), 
              snSnappyComponent("sd snap", 26, TRACK_NUMBER_808), 
              snToneComponent("sd tone", 25, TRACK_NUMBER_808);
/*
20  BD TUNE
21  BD TONE
22  BD COMP
23  BD DECAY
24  BD LEVEL 

 */
#endif

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

// called each cycle interaction of interface object for UI refresh
void live_page_refresh(uint8_t subpage)
{
  uCtrl.page->component(topBarComponent, 1, 1);

  // all elements hooked to change sequence A/B

  if (subpage == 0) {
    // add div control? 32, 16, 8

  } else if (subpage == 1) {
    // pattern control subpage
#ifdef MIDI_CONTROLER
    if (AcidSequencer.is303(_selected_track)) {
      // midi control subpage?
      uCtrl.page->component(filterComponent, 3, 1, true);
      uCtrl.page->component(resonanceComponent, 3, 2);
  
      uCtrl.page->component(envelopeComponent, 4, 1);
      uCtrl.page->component(decayComponent, 4, 2);
  
      uCtrl.page->component(accentComponent, 5, 1);
    } else {
      uCtrl.page->component(bdDecayComponent, 3, 1, true);
      uCtrl.page->component(bdToneComponent, 3, 2);
  
      uCtrl.page->component(snSnappyComponent, 4, 1);
      uCtrl.page->component(snToneComponent, 4, 2);
    }
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
