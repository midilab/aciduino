# ACIDUINO V2 — uONE 16+8 Track Strip User Manual

## Introduction

The **track strip** turns the uONE pad grid into an instrument surface. The 8 pads
above the 16 pads become strip controls, and the 16 pads below become the active
**section**: notes, steps, mutes or patterns.

It is not part of a page. The strip behaves the same on every page of the OLED
interface, and it also runs on an aciduino without any page or display at all.
Pages only show what the strip is doing; they never take the pads back.

The strip is enabled per port with:

```c
#define USE_TRACK_STRIP           // 8 selector pads + 16 section pads
#define USE_TRACK_STRIP_ENCODER   // optional: the encoder drives the active section
```

```
  [1] [2] [3] [4] [5] [6] [7] [8]      <- 8 selector pads
 [ 1][ 2][ 3][ 4][ 5][ 6][ 7][ 8][ 9][10][11][12][13][14][15][16]   <- 16 section pads
```

---

## 1. The 8 selector pads

| Pad | Function | LED |
|-----|----------|-----|
| 1 | **note** section | on while it is the active section |
| 2 | **step** section | on while it is the active section |
| 3 | **mute** section | on while it is the active section |
| 4 | **pattern** section | on while it is the active section |
| 5 | free (reserved) | off |
| 6 | free (reserved) | off |
| 7 | **rec** toggle | on while recording is armed |
| 8 | **play / stop** toggle | on while the sequencer is playing |

- At boot the **note** section is selected.
- Tapping a section pad switches the 16 pads to that section immediately.
- Only one section LED is lit at a time.
- The section choice is not saved; a fresh boot starts on **note** again.

---

## 2. The track select overlay (hold shift)

Hold **shift** and the 16 section pads become the **global track select**,
whatever section is currently active:

| Pad state | Meaning |
|-----------|---------|
| lit | a track exists in that slot |
| blinking | the currently selected track |
| dark | empty slot |

- Tap a lit pad to select that track.
- Release shift and the pads return to the section that was active before.
  Nothing is lost, because the section never changed.

While shift is held:

- the **encoder is locked** (nothing moves the section data),
- the **8 selector pads are locked** (no accidental section change),
- holding a note and then pressing shift still releases the note correctly.

Shift keeps its normal OLED duties as well: `shift + up/down` changes page,
`shift + left/right` changes subpage.

---

## 3. Sections

### 3.1 Note

The 16 pads play notes on the selected track.

**303 and poly tracks**

- Pad 1 is **C2** (note 36) by default, each following pad is one semitone:
  pads 1–16 = 16 semitones.
- **Encoder** = octave up / down (±12 semitones).
- LED: pad lit while it is held.

**808 tracks**

- Pads 1–11 play drum parts 1–11. Pads 12–16 are inactive (dark).
- The **last triggered part becomes the selected element**. It is the part shown
  on the OLED and the part the encoder acts on.
- **Encoder** = transpose the selected element one semitone up / down.
  All events of that part are remapped to the new note.
- LED: pad lit while it is held.

**Recording from the note section**

- **Rec armed + sequencer stopped (step rec):** every pad press writes a note at
  the current step cursor and advances the cursor one step. Use the **step**
  section to place the cursor or change the bar.
- **Rec armed + sequencer playing (realtime rec):** pads record in realtime like
  a normal MIDI keyboard.
- **Rec off:** pads simply play the track.

### 3.2 Step

A step editor for the selected track.

- The 16 pads always show **one 16-step bar** of the sequence.
- Tap a pad to toggle that step on/off and make it the **step cursor**.
- **Encoder** = move to the previous / next 16-step bar. It only moves when the
  track is longer than 16 steps (max bar count comes from the track length).
- The bar shown does **not** follow the playhead. It stays where you put it, so
  editing is never pulled away under your finger.
- On an **808** track the step pads edit the currently selected drum part.

LEDs:

| LED | Meaning |
|-----|---------|
| on | step is active |
| off | step is a rest |
| blinking | the step cursor |
| inverted while playing | the step currently playing |

The step cursor is shared with the note section: step rec writes there, and
selecting a step here moves the record position. The OLED sequencer page
highlights the same step and bar.

### 3.3 Mute

Live track mutes.

| LED | Meaning |
|-----|---------|
| on | track exists and is unmuted |
| blinking | track is muted |
| dark | empty slot |

- Tap a pad to mute/unmute that track.
- Only the tracks that exist are shown; empty slots stay dark.
- 303/poly track mutes and 808 **part** mutes are written into the active
  **mute automation pattern**, so they are kept with the pattern. An 808
  track-level mute is live only (as on the mute page).

**808 tracks: track mute vs. part mute**

With an 808 track selected, the encoder swaps the 16 pads between two modes:

1. **Track mute** (default): pads mute/unmute whole tracks, like above.
2. **Part mute**: pads 1–11 mute/unmute the drum parts of the selected 808
   track. Pads 12–16 are dark.

Turn the encoder once to change mode, again to change back. On 303 and poly
tracks the encoder does nothing in this section.

### 3.4 Pattern

Pattern and scene selection.

- Tap a pad to load that **pattern on every track** (a scene).
- The pads show the visible 16 patterns of the pattern bank.
- **Encoder** = move forward / backward by 16 patterns, when more patterns exist.

LEDs:

| LED | Meaning |
|-----|---------|
| on | saved pattern |
| blinking | the current pattern of the selected track |
| dark | no pattern in that slot |

The OLED pattern page still shows the same state, including copy/paste and
save; the strip only does live scene loading.

---

## 4. LED quick reference

| Area | LED | Meaning |
|------|-----|---------|
| selector | 1–4 | active section |
| selector | 7 | rec armed |
| selector | 8 | playing |
| shift held | 16 pads | lit = track exists, blink = selected track |
| note | 16 pads | note held |
| step | 16 pads | step active / cursor blink / playhead inverted |
| mute | 16 pads | unmuted / muted blink / empty dark |
| mute (808 parts) | 1–11 | part unmuted / part muted blink |
| pattern | 16 pads | pattern exists / current pattern blink |

---

## 5. Encoder quick reference

| Section | Encoder turn |
|---------|--------------|
| note, 303/poly | octave up / down |
| note, 808 | transpose the last triggered part one semitone |
| step | previous / next 16-step bar |
| mute, 808 | switch track mute ⟷ part mute |
| mute, 303/poly | no function |
| pattern | previous / next 16 patterns |
| shift held | locked in every section |

---

## 6. Workflows

### Record a 303 line

1. Hold **shift**, tap the pad of the 303 track, release shift.
2. Select the **note** section (selector pad 1).
3. Turn **rec** on (selector pad 7) and make sure the sequencer is stopped.
4. Tap the 16 pads: each press writes a note at the cursor and advances it.
5. Select the **step** section to review the bar, toggle steps, or move to the
   next bar with the encoder.
6. Rec off, play, and the line runs.

### Mute a drum part on an 808

1. Hold **shift**, select the 808 track.
2. Select the **mute** section (selector pad 3).
3. Turn the encoder once: the pads switch from track mute to part mute.
4. Tap pad 1–11 to mute/unmute that drum part.
5. Turn the encoder again to return to track mutes.

### Change a scene

1. Select the **pattern** section (selector pad 4).
2. Tap a lit pad: that pattern loads on every track at once.
3. Use the encoder to reach patterns 17–32, 33–48, and so on.

### Live play from the pads

1. Hold **shift** to pick the track.
2. Select the **note** section.
3. Play. On 303/poly turn the encoder for octaves; on 808 trigger a part then
   turn the encoder to transpose it.

---

## 7. Notes and limits

- **Accent is not available from the pads.** Shift is the track select overlay,
  so a pad press cannot carry shift. Use the OLED sequencer page function
  button to set accent on the selected step.
- **The step bar does not auto-follow** the playhead; it is a manual edit view.
- **All tracks share one note budget.** uONE holds 512 events (AVR ports 128),
  enough for 8 tracks of up to 64 steps. When that budget is full, new
  step/accent/slide writes are ignored without a warning, so if a section stops
  reacting, clear or shorten a track to free notes.
- **Updating from an older build:** patterns saved by the previous firmware were
  stored with a smaller image size, so they can load garbled once. Re-save them
  after the update. Keep the SD card inserted — the larger image no longer fits
  the on-chip EEPROM, so EEPROM-only units cannot hold patterns.
- **Pattern scene loading** loads all tracks at once. Loading a single track is
  done on the OLED pattern page.
- **Section and track-selection state are not persisted.** A boot starts on the
  note section with the track the session loaded.
- **Encoder direction:** if your encoder feels inverted (increment should go
  forward), swap `TRACK_STRIP_ENCODER_DEC_PIN` and `TRACK_STRIP_ENCODER_INC_PIN`
  in the port definition.
- The strip works on ports with or without the page/OLED layer; the page module
  is never required.

---

## 8. Support

- Project documentation: [v2/README](https://github.com/midilab/aciduino/tree/master/v2)
- Issues and feature requests: [github.com/midilab/aciduino/issues](https://github.com/midilab/aciduino/issues)
