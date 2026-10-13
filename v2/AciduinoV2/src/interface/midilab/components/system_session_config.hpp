struct SessionConfig : PageComponent {

    uint16_t selected_session = 0;

    SessionConfig() {
      grid_size = 2;    
    }

    void view() {
      if (!uCtrl.storage->isSdActive()) {
        genericOptionView("session", String("epprom"), line, col, selected, true);
        setF1("load");
        setF2("save");
        return;
      }

      // last slot is always the new session option
      uint16_t num_sessions = aciduino.getNumOfSessions();
      if (selected_session > num_sessions) {
        selected_session = num_sessions;
      }

      if (selected_session == num_sessions) {
        genericOptionView("session", String("new"), line, col, selected, true);
        setF1("fork");
      } else {
        char session_label[8];
        snprintf(session_label, sizeof(session_label), "s%03u", selected_session);
        genericOptionView("session", String(session_label), line, col, selected, true);
        setF1("load");
      }
      setF2("save");
    }
    
    void change(int16_t data) {
      if (!uCtrl.storage->isSdActive()) {
        return;
      }
      data = parseData(data, 0, aciduino.getNumOfSessions(), selected_session);
      selected_session = data;
    }

    void function1() {
      if (!uCtrl.storage->isSdActive()) {
        aciduino.loadSession();
        return;
      }
      // fork current state into a new session slot
      if (selected_session == aciduino.getNumOfSessions()) {
        if (aciduino.newSession()) {
          selected_session = aciduino.getActiveSession();
        }
        return;
      }
      // load selected session slot
      if (aciduino.selectSession(selected_session)) {
        selected_session = aciduino.getActiveSession();
      }
    }

    void function2() {
      aciduino.saveSession();
    }

} sessionConfigComponent;
