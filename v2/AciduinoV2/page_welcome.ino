// PS: move it to controller page
//
// MIDI Controller config
//
#define USE_MIDI_CTRL         
#define MIDI_CTRL_TUNNING     79
#define MIDI_CTRL_CUTOFF      80
#define MIDI_CTRL_RESONANCE   81
#define MIDI_CTRL_ENVMOD      82
#define MIDI_CTRL_DECAY       83
#define MIDI_CTRL_ACCENT      84
#define MIDI_CTRL_WAVE        85 

uint8_t _select_page_menu = 1;
bool _powersave = false;

extern "C" char* sbrk(int incr);
int freeram() {
  char top;
  return &top - reinterpret_cast<char*>(sbrk(0));
}

void welcome_page_init()
{ 
  uCtrl.page->set("", welcome_page_create, welcome_page_destroy, welcome_page_refresh, welcome_page_digital_input);    
}

void welcome_page_create()
{

}

void welcome_page_destroy()
{
  
}

// called each cycle interaction of interface object for UI refresh
void welcome_page_refresh(uint8_t subpage)
{ 
  uCtrl.oled->print("# aciduino v2", 1, 1);   

  // debug stack/heap memory
  uCtrl.oled->print("free ram: ", 3, 1);  
  uCtrl.oled->print(String(freeram()), 3, 11); 

  uCtrl.oled->print("Micro Controlled", 8, 9); 
}

void welcome_page_digital_input(uint8_t control_id, uint16_t value, uint8_t subpage)
{ 
  switch (control_id) {
    
  }   
}
