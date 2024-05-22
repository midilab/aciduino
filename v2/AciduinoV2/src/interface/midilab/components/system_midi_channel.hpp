struct MidiChannelConfig : PageComponent {

    void view() {
      genericOptionView("channel", String(_track_output_setup[aciduino.getSelectedTrack()].channel+1), line, col, selected);
    }
    
    void change(int16_t data) {
      data = parseData(data, 0, 15, _track_output_setup[aciduino.getSelectedTrack()].channel);
      ATOMIC(_track_output_setup[aciduino.getSelectedTrack()].channel = data)
    }

} midiChannelConfigComponent;