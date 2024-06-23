struct RecMode : PageComponent {

    uint8_t rec_mode = 0;

    void view() {
      genericOptionView("mode", rec_mode == 0 ? String("real") : String("step"), line, col, selected);
    }

    void change(int16_t data) {
      data = parseData(data, 0, 1, rec_mode);
      rec_mode = data;
    }

} recModeComponent;