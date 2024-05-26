struct MidiChannelConfig : PageComponent {

    void view() {
      genericOptionView("channel", String(aciduino.getTrackOutputParam(TRACK_CHANNEL)+1), line, col, selected);
    }
    
    void change(int16_t data) {
      data = parseData(data, 0, 15, aciduino.getTrackOutputParam(TRACK_CHANNEL));
      aciduino.setTrackOutputParam(TRACK_CHANNEL, data);
    }

} midiChannelConfigComponent;