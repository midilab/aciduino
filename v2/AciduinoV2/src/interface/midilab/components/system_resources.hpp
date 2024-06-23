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
          genericOptionView("epprom", String(aciduino.getStorageSize()), line, col, selected, true);
          break;
        case 2:
          genericOptionView("session", String(aciduino.getSessionSize()), line, col, selected, true);
          break;
        case 3:
          genericOptionView("pattern", String(aciduino.getNumOfPatterns()), line, col, selected, true);
          break;
      }
      
    }
    
    void change(int16_t data) {
      data = parseData(data, 0, 3, resource_id);
      resource_id = data;
    }
    
} systemResourcesComponent;