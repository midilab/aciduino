
struct TrackLength : PageComponent {

    void view() {
      genericOptionView("lenght", String(aciduino.seq.getTrackLength(aciduino.getSelectedTrack())), line, col, selected);
    }

    void change(int16_t data) {
      //clearStackNote(aciduino.getSelectedTrack());
      data = parseData(data, 1, aciduino.seq.getTrackMaxLength(aciduino.getSelectedTrack()), aciduino.seq.getTrackLength(aciduino.getSelectedTrack()));
      aciduino.seq.setTrackLength(aciduino.getSelectedTrack(), data);
    }
    
} lengthComponent;