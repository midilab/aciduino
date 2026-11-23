// uONE selector row: a persistent 8 track strip.
// tap = select track, shift+tap = mute/unmute. The row banks with the
// selected track (1-8, 9-16) and the OLED top bar shows the bank.
// Only ports that define USE_TRACK_STRIP get this behaviour; everywhere
// else both functions are no-ops and the selector pads keep their page role.

#if defined(USE_TRACK_STRIP)

inline bool trackStripInput(uint8_t control_id, uint16_t value)
{
  if (control_id < SELECTOR_BUTTON_1 || control_id > SELECTOR_BUTTON_8) {
    return false;
  }
  // consume the release too, it belongs to this row
  if (value == LOW) {
    return true;
  }

  uint8_t pad = control_id - SELECTOR_BUTTON_1;
  uint8_t track = (aciduino.getSelectedTrack() / 8) * 8 + pad;
  if (track >= SEQ_MAX_TRACKS || aciduino.seq.trackType(track) == SEQ_TRACK_NONE) {
    return true;
  }

  if (uCtrl.page->isShiftPressed()) {
    aciduino.seq.setMute(track, !aciduino.seq.getMute(track));
    mutePatternComponent.syncTrackMute(track);
  } else {
    aciduino.setSelectedTrack(track);
  }
  return true;
}

inline void trackStripView()
{
#if defined(USE_LED_8) || defined(USE_LED_24)
  uint8_t bank = (aciduino.getSelectedTrack() / 8) * 8;
  for (uint8_t i=0; i < 8; i++) {
    uint8_t track = bank + i;
    uint8_t led = SELECTOR_LED_1 + i;
    if (track >= SEQ_MAX_TRACKS || aciduino.seq.trackType(track) == SEQ_TRACK_NONE) {
      uCtrl.dout->write(led, LOW);
    } else if (aciduino.seq.getMute(track)) {
      uCtrl.dout->write(led, uCtrl.dout->blink() ? HIGH : LOW);
    } else {
      uCtrl.dout->write(led, HIGH);
    }
  }
#endif
}

#else

inline bool trackStripInput(uint8_t, uint16_t) { return false; }
inline void trackStripView() {}

#endif
