struct SystemResources : PageComponent {

    uint8_t resource_id = 0;

    SystemResources() {
      grid_size = 2;    
    }

    void view() {

      switch(resource_id) {
        case 0:
          genericOptionView("free ram", String(aciduino.freeRam()), line, col, selected, true);
          break;
        case 1:
          genericOptionView("epprom", String(EPPROM_SIZE), line, col, selected, true);
          break;
        case 2:
          //genericOptionView("session", String(String((EPPROM_SESSION_SIZE/(float)EEPROM.length())*100) + String("%")), line, col, selected, true);
          genericOptionView("session", String(EPPROM_SESSION_SIZE), line, col, selected, true);
          break;
        case 3:
          genericOptionView("pattern", String(EPRROM_PATTERN_AVAILABLE), line, col, selected, true);
          break;
      }
      
    }
    
    void change(int16_t data) {
      data = parseData(data, 0, 3, resource_id);
      resource_id = data;
    }
    
} systemResourcesComponent;