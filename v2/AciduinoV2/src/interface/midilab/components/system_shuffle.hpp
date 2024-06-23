struct ShuffleControl : PageComponent {

  void view() {
    genericOptionView("shuffle", uClock.isShuffled() ? String("on") : String("off"), line, col, selected);
  }

  void change(int16_t data) {
    data = parseData(data, 0, 1, uClock.isShuffled());
    uClock.setShuffle(data);
  }

} shuffleControlComponent;

struct ShuffleSignature : PageComponent {

  // MPC60 groove signatures?
  // Same as 909
  uint8_t current_shuffle = 0;
  uint8_t template_size = 2;
  int8_t shuffle_50[2] = {0, 0};
  int8_t shuffle_54[2] = {0, 2};
  int8_t shuffle_58[2] = {0, 4};
  int8_t shuffle_62[2] = {0, 6};
  int8_t shuffle_66[2] = {0, 8};
  int8_t shuffle_71[2] = {0, 10};
  int8_t shuffle_75[2] = {0, 12};

  String shuffle_name[7] = {"50%", "54%", "58%", "62%", "66%", "71%", "75%"};
  
  int8_t* shuffle_templates[7] = {shuffle_50, shuffle_54, shuffle_58, shuffle_62, shuffle_66, shuffle_71, shuffle_75};

  void view() {
    genericOptionView("sig.", shuffle_name[current_shuffle], line, col, selected);
  }

  void change(int16_t data) {
    current_shuffle = parseData(data, 0, 6, current_shuffle);
    // uClock shuffle
    uClock.setShuffleTemplate(shuffle_templates[current_shuffle], template_size);
  }

} shuffleSignatureComponent;