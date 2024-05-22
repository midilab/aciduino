// esp32 ble midi connection status
uint8_t _ble_midi_connected = 0;

struct TopBar : PageComponent {

    // http://dotmatrixtool.com/#
    // 8px by 8px, row major, little endian
    const uint8_t SUBPAGE[8] = {0x00, 0x00, 0x00, 0x18, 0x18, 0x00, 0x00, 0x00};
    const uint8_t SUBPAGE_SELECTED[8] = {0x00, 0x00, 0x3c, 0x3c, 0x3c, 0x3c, 0x00, 0x00};
    const uint8_t PLAYING[8] = {0x00, 0x06, 0x1e, 0x7e, 0x7e, 0x1e, 0x06, 0x00};
    
    TopBar() {
      // no directly navigation thru here
      no_nav = true;
    }

    void view() {
      // short page name at left most with box over it
      // |T1| 808 |seqr|o.   |i| 120.0
      uint8_t subpage_size = uCtrl.page->getSubPageSize();
      uint8_t selected_subpage = uCtrl.page->getSubPage();

      // track number and type
      //uCtrl.oled->display->drawUTF8(2, 0, atoi(aciduino.getSelectedTrack()+1)); 
      uCtrl.oled->print("T", 1, 1); 
      uCtrl.oled->print(String(aciduino.getSelectedTrack()+1), 1, 2); 
      uCtrl.oled->display->drawBox(0, 0, 10, 8);
      uCtrl.oled->print(aciduino.seq.is303(aciduino.getSelectedTrack()) ? "303" : "808", 1, 4); 
/*
      uCtrl.oled->display->drawBox(0, 0, 15, 8);
      //uCtrl.oled->drawBox();
      if (aciduino.getSelectedTrack() < 9) {
        uCtrl.oled->print("T0", 1, 1); 
        uCtrl.oled->print(String(aciduino.getSelectedTrack()+1), 1, 3); 
      } else {
        uCtrl.oled->print("T", 1, 1); 
        uCtrl.oled->print(String(aciduino.getSelectedTrack()+1), 1, 2); 
      }
 */
/*
      // current pattern
      uint8_t placement = 8;
      uCtrl.oled->print("P", 1, placement);
      uCtrl.oled->display->drawBox(((placement-1)*5)-2, 0, 8, 8);
      uCtrl.oled->print("1", 1, placement+2);

      // page/subpage
      placement = 12;
      uCtrl.oled->print(uCtrl.page->getPageName(), 1, placement);
      uCtrl.oled->display->drawBox(((placement-1)*5)-2, 0, 23, 8);
      uCtrl.oled->print(String(selected_subpage+1), 1, placement+5);
*/
       
      // page/subpage 
      uCtrl.oled->print(uCtrl.page->getPageName(), 1, 9);
      uCtrl.oled->display->drawBox(39, 0, 22, 8);
      for (uint8_t i=0; i < subpage_size; i++) {
        if (i == selected_subpage) {
          uCtrl.oled->print(SUBPAGE_SELECTED, 1, i+9);
        } else {
          uCtrl.oled->print(SUBPAGE, 1, i+9);
        }
      }

      if (_ble_midi_connected) {
        uCtrl.oled->print("B", 1, 17);
      }
      
      // bpm display and setup
      //uCtrl.oled->print(String(uClock.getTempo(), 1), 1, 21, selected);
      uCtrl.oled->display->drawBox(88, 0, 8, 8);
      uCtrl.oled->print(String(uClock.getTempo(), 1), 1, 21);
      if (aciduino.isPlaying()) {
        uCtrl.oled->print(PLAYING, 1, 19);
      } else {
        uCtrl.oled->print(uClock.getMode() == uClock.INTERNAL_CLOCK ? "i" : "e", 1, 19);  
      }

      // top bar big line 
      uCtrl.oled->display->drawBox(0, 9, 128, 1);
    }

} topBarComponent;