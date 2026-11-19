// SYS page 2 track management: delete the selected track or create a new one.
// The confirm screen is a blocking modal, same event handling style as the
// sdcard format dialog at boot.

struct TrackTypeSelector : PageComponent {

  uint8_t index = 0;

  TrackTypeSelector() {
    // full row and 2 lines high(one line of padding above it in the dialog)
    line_size = 2;
    grid_size = 2;
  }

  const char * label() {
    if (index == 0) return "303";
    if (index == 1) return "808";
    return "POLY";
  }

  uint8_t type() {
    if (index == 0) return SEQ_TRACK_303;
    if (index == 1) return SEQ_TRACK_808;
    return SEQ_TRACK_POLY;
  }

  void view() {
    genericOptionView("type", String(label()), line, col, selected, true);
  }

  void change(int16_t data) {
    index = parseData(data, 0, 2, index);
  }
};

struct TrackManagement : PageComponent {

  // 0 = delete, 1 = new
  uint8_t action = 0;

  TrackManagement() {
    grid_size = 2;
  }

  void view() {
    genericOptionView("track mngmt", String(action == 0 ? "delete" : "new"), line, col, selected, true);
    setF1("");
    setF2("confirm");
  }

  void change(int16_t data) {
    action = parseData(data, 0, 1, action);
  }

  void function2() {
    if (action == 0) {
      if (confirm("are you sure you want", "to delete this track?", nullptr)) {
        uint8_t track = aciduino.getSelectedTrack();
        if (aciduino.deleteTrack(track)) {
          midiControllerComponent.resetTrack(track);
        }
      }
      return;
    }

    uint8_t type = SEQ_TRACK_303;
    if (!confirm("are you sure you want", "to create a new track?", &type)) {
      return;
    }
    uint8_t slot = aciduino.addTrack(type);
    if (slot == 0xFF) {
      uCtrl.oled->clearDisplay();
      uCtrl.oled->print("no free slot", 4, 1);
      uCtrl.oled->refreshDisplay();
      delay(800);
      return;
    }
    midiControllerComponent.resetTrack(slot);
    aciduino.setSelectedTrack(slot);
  }

  // blocking yes/no modal. type != nullptr shows the 303/808/POLY selector,
  // changed by the same inc/dec schema the page components use(also accepts
  // the nav pot and the secondary encoder ports when the build has them).
  bool confirm(const char *line1, const char *line2, uint8_t *type) {
    TrackTypeSelector selector;
    selector.line = 5;
    selector.col = 1;
    selector.selected = true;

    bool answered = false;
    bool yes = false;
    bool redraw = true;

    while (!answered) {
      if (redraw) {
        uCtrl.oled->clearDisplay();
        uCtrl.oled->print(line1, 2, 1);
        uCtrl.oled->print(line2, 3, 1);
        if (type != nullptr) {
          selector.view();
        }
        functionDrawCallback("no", "yes", 0, 0);
        uCtrl.oled->refreshDisplay();
        redraw = false;
      }

#if defined(USE_DIN_MODULE)
      if (uCtrl.din != nullptr) {
        while (uCtrl.din->event_queue.head != uCtrl.din->event_queue.tail) {
          uint8_t port = uCtrl.din->event_queue.event[uCtrl.din->event_queue.head].port;
          uint16_t value = uCtrl.din->event_queue.event[uCtrl.din->event_queue.head].value;
          uCtrl.din->event_queue.head = (uCtrl.din->event_queue.head + 1) % uCtrl.din->event_queue.size;
          if (value == LOW) {
            continue;
          }
          if (port == uCtrl.page->_nav_ctrl_port.function1) {
            answered = true;
          } else if (port == uCtrl.page->_nav_ctrl_port.function2) {
            answered = true;
            yes = true;
          } else if (type != nullptr) {
            if (port == uCtrl.page->_nav_ctrl_port.incrementer || port == uCtrl.page->_nav_ctrl_port.incrementer_secondary) {
              selector.change(INCREMENT);
              redraw = true;
            } else if (port == uCtrl.page->_nav_ctrl_port.decrementer || port == uCtrl.page->_nav_ctrl_port.decrementer_secondary) {
              selector.change(DECREMENT);
              redraw = true;
            }
          }
        }
      }
#endif

#if defined(USE_TOUCH_MODULE)
      if (uCtrl.touch != nullptr) {
        uint8_t port_ref = 0;
#if defined(USE_DIN_MODULE)
        if (uCtrl.din != nullptr) {
          port_ref = uCtrl.din->sizeOf();
        }
#endif
        while (uCtrl.touch->event_queue.head != uCtrl.touch->event_queue.tail) {
          uint8_t port = uCtrl.touch->event_queue.event[uCtrl.touch->event_queue.head].port + port_ref;
          uint16_t value = uCtrl.touch->event_queue.event[uCtrl.touch->event_queue.head].value;
          uCtrl.touch->event_queue.head = (uCtrl.touch->event_queue.head + 1) % uCtrl.touch->event_queue.size;
          if (value == LOW) {
            continue;
          }
          if (port == uCtrl.page->_nav_ctrl_port.function1) {
            answered = true;
          } else if (port == uCtrl.page->_nav_ctrl_port.function2) {
            answered = true;
            yes = true;
          } else if (type != nullptr) {
            if (port == uCtrl.page->_nav_ctrl_port.incrementer || port == uCtrl.page->_nav_ctrl_port.incrementer_secondary) {
              selector.change(INCREMENT);
              redraw = true;
            } else if (port == uCtrl.page->_nav_ctrl_port.decrementer || port == uCtrl.page->_nav_ctrl_port.decrementer_secondary) {
              selector.change(DECREMENT);
              redraw = true;
            }
          }
        }
      }
#endif

#if defined(USE_AIN_MODULE)
      // nav pot support: same change schema as the page component system
      if (type != nullptr && uCtrl.page->_use_nav_pot) {
        while (uCtrl._ain_event_queue.head != uCtrl._ain_event_queue.tail) {
          uint16_t port = uCtrl._ain_event_queue.event[uCtrl._ain_event_queue.head].port;
          uint16_t value = uCtrl._ain_event_queue.event[uCtrl._ain_event_queue.head].value;
          uCtrl._ain_event_queue.head = (uCtrl._ain_event_queue.head + 1) % uCtrl._ain_event_queue.size;
          if (port == uCtrl.page->_nav_ctrl_port.pot) {
            selector.change(value);
            redraw = true;
          }
        }
      }
#endif
      delay(1);
    }

    if (yes && type != nullptr) {
      *type = selector.type();
    }
    return yes;
  }
} trackManagementComponent;
