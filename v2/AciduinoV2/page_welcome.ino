uint8_t _select_page_menu = 1;
bool _powersave = false;

#if defined(TEENSYDUINO)
extern "C" char* sbrk(int incr);
int freeRam() {
  char top;
  return &top - reinterpret_cast<char*>(sbrk(0));
}
#elif defined(ARDUINO_ARCH_ESP32) || defined(ESP32)
int freeRam () 
{
  return ESP.getFreeHeap(); 
}
#elif defined(ARDUINO_ARCH_AVR)
int freeRam () 
{
  extern int __heap_start, *__brkval; 
  int v; 
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval); 
}
#endif

void welcome_page_init()
{ 
  uCtrl.page->set("", welcome_page_create, welcome_page_destroy, welcome_page_refresh, welcome_page_digital_input);    
}

void welcome_page_create()
{
  uCtrl.dout->writeAll(LOW);
}

void welcome_page_destroy()
{
  uCtrl.dout->writeAll(LOW);
}

// called each cycle interaction of interface object for UI refresh
void welcome_page_refresh(uint8_t subpage)
{ 
  uCtrl.oled->print("# aciduino v2", 1, 1);   

  // debug stack/heap memory
  uCtrl.oled->print("free ram: ", 3, 1);  
  uCtrl.oled->print(String(freeRam()), 3, 11); 

  uCtrl.oled->print("Micro Controlled", 8, 9); 
}

void welcome_page_digital_input(uint8_t control_id, uint16_t value, uint8_t subpage)
{ 
  switch (control_id) {
    
  }   
}
