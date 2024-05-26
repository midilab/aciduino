struct SessionConfig : PageComponent {
  
    SessionConfig() {
      grid_size = 2;    
    }

    void view() {
      genericOptionView("session", String("epprom"), line, col, selected, true);
      setF1("load");
      setF2("save");
    }
    
    void change(int16_t data) {
    }

    void function1() {
      aciduino.loadSession();
    }

    void function2() {
      aciduino.saveSession();
    }

} sessionConfigComponent;