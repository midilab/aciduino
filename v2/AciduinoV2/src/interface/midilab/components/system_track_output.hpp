struct TrackOutputSelector : PageComponent {

    void view() {
      genericOptionView("out", String(String("midi") + (aciduino.getTrackOutputParam(TRACK_PORT)+1)), line, col, selected);
    }

    void change(int16_t data) {
      data = parseData(data, 0, uCtrl.midi->sizeOf()-1, aciduino.getTrackOutputParam(TRACK_PORT));
      aciduino.setTrackOutputParam(TRACK_PORT, data);
    }

} trackOutputSelectorComponent;