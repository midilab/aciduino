#ifndef __SHARED_GUI_FUNCS_H__
#define __SHARED_GUI_FUNCS_H__

//
// uCtrl Shared PageComponent 
// all Shared GUI components are programmed as PageComponent to be reused on different pages
//

// for a 1x grid 1x line size
// large == true ? 2x grid size
// used by a lot of simple components
void genericOptionView(const char * title_char, String value, uint8_t line, uint8_t col, bool selected, bool large = false)
{
  String title(title_char);
  const uint8_t view_size = 12;
  uint8_t x=(col-1)*5, y=(line-1)*8;
  uint8_t value_col = large ? (view_size*2)-value.length()+1 : view_size-value.length();
  uCtrl.oled->display->drawBox(x+1, y, 1, 7);
  uCtrl.oled->print(title, line, col+1, selected);
  uCtrl.oled->print(value, line, col+value_col, selected);
}

// used by page module for themed function button display
void functionDrawCallback(const char * f1, const char * f2, uint8_t f1_state, uint8_t f2_state)
{
  // menu action
  uCtrl.oled->print(f1, 8, 1+((14-strlen(f1))/2)); 
  uCtrl.oled->print(f2, 8, 14+((14-strlen(f2))/2)); 
  // state variation
  if (f1_state == 1) {
    uCtrl.oled->display->drawBox(0, 57, 63, 7);
  }
  if (f2_state == 1) {
    uCtrl.oled->display->drawBox(64, 57, 64, 7);
  }
  // horizontal line
  uCtrl.oled->display->drawBox(0, 56, 128, 1);
  // vertical separator
  //uCtrl.oled->display->drawBox(64, 59, 1, 5);
}

#endif // __SHARED_GUI_FUNCS_H__