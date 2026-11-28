#ifndef __TRACK_STRIP_HPP__
#define __TRACK_STRIP_HPP__

#include <Arduino.h>

#if defined(USE_TRACK_STRIP)

#include "aciduino.hpp"

// 16+8 pad surface, independent from the page/UI layer.
//
//   8 selector pads  : note | step | mute | pattern, then (free), (free), rec, play
//   16 section pads  : the active section surface
//   16 pads on shift : global track select overlay
//
// It only drives uCtrl(input/LED modules) and the aciduino/sequencer API, so a
// port without pages or a display gets the same instrument surface. Pages may
// read this state for their own views, never command it.
//
// the 24 pads are the touch channels right after the unused ones in the uCtrl
// port space(din pins first, then touch channels)
#ifndef TRACK_STRIP_PAD_SKIP
#define TRACK_STRIP_PAD_SKIP 8
#endif
// first dout port of the 24 pad leds
#ifndef TRACK_STRIP_LED_BASE
#define TRACK_STRIP_LED_BASE 0
#endif
// shift is the din pin plugged after the encoder pair
#ifndef TRACK_STRIP_SHIFT_PORT
#if defined(USE_CHANGER_ENCODER) || defined(USE_TRACK_STRIP_ENCODER)
#define TRACK_STRIP_SHIFT_PORT 2
#else
#define TRACK_STRIP_SHIFT_PORT 0
#endif
#endif
// din plug order: decrementer then incrementer
#ifndef TRACK_STRIP_ENCODER_DEC_PORT
#define TRACK_STRIP_ENCODER_DEC_PORT 0
#endif
#ifndef TRACK_STRIP_ENCODER_INC_PORT
#define TRACK_STRIP_ENCODER_INC_PORT 1
#endif

typedef enum {
  TRACK_STRIP_NOTE,
  TRACK_STRIP_STEP,
  TRACK_STRIP_MUTE,
  TRACK_STRIP_PATTERN,
  TRACK_STRIP_SECTIONS
} TRACK_STRIP_SECTION;

typedef enum {
  TRACK_STRIP_MUTE_TRACKS,
  TRACK_STRIP_MUTE_PARTS,
} TRACK_STRIP_MUTE_MODE;

struct TrackStrip
{
  // live state, read by the page views
  uint8_t section = TRACK_STRIP_NOTE;
  uint8_t selected_step = 0;      // shared step cursor: step section + step record
  uint8_t bar_window = 0;         // step section: first of the 16 steps shown
  uint8_t note_base = 36;         // 303/poly: note of pad 1
  uint8_t part_selected = 0;      // 808: last part triggered on the note section
  uint8_t mute_mode = TRACK_STRIP_MUTE_TRACKS;
  uint8_t pattern_window = 0;     // pattern section: first of the 16 patterns shown
  // one bit per pad: 16 pads need 16 bits, an 8 bit mask drops pads 9-16
  uint16_t keys_active = 0;
  uint8_t keys_note[16] = {0};
  uint8_t keys_track = 0;

  void init();

  // uCtrl module callbacks
  void input(uint8_t port, uint16_t value);
  void render();

  private:
  uint8_t padBase();
  bool shiftHeld();
  uint8_t track();
  uint16_t lastPattern();

  void setSection(uint8_t section);
  void selectTrack(uint8_t number);
  void padPress(uint8_t number);
  void padRelease(uint8_t number);
  void encoderChange(int8_t dir);

  void keyOn(uint8_t number);
  void keyOff(uint8_t number);
  void resetKeys();
  void advanceStep();
  void noteChange(int8_t dir);

  void stepPad(uint8_t number);
  void mutePad(uint8_t number);
  void patternPad(uint8_t number);
};

TrackStrip trackStrip;


inline void trackStripInput(uint8_t port, uint16_t value)
{
  trackStrip.input(port, value);
}

inline void trackStripRender()
{
  trackStrip.render();
}

inline void TrackStrip::init()
{
  // one handler for every input module so the pads can come from din or touch
  if (uCtrl.din != nullptr) uCtrl.din->setCallback(trackStripInput);
  if (uCtrl.touch != nullptr) uCtrl.touch->setCallback(trackStripInput);
  uCtrl.setLoopCallback(trackStripRender);
}

inline uint8_t TrackStrip::padBase()
{
  return (uCtrl.din != nullptr ? uCtrl.din->sizeOf() : 0) + TRACK_STRIP_PAD_SKIP;
}

inline bool TrackStrip::shiftHeld()
{
  if (uCtrl.din == nullptr) return false;
  return uCtrl.din->getDataRaw(TRACK_STRIP_SHIFT_PORT) == 1;
}

inline uint8_t TrackStrip::track()
{
  return aciduino.getSelectedTrack();
}

inline uint16_t TrackStrip::lastPattern()
{
  uint16_t saved = aciduino.getNumOfPatterns();
  return aciduino.hasPendingPatternSlot() ? saved : (saved > 0 ? saved - 1 : 0);
}

//
// input
//
inline void TrackStrip::input(uint8_t port, uint16_t value)
{
  uint8_t base = padBase();

  if (port >= base && port < (uint16_t)base + 24) {
    uint8_t pad = port - base;
    if (pad < 8) {
      // selector row acts on press only; the shift overlay owns the surface
      if (value == LOW || shiftHeld()) return;
      switch (pad) {
        case 0: setSection(TRACK_STRIP_NOTE); break;
        case 1: setSection(TRACK_STRIP_STEP); break;
        case 2: setSection(TRACK_STRIP_MUTE); break;
        case 3: setSection(TRACK_STRIP_PATTERN); break;
        case 6: Aciduino::recToggle(); break;
        case 7: Aciduino::playStop(); break;
      }
      return;
    }

    pad -= 8;
    if (value == LOW) {
      padRelease(pad);
      return;
    }
    // shift is a global overlay: the 16 pads select a track while it is held
    if (shiftHeld()) {
      selectTrack(pad);
      return;
    }
    padPress(pad);
    return;
  }

  // encoder detents only ever arrive as a press
  if (value == LOW) return;
  if (port == TRACK_STRIP_ENCODER_DEC_PORT) encoderChange(-1);
  else if (port == TRACK_STRIP_ENCODER_INC_PORT) encoderChange(1);
}

inline void TrackStrip::selectTrack(uint8_t number)
{
  if (number >= SEQ_MAX_TRACKS) return;
  if (aciduino.seq.trackType(number) == SEQ_TRACK_NONE) return;
  resetKeys();
  part_selected = 0;
  aciduino.setSelectedTrack(number);
}

inline void TrackStrip::setSection(uint8_t new_section)
{
  if (section == new_section) return;
  // never leave notes hanging on the old section surface
  resetKeys();
  section = new_section;
}

inline void TrackStrip::padPress(uint8_t number)
{
  switch (section) {
    case TRACK_STRIP_NOTE: keyOn(number); break;
    case TRACK_STRIP_STEP: stepPad(number); break;
    case TRACK_STRIP_MUTE: mutePad(number); break;
    case TRACK_STRIP_PATTERN: patternPad(number); break;
  }
}

inline void TrackStrip::padRelease(uint8_t number)
{
  // only the note section holds pads, the others act on press
  if (section == TRACK_STRIP_NOTE) keyOff(number);
}

inline void TrackStrip::encoderChange(int8_t dir)
{
  // the track select overlay blocks the encoder so section state stays safe
  if (shiftHeld()) return;

  uint8_t sel = track();

  switch (section) {
    case TRACK_STRIP_NOTE:
      noteChange(dir);
      break;

    case TRACK_STRIP_STEP: {
      if (aciduino.seq.trackType(sel) == SEQ_TRACK_NONE) return;
      uint16_t size = aciduino.seq.getTrackLength(sel);
      uint8_t bars = (size + 15) / 16;
      if (bars <= 1) return;
      int16_t window = (int16_t)bar_window + dir;
      if (window < 0) window = bars - 1;
      else if (window >= bars) window = 0;
      bar_window = window;
      break;
    }

    case TRACK_STRIP_MUTE:
      // 808: swap the 16 pads between global track mute and part mute
      if (aciduino.seq.trackType(sel) == SEQ_TRACK_808) {
        mute_mode = mute_mode == TRACK_STRIP_MUTE_TRACKS ? TRACK_STRIP_MUTE_PARTS : TRACK_STRIP_MUTE_TRACKS;
      }
      break;

    case TRACK_STRIP_PATTERN: {
      int16_t window = (int16_t)pattern_window + (int16_t)dir * 16;
      if (window < 0 || window > lastPattern()) return;
      pattern_window = window;
      break;
    }
  }
}

//
// note section
//
inline void TrackStrip::keyOn(uint8_t number)
{
  uint8_t sel = track();
  uint8_t type = aciduino.seq.trackType(sel);
  if (type == SEQ_TRACK_NONE) return;
  if (keys_active & (1 << number)) return;

  bool is_808 = type == SEQ_TRACK_808;
  uint8_t note;

  if (is_808) {
    if (number >= PARTS_808) return;
    // the triggered part becomes the selected element(encoder transposes it)
    aciduino.seq.setTrackVoice(sel, number);
    part_selected = number;
    note = aciduino.seq.getPartNote(sel, number);
  } else {
    if (note_base + number > 127) return;
    note = note_base + number;
  }

  if (keys_active == 0) keys_track = sel;
  keys_active |= 1 << number;
  keys_note[number] = note;

  aciduino.seq.input(sel, NOTE_ON, note, is_808 ? NOTE_VELOCITY_808 : NOTE_VELOCITY_303);

  // step record: write on the shared cursor and advance it
  if (aciduino.seq.getRecStatus() && aciduino.seq.getRecMode() == STEP && !aciduino.isPlaying()) {
    uint16_t size = aciduino.seq.getTrackLength(sel);
    if (size != 0 && selected_step >= size) selected_step = size - 1;
    aciduino.seq.noteOnAtStep(sel, selected_step, note, 0, is_808 || aciduino.seq.isPoly(sel));
    advanceStep();
  }
}

inline void TrackStrip::keyOff(uint8_t number)
{
  if (!(keys_active & (1 << number))) return;
  aciduino.seq.input(keys_track, NOTE_OFF, keys_note[number], 0);
  keys_active &= ~(1 << number);
}

inline void TrackStrip::resetKeys()
{
  for (uint8_t i=0; i < 16; i++) {
    if (keys_active & (1 << i)) {
      aciduino.seq.input(keys_track, NOTE_OFF, keys_note[i], 0);
      keys_active &= ~(1 << i);
    }
  }
}

inline void TrackStrip::advanceStep()
{
  uint16_t size = aciduino.seq.getTrackLength(track());
  if (size == 0) return;
  selected_step = (selected_step + 1) % size;
}

inline void TrackStrip::noteChange(int8_t dir)
{
  uint8_t sel = track();
  uint8_t type = aciduino.seq.trackType(sel);
  if (type == SEQ_TRACK_NONE) return;

  if (type == SEQ_TRACK_808) {
    // transpose the last triggered part one semitone
    if (part_selected >= PARTS_808) return;
    aciduino.seq.setTrackVoice(sel, part_selected);
    int16_t note = (int16_t)aciduino.seq.getTrackVoiceConfig(sel) + dir;
    if (note < 0 || note > 127) return;
    aciduino.seq.setTrackVoiceConfig(sel, note);
    aciduino.sendNote(note, aciduino.getTrackOutputParam(TRACK_CHANNEL), aciduino.getTrackOutputParam(TRACK_PORT), NOTE_VELOCITY_808);
    aciduino.sendNote(note, aciduino.getTrackOutputParam(TRACK_CHANNEL), aciduino.getTrackOutputParam(TRACK_PORT), 0);
    return;
  }

  note_base = constrain((int16_t)note_base + (int16_t)dir * 12, 0, 127);
}

//
// step section
//
inline void TrackStrip::stepPad(uint8_t number)
{
  uint8_t sel = track();
  if (aciduino.seq.trackType(sel) == SEQ_TRACK_NONE) return;
  uint16_t step = (uint16_t)bar_window * 16 + number;
  if (step >= aciduino.seq.getTrackLength(sel)) return;
  aciduino.seq.rest(sel, step, aciduino.seq.stepOn(sel, step));
  selected_step = step;
}

//
// mute section
//
inline void TrackStrip::mutePad(uint8_t number)
{
  uint8_t sel = track();

  if (mute_mode == TRACK_STRIP_MUTE_PARTS && aciduino.seq.trackType(sel) == SEQ_TRACK_808) {
    if (number >= PARTS_808) return;
    bool muted = aciduino.seq.getMute(sel, number);
    aciduino.seq.setMute(sel, number, !muted);
    aciduino.setMuteGridState(aciduino.getMutePattern(), muted ? 1 : 0, sel, number);
    return;
  }

  if (number >= SEQ_MAX_TRACKS || aciduino.seq.trackType(number) == SEQ_TRACK_NONE) return;
  bool muted = aciduino.seq.getMute(number);
  aciduino.seq.setMute(number, !muted);
  // 808 track mute is independent from the per part mute grid
  if (!aciduino.seq.is808(number)) {
    aciduino.setMuteGridState(aciduino.getMutePattern(), muted ? 1 : 0, number);
  }
}

//
// pattern section: a tap loads the scene on every track
//
inline void TrackStrip::patternPad(uint8_t number)
{
  uint16_t pattern = (uint16_t)pattern_window + number;
  if (pattern > lastPattern()) return;
  aciduino.loadPattern(pattern);
  for (uint8_t i=0; i < SEQ_MAX_TRACKS; i++) {
    if (aciduino.seq.trackType(i) != SEQ_TRACK_NONE) {
      aciduino.setPatternGrid(i, pattern);
    }
  }
}

//
// led view
//
inline void TrackStrip::render()
{
  if (uCtrl.dout == nullptr) return;

  // a track change while pads are held must not hang notes on the old track
  if (keys_active != 0 && aciduino.getSelectedTrack() != keys_track) {
    resetKeys();
  }

  uint8_t led = TRACK_STRIP_LED_BASE;
  bool blink = uCtrl.dout->blink();

  // selector row
  for (uint8_t i=0; i < 8; i++) {
    bool on = false;
    if (i < TRACK_STRIP_SECTIONS) {
      on = section == i;
    } else if (i == 6) {
      on = aciduino.seq.getRecStatus();
    } else if (i == 7) {
      on = aciduino.isPlaying();
    }
    uCtrl.dout->write(led + i, on ? HIGH : LOW);
  }
  led += 8;

  uint8_t sel = track();
  uint8_t type = aciduino.seq.trackType(sel);

  // global track select overlay while shift is held
  if (shiftHeld()) {
    for (uint8_t i=0; i < 16; i++) {
      bool exists = i < SEQ_MAX_TRACKS && aciduino.seq.trackType(i) != SEQ_TRACK_NONE;
      bool on = exists && (i != aciduino.getSelectedTrack() || blink);
      uCtrl.dout->write(led + i, on ? HIGH : LOW);
    }
    return;
  }

  switch (section) {
    case TRACK_STRIP_NOTE:
      for (uint8_t i=0; i < 16; i++) {
        bool playable = type != SEQ_TRACK_NONE && !(type == SEQ_TRACK_808 && i >= PARTS_808);
        bool on = playable && (keys_active & (1 << i));
        uCtrl.dout->write(led + i, on ? HIGH : LOW);
      }
      break;

    case TRACK_STRIP_STEP: {
      uint16_t size = type == SEQ_TRACK_NONE ? 0 : aciduino.seq.getTrackLength(sel);
      uint8_t bars = size == 0 ? 1 : (size + 15) / 16;
      // a shorter track must not leave the view on an empty bar
      if (bar_window >= bars) bar_window = bars - 1;
      uint8_t current = type == SEQ_TRACK_NONE ? 0xFF : aciduino.seq.getCurrentStep(sel);
      for (uint8_t i=0; i < 16; i++) {
        uint16_t step = (uint16_t)bar_window * 16 + i;
        bool on = false;
        if (step < size) {
          on = aciduino.seq.stepOn(sel, step);
          if (aciduino.isPlaying() && step == current) on = !on;
          if (step == selected_step) on = !blink;
        }
        uCtrl.dout->write(led + i, on ? HIGH : LOW);
      }
      break;
    }

    case TRACK_STRIP_MUTE:
      if (mute_mode == TRACK_STRIP_MUTE_PARTS && type == SEQ_TRACK_808) {
        for (uint8_t i=0; i < 16; i++) {
          // led on = unmuted(plays), blinking = muted, same as the mute page
          bool on = i < PARTS_808 && (aciduino.seq.getMute(sel, i) ? blink : true);
          uCtrl.dout->write(led + i, on ? HIGH : LOW);
        }
      } else {
        for (uint8_t i=0; i < 16; i++) {
          bool exists = i < SEQ_MAX_TRACKS && aciduino.seq.trackType(i) != SEQ_TRACK_NONE;
          // led on = unmuted(plays), blinking = muted, same as the mute page
          bool on = exists && (aciduino.seq.getMute(i) ? blink : true);
          uCtrl.dout->write(led + i, on ? HIGH : LOW);
        }
      }
      break;

    case TRACK_STRIP_PATTERN: {
      uint16_t last = lastPattern();
      uint8_t current = type == SEQ_TRACK_NONE ? 0xFF : aciduino.getPatternGrid(sel);
      for (uint8_t i=0; i < 16; i++) {
        uint16_t pattern = (uint16_t)pattern_window + i;
        bool on = pattern <= last && (pattern == current ? blink : true);
        uCtrl.dout->write(led + i, on ? HIGH : LOW);
      }
      break;
    }
  }
}


#endif // USE_TRACK_STRIP

#endif // __TRACK_STRIP_HPP__
