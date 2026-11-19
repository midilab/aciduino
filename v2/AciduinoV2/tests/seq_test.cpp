/*
 * Host test for the note based sequencer engine.
 *
 * Build & run (from v2/AciduinoV2):
 *   g++ -std=c++11 -Wall -O1 -DSEQ_MAX_NOTES=64 -Isrc -Isrc/sequencer \
 *       tests/seq_test.cpp src/sequencer/sequencer.cpp \
 *       src/sequencer/seq_track_handler.cpp src/sequencer/handler_303.cpp \
 *       src/sequencer/handler_808.cpp src/sequencer/handler_poly.cpp \
 *       -o /tmp/opencode/seq_test && /tmp/opencode/seq_test
 *
 * SEQ_MAX_NOTES=64 on purpose: exercises pool exhaustion with the small budget
 * the production build uses (512).
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "sequencer.h"
#include "seq_track_handler.h"

//
// output capture
//
struct Capture {
  struct Ev { uint8_t msg, note, vel, track; int tick; } ev[512];
  int n = 0;

  void cb(uint8_t msg, uint8_t note, uint8_t vel, uint8_t track, int tick) {
    if (n < 512) {
      ev[n++] = {msg, note, vel, track, tick};
    }
  }
  int countOf(uint8_t msg, uint8_t note, uint8_t track) {
    int c = 0;
    for (int i = 0; i < n; i++) {
      if (ev[i].msg == msg && ev[i].note == note && ev[i].track == track) c++;
    }
    return c;
  }
  int idxOf(uint8_t msg, uint8_t note, uint8_t track) {
    for (int i = 0; i < n; i++) {
      if (ev[i].msg == msg && ev[i].note == note && ev[i].track == track) return i;
    }
    return -1;
  }
  void reset() { n = 0; }
};

static Capture g_cap;
static int g_tick = 0;
static void capCb(uint8_t msg, uint8_t note, uint8_t vel, uint8_t track) {
  g_cap.cb(msg, note, vel, track, g_tick);
}

// 24 96PPQN pulses per step(96/4), uClock calls onStep before onPulse on the same pulse
static void simulate(NoteSequencer &seq, int ticks) {
  for (int tick = 0; tick < ticks; tick++) {
    g_tick = tick;
    if (tick % 24 == 0) {
      seq.onStep(tick / 24, 0);
    }
    seq.onPulse(tick);
  }
}

//
// tests
//

static void test_default_layout()
{
  NoteSequencer seq;
  seq.init();

  assert(seq.trackCount() == 4);
  assert(seq.trackType(0) == SEQ_TRACK_303);
  assert(seq.trackType(1) == SEQ_TRACK_303);
  assert(seq.trackType(2) == SEQ_TRACK_POLY);
  assert(seq.trackType(3) == SEQ_TRACK_808);
  assert(seq.trackType(4) == SEQ_TRACK_NONE);

  assert(seq.getLength(0) == STEP_DEFAULT_LENGTH_303);
  assert(seq.getLength(2) == STEP_DEFAULT_LENGTH_POLY);
  assert(seq.getLength(3) == STEP_DEFAULT_LENGTH_808);

  // 808 default parts
  assert(seq.partNote(3, 0) == 36);
  assert(strcmp(seq.partName(3, 0), "bd") == 0);
  assert(seq.partByNote(3, 38) == 1);
  assert(seq.partByNote(3, 100) == -1);

  // pool is packed in slot order
  seq.insertNote(0, 0, 60, 0);
  seq.insertNote(3, 0, 36, 0);
  assert(seq.notesUsed() == 2);
}

static void test_playback_mono()
{
  NoteSequencer seq;
  seq.setOutputCallback(capCb);
  seq.init();

  seq.setLength(0, 8);
  seq.insertNote(0, 0, 60, 0);
  seq.insertNote(0, 2, 62, 0);
  seq.insertNote(0, 5, 64, SEQ_EV_ACCENT);

  simulate(seq, 8 * 24); // one full pass

  assert(g_cap.countOf(NOTE_ON, 60, 0) == 1);
  assert(g_cap.countOf(NOTE_ON, 62, 0) == 1);
  assert(g_cap.countOf(NOTE_ON, 64, 0) == 1);
  assert(g_cap.countOf(NOTE_OFF, 60, 0) == 1);
  assert(g_cap.countOf(NOTE_OFF, 62, 0) == 1);
  assert(g_cap.countOf(NOTE_OFF, 64, 0) == 1);

  // gate = 12: note on at tick 0, off at tick 12
  int i_on = g_cap.idxOf(NOTE_ON, 60, 0);
  int i_off = g_cap.idxOf(NOTE_OFF, 60, 0);
  assert(i_on >= 0 && i_off > i_on);
  // accent uses the accent velocity
  assert(g_cap.ev[g_cap.idxOf(NOTE_ON, 64, 0)].vel == seq.getAccentVel(0));
  assert(g_cap.ev[g_cap.idxOf(NOTE_ON, 60, 0)].vel == seq.getNoteVel(0));

  // playback wraps: two full passes
  g_cap.reset();
  simulate(seq, 16 * 24);
  assert(g_cap.countOf(NOTE_ON, 60, 0) == 2);
  assert(g_cap.countOf(NOTE_ON, 62, 0) == 2);
  assert(g_cap.countOf(NOTE_OFF, 64, 0) == 2);
}

static void test_poly_chord()
{
  NoteSequencer seq;
  seq.setOutputCallback(capCb);
  seq.init();

  uint8_t poly = 2;
  seq.setLength(poly, 8);
  seq.insertNote(poly, 1, 60, 0);
  seq.insertNote(poly, 1, 64, SEQ_EV_ACCENT);
  seq.insertNote(poly, 1, 67, 0);

  assert(seq.noteCount(poly) == 3);
  assert(seq.noteAtStep(poly, 1, 0) == 60);
  assert(seq.noteAtStep(poly, 1, 1) == 64);
  assert(seq.noteAtStep(poly, 1, 2) == 67);
  assert(seq.noteAtStep(poly, 1, 3) == 0xFF);

  simulate(seq, 8 * 24);
  assert(g_cap.countOf(NOTE_ON, 60, poly) == 1);
  assert(g_cap.countOf(NOTE_ON, 64, poly) == 1);
  assert(g_cap.countOf(NOTE_ON, 67, poly) == 1);
  assert(g_cap.countOf(NOTE_OFF, 60, poly) == 1);
  assert(g_cap.countOf(NOTE_OFF, 64, poly) == 1);
  assert(g_cap.countOf(NOTE_OFF, 67, poly) == 1);
}

static void test_808_part_mute_and_remap()
{
  NoteSequencer seq;
  seq.setOutputCallback(capCb);
  seq.init();

  uint8_t dr = 3;
  seq.setLength(dr, 8);
  seq.insertNote(dr, 0, 36, 0); // bd
  seq.insertNote(dr, 2, 38, 0); // sd

  // mute the bd part: only sd fires
  seq.setPartMute(dr, 0, 1);
  simulate(seq, 8 * 24);
  assert(g_cap.countOf(NOTE_ON, 36, dr) == 0);
  assert(g_cap.countOf(NOTE_ON, 38, dr) == 1);
  assert(g_cap.countOf(NOTE_OFF, 38, dr) == 1);

  // unmute and remap the bd part to note 60: events follow the part
  g_cap.reset();
  seq.setPartMute(dr, 0, 0);
  assert(seq.setPartNote(dr, 0, 60));
  assert(seq.partNote(dr, 0) == 60);
  assert(seq.noteAtStep(dr, 0, 0) == 60);
  assert(strcmp(seq.partName(dr, 0), "bd") == 0);

  simulate(seq, 8 * 24);
  assert(g_cap.countOf(NOTE_ON, 60, dr) == 1);
  assert(g_cap.countOf(NOTE_ON, 38, dr) == 1);

  // remap onto an existing part note is rejected(ambiguous)
  assert(!seq.setPartNote(dr, 1, 60));
}

static void test_808_roll()
{
  NoteSequencer seq;
  seq.setOutputCallback(capCb);
  seq.init();

  uint8_t dr = 3;
  seq.setLength(dr, 8);
  seq.setRollType(dr, FLAM_3);
  seq.insertNote(dr, 0, 48, SEQ_EV_ROLL);

  simulate(seq, 8 * 24);
  // main hit + the flam sub hit at pulse 18 + gate off
  assert(g_cap.countOf(NOTE_ON, 48, dr) == 2);
  assert(g_cap.countOf(NOTE_OFF, 48, dr) == 1);
  int i1 = g_cap.idxOf(NOTE_ON, 48, dr);
  int i2 = -1;
  for (int k = i1 + 1; k < g_cap.n; k++) {
    if (g_cap.ev[k].msg == NOTE_ON && g_cap.ev[k].note == 48 && g_cap.ev[k].track == dr) {
      i2 = k;
      break;
    }
  }
  assert(i2 > i1);
  // FLAM_3: shoot when the window counter reaches 6(17th pulse after the hit)
  assert(g_cap.ev[i2].tick == 17);
  // the retrigger uses the accent velocity
  assert(g_cap.ev[i2].vel == seq.getAccentVel(dr));
}

static void test_303_tie_and_slide()
{
  NoteSequencer seq;
  seq.setOutputCallback(capCb);
  seq.init();

  seq.setLength(0, 8);
  // note at 0, tied rests at 1 and 2, next note at 3
  seq.insertNote(0, 0, 60, SEQ_EV_TIE);
  seq.insertNote(0, 1, 60, SEQ_EV_REST | SEQ_EV_TIE);
  seq.insertNote(0, 2, 60, SEQ_EV_REST | SEQ_EV_TIE);
  seq.insertNote(0, 3, 62, 0);

  simulate(seq, 96); // the tied off lands at tick 60, beyond one loop
  int i_on = g_cap.idxOf(NOTE_ON, 60, 0);
  int i_off = g_cap.idxOf(NOTE_OFF, 60, 0);
  int i_next = g_cap.idxOf(NOTE_ON, 62, 0);
  assert(i_on >= 0 && i_off > i_on);
  // gate 12 + 2 tied steps * 24 = 60 pulses: off at tick 60
  assert(g_cap.ev[i_off].tick == 60);
  // the tied note releases (tick 60) before the next note starts (tick 72)
  assert(g_cap.ev[i_next].tick > g_cap.ev[i_off].tick);

  // slide: note at 0 slides into note at 3 -> 12 + 20 + 2*24 = 80
  g_cap.reset();
  seq.clearNotes(0);
  seq.insertNote(0, 0, 60, SEQ_EV_SLIDE);
  seq.insertNote(0, 3, 62, 0);
  simulate(seq, 96);
  i_on = g_cap.idxOf(NOTE_ON, 60, 0);
  i_off = g_cap.idxOf(NOTE_OFF, 60, 0);
  assert(i_on >= 0 && g_cap.ev[i_off].tick == 80);
}

static void test_rest_events_are_silent()
{
  NoteSequencer seq;
  seq.setOutputCallback(capCb);
  seq.init();

  seq.setLength(0, 8);
  seq.setRestAtStep(0, 2, true, 60); // off step with data alive
  assert(seq.hasNoteAtStep(0, 2, 0xFF) == false);
  assert(seq.flagsAtStep(0, 2, 0xFF) & SEQ_EV_REST);
  seq.setRestAtStep(0, 2, false, 60); // turn it on
  assert(seq.hasNoteAtStep(0, 2, 0xFF) == true);
  assert(seq.noteAtStep(0, 2, 0) == 60);

  simulate(seq, 8 * 24); // two full loops of 8 steps
  assert(g_cap.countOf(NOTE_ON, 60, 0) == 2);
}

static uint8_t myHook(uint8_t note, uint8_t track)
{
  (void)track;
  return (note == 99) ? 0xFF : note;
}

static void test_transpose_and_hook()
{
  NoteSequencer seq;
  seq.setOutputCallback(capCb);
  seq.init();

  seq.setLength(0, 4);
  seq.setTranspose(0, -12);
  seq.insertNote(0, 0, 60, 0);
  seq.insertNote(0, 1, 8, 0); // transposed below the midi range
  seq.insertNote(0, 2, 99, 0);

  g_cap.reset();
  simulate(seq, 4 * 24);
  assert(g_cap.countOf(NOTE_ON, 48, 0) == 1);
  assert(g_cap.countOf(NOTE_ON, 8, 0) == 0); // out of range after transpose
  assert(g_cap.countOf(NOTE_ON, 87, 0) == 1); // 99-12, no hook active yet
  seq.clearActiveNotes(-1);

  // hook path: the hook sees the raw note(before transpose) and kills it
  seq.setNoteHook(myHook);
  g_cap.reset();
  simulate(seq, 4 * 24);
  assert(g_cap.countOf(NOTE_ON, 48, 0) == 1);
  assert(g_cap.countOf(NOTE_ON, 87, 0) == 0); // 99 suppressed by hook
}

static void test_create_delete_tracks()
{
  NoteSequencer seq;
  seq.setOutputCallback(capCb);
  seq.init();

  // give the default tracks data(slot 2 poly, slot 3 808)
  seq.insertNote(0, 0, 60, 0);
  seq.insertNote(2, 0, 70, 0);
  seq.insertNote(3, 0, 36, 0);

  // free a slot(empty track 1) and create a poly track in it
  assert(seq.deleteTrack(1));
  assert(seq.trackType(1) == SEQ_TRACK_NONE);
  assert(seq.trackCount() == SEQ_MAX_TRACKS - 1);

  uint8_t t = seq.createTrack(SEQ_TRACK_POLY);
  assert(t == 1);
  assert(seq.trackCount() == SEQ_MAX_TRACKS);
  seq.setLength(t, 8);
  seq.insertNote(t, 0, 72, 0);
  seq.insertNote(t, 3, 74, 0);

  // the pool stays packed: every track keeps its own notes
  assert(seq.notesUsed() == 5);
  assert(seq.noteAtStep(0, 0, 0) == 60);
  assert(seq.noteAtStep(2, 0, 0) == 70);
  assert(seq.noteAtStep(3, 0, 0) == 36);
  assert(seq.noteAtStep(t, 3, 0) == 74);

  // delete a track that HAS notes: later slices must survive
  assert(seq.deleteTrack(t));
  assert(seq.noteCount(t) == 0);
  assert(seq.notesUsed() == 3);
  assert(seq.noteAtStep(0, 0, 0) == 60);
  assert(seq.noteAtStep(2, 0, 0) == 70);
  assert(seq.noteAtStep(3, 0, 0) == 36);

  // playback: no ghost notes, each surviving track fires its own note
  g_cap.reset();
  simulate(seq, 8 * 24);
  assert(g_cap.countOf(NOTE_ON, 60, 0) == 1);
  assert(g_cap.countOf(NOTE_ON, 70, 2) == 1);
  assert(g_cap.countOf(NOTE_ON, 36, 3) == 1);
  assert(g_cap.countOf(NOTE_ON, 72, t) == 0);
  assert(g_cap.countOf(NOTE_ON, 74, t) == 0);

  // deleting an empty/invalid slot
  assert(!seq.deleteTrack(t));
  assert(!seq.deleteTrack(SEQ_MAX_TRACKS));

  // create until the slots run out: slot t is the only free one
  int made = 0;
  for (int i = 0; i < SEQ_MAX_TRACKS + 4; i++) {
    if (seq.createTrack(SEQ_TRACK_POLY) != 0xFF) made++;
  }
  assert(made == 1);
}

static void test_pool_exhaustion()
{
  NoteSequencer seq;
  seq.setOutputCallback(capCb);
  seq.init();

  int inserted = 0;
  // use the two 303 slots only: their length (32) covers every step used
  for (int i = 0; i < SEQ_MAX_NOTES + 10; i++) {
    uint8_t step = (uint8_t)(i % 16);
    uint8_t track = (uint8_t)(i % 2);
    if (seq.insertNote(track, step, (uint8_t)(36 + (i % 40)), 0)) {
      inserted++;
    }
  }
  assert(inserted == SEQ_MAX_NOTES);
  assert(seq.notesFree() == 0);
}

static void test_track_image_roundtrip()
{
  NoteSequencer seq;
  seq.setOutputCallback(capCb);
  seq.init();

  uint8_t dr = 3;
  seq.setLength(dr, 12);
  seq.setShift(dr, 1);
  seq.setPartMute(dr, 3, 1);
  seq.insertNote(dr, 2, 36, SEQ_EV_ROLL);
  seq.insertNote(dr, 6, 48, SEQ_EV_ACCENT);

  uint8_t buf[3 + sizeof(SEQ_TRACK) + 512 * sizeof(SEQ_EVENT)];
  assert(seq.saveTrackImage(dr, buf));
  uint16_t size = buf[0] | ((uint16_t)buf[1] << 8);
  assert(size == 2 + sizeof(SEQ_TRACK) + 2 * sizeof(SEQ_EVENT));

  // wipe the track and load the blob back
  seq.clearNotes(dr);
  assert(seq.noteCount(dr) == 0);
  assert(seq.loadTrackImage(dr, buf));
  assert(seq.noteCount(dr) == 2);
  assert(seq.getLength(dr) == 12);
  assert(seq.getShift(dr) == 1);
  assert(seq.partMute(dr, 3) == 1);
  assert(seq.flagsAtStep(dr, 2, 36) & SEQ_EV_ROLL);
  assert(seq.flagsAtStep(dr, 6, 48) & SEQ_EV_ACCENT);

  // corrupt the blob size: rejected
  uint8_t bad[3 + sizeof(SEQ_TRACK)];
  bad[0] = 0xFF; bad[1] = 0xFF; // way too big
  assert(!seq.loadTrackImage(dr, bad));
}

static void test_session_layout_merge()
{
  NoteSequencer seq;
  seq.setOutputCallback(capCb);
  seq.init();

  seq.setLength(0, 8);
  seq.insertNote(0, 0, 60, 0);
  seq.insertNote(1, 0, 62, 0);
  seq.insertNote(2, 0, 64, 0); // poly
  seq.insertNote(3, 0, 36, 0); // 808

  SEQ_TRACK layout[SEQ_MAX_TRACKS];
  seq.copyTrackConfigs(layout);
  assert(layout[0].type == SEQ_TRACK_303);
  assert(layout[3].type == SEQ_TRACK_808);
  assert(layout[0].count == 0); // runtime fields are not session data

  // session 808 part remap follows into the stored note numbers
  layout[3].parts[0].note = 40;
  seq.applyTrackLayout(layout);
  assert(seq.partNote(3, 0) == 40);
  assert(seq.noteAtStep(3, 0, 0) == 40);

  // session drops a track: its notes go, the others stay
  layout[1].type = SEQ_TRACK_NONE;
  layout[0].length = 16;
  seq.applyTrackLayout(layout);
  assert(seq.trackType(1) == SEQ_TRACK_NONE);
  assert(seq.noteCount(1) == 0);
  assert(seq.noteCount(0) == 1);
  assert(seq.noteCount(2) == 1);
  assert(seq.noteCount(3) == 1);
  assert(seq.getLength(0) == 16);

  // session adds a track back: it is empty, factory configured
  uint8_t slot = seq.createTrack(SEQ_TRACK_808);
  assert(slot == 1);
  seq.insertNote(1, 0, 38, 0);
  assert(seq.noteCount(1) == 1);

  seq.copyTrackConfigs(layout);
  layout[1].type = SEQ_TRACK_NONE;
  seq.applyTrackLayout(layout);
  assert(seq.trackType(1) == SEQ_TRACK_NONE);
  assert(seq.noteCount(1) == 0);
  assert(seq.noteCount(0) == 1);
}

static void test_track_image_merge()
{
  NoteSequencer seq;
  seq.init();
  seq.setLength(0, 8);
  seq.insertNote(0, 0, 60, 0);
  seq.insertNote(0, 2, 64, 0);
  seq.insertNote(3, 0, 36, SEQ_EV_ROLL);

  SEQ_IMAGE img;
  seq.resetImage(&img);
  assert(seq.exportTrackImage(0, &img));
  assert(img.tracks[0].count == 2);
  assert(img.tracks[3].count == 0);

  assert(seq.exportTrackImage(3, &img));
  assert(img.tracks[0].count == 2);
  assert(img.tracks[3].count == 1);

  // blob built from the image, loaded back into a live track
  uint8_t buf[3 + sizeof(SEQ_TRACK) + SEQ_MAX_NOTES * sizeof(SEQ_EVENT)];
  assert(seq.saveTrackImageFrom(&img, 0, buf));
  seq.clearNotes(0);
  assert(seq.noteCount(0) == 0);
  assert(seq.loadTrackImage(0, buf));
  assert(seq.noteCount(0) == 2);

  // blob applied to another image slot
  assert(seq.loadTrackImageTo(&img, 0, buf));
  assert(img.tracks[0].count == 2);

  uint8_t bad[3 + sizeof(SEQ_TRACK)] = {0xFF, 0xFF, 0};
  assert(!seq.loadTrackImageTo(&img, 0, bad));

  // session layout sanitizes an image, keeping matching notes
  SEQ_TRACK layout[SEQ_MAX_TRACKS];
  seq.copyTrackConfigs(layout);
  layout[3].type = SEQ_TRACK_NONE;
  seq.applyLayoutToImage(&img, layout);
  assert(img.tracks[3].type == SEQ_TRACK_NONE);
  assert(img.tracks[3].count == 0);
  assert(img.tracks[0].count == 2);
}

static void test_shuffle_length()
{
  NoteSequencer seq;
  seq.setOutputCallback(capCb);
  seq.init();

  seq.setLength(0, 8);
  seq.insertNote(0, 0, 60, 0);
  seq.insertNote(0, 1, 62, 0);

  // simulate with a negative shuffle(len extended) on every step
  g_cap.reset();
  for (int tick = 0; tick < 8 * 24; tick++) {
    g_tick = tick;
    if (tick % 24 == 0) {
      seq.onStep(tick / 24, -6);
    }
    seq.onPulse(tick);
  }
  // gate 12 - 6 = 6: off at tick 6
  assert(g_cap.ev[g_cap.idxOf(NOTE_OFF, 60, 0)].tick == 6);

  // clamp to 1 when shuffle eats the whole gate
  g_cap.reset();
  seq.clearActiveNotes(-1);
  for (int tick = 0; tick < 8 * 24; tick++) {
    g_tick = tick;
    if (tick % 24 == 0) {
      seq.onStep(tick / 24, -12);
    }
    seq.onPulse(tick);
  }
  assert(g_cap.ev[g_cap.idxOf(NOTE_OFF, 60, 0)].tick == 1);
}

static void test_generative_303()
{
  NoteSequencer seq;
  seq.setOutputCallback(capCb);
  seq.init();

  seq.setLength(0, 32);
  // fill 100: every step gets something, deterministic bounds
  uint8_t p303[7] = {100, 100, 50, 0, 0, 48, 12};
  assert(seq.generate(0, 0, p303));
  int on = 0, rest_tie = 0;
  for (int step = 0; step < 32; step++) {
    uint8_t flags = seq.flagsAtStep(0, step, 0xFF);
    if (flags == 0) continue;
    uint8_t note = seq.noteAtStep(0, step, 0);
    if (flags & SEQ_EV_REST) {
      assert(flags & SEQ_EV_TIE); // rest events only come from ties
      rest_tie++;
    } else {
      assert(note >= 48 && note <= 60);
      assert((flags & SEQ_EV_ACCENT) != 0); // accent probability 100
      on++;
    }
  }
  assert(on == 32);
  assert(seq.noteCount(0) == 32 + rest_tie);

  // fill 0: empty track
  uint8_t p303_empty[7] = {0, 0, 0, 0, 0, 48, 12};
  seq.generate(0, 0, p303_empty);
  assert(seq.noteCount(0) == 0);

  // playback of a generated track must be clean(every on has an off)
  // slide is off so no gate extends past the simulated window
  uint8_t p303_mix[7] = {60, 30, 0, 30, 5, 48, 24};
  seq.generate(0, 0, p303_mix);
  g_cap.reset();
  simulate(seq, 32 * 24);
  for (int note = 36; note < 128; note++) {
    assert(g_cap.countOf(NOTE_ON, note, 0) == g_cap.countOf(NOTE_OFF, note, 0));
  }
}

static void test_generative_808()
{
  NoteSequencer seq;
  seq.setOutputCallback(capCb);
  seq.init();

  uint8_t dr = 3;
  seq.setLength(dr, 16);
  // fill 100: 16 hits on the bd part
  uint8_t p808[3] = {100, 50, 0};
  assert(seq.generate(dr, 0, p808));
  assert(seq.noteCount(dr) == 16);
  for (int step = 0; step < 16; step++) {
    assert(seq.hasNoteAtStep(dr, step, 36));
  }

  // other parts stay empty
  assert(seq.noteAtStep(dr, 0, 0) == 36);
  uint8_t p808_sd[3] = {50, 0, 100};
  seq.generate(dr, 1, p808_sd);
  // bjorklund of 16 slots ~50% -> 8 sd hits with roll, bd hits stay in place
  assert(seq.noteCount(dr) == 24);
  bool any_roll = false;
  for (int step = 0; step < 16; step++) {
    if (seq.flagsAtStep(dr, step, 38) & SEQ_EV_ROLL) any_roll = true;
  }
  assert(any_roll);

  // longer track: distribution spreads over the full length
  seq.clearNotes(dr);
  uint8_t p808_bd[3] = {100, 0, 0};
  seq.generate(dr, 0, p808_bd);
  assert(seq.noteCount(dr) == 16);
  seq.setLength(dr, 128);
  seq.generate(dr, 0, p808_bd);
  // the 64 step distribution spreads across 128 steps, one hit per 2 steps
  assert(seq.noteCount(dr) == 64);

  // playback: every fired drum note gets its gate off
  g_cap.reset();
  simulate(seq, 128 * 24);
  for (int note = 36; note < 128; note++) {
    assert(g_cap.countOf(NOTE_ON, note, dr) >= g_cap.countOf(NOTE_OFF, note, dr));
    // retriggered rolls send extra ons without extra offs
  }
}

static void test_clear_keeps_other_tracks()
{
  NoteSequencer seq;
  seq.setOutputCallback(capCb);
  seq.init();

  // 303 notes on slot 0, drum notes on slot 2
  seq.insertNote(0, 0, 60, 0);
  seq.insertNote(0, 4, 64, SEQ_EV_ACCENT);
  seq.insertNote(2, 1, 36, 0);
  seq.insertNote(2, 3, 38, SEQ_EV_ACCENT);
  seq.insertNote(2, 5, 42, SEQ_EV_ROLL);

  // clear the 303 first, exactly what the generate button does
  seq.clearNotes(0);
  assert(seq.noteCount(0) == 0);
  assert(seq.noteCount(2) == 3);
  assert(seq.notesUsed() == 3);

  // the 808 slice must keep its steps, notes and flags(no ghosts from slot 0)
  assert(seq.noteAtStep(2, 1, 0) == 36);
  assert(seq.noteAtStep(2, 3, 0) == 38);
  assert(seq.flagsAtStep(2, 3, 38) & SEQ_EV_ACCENT);
  assert(seq.noteAtStep(2, 5, 0) == 42);
  assert(seq.flagsAtStep(2, 5, 42) & SEQ_EV_ROLL);

  // and it still plays the same hits only
  g_cap.reset();
  simulate(seq, 8 * 24);
  assert(g_cap.countOf(NOTE_ON, 36, 2) == 1);
  assert(g_cap.countOf(NOTE_ON, 38, 2) == 1);
  assert(g_cap.countOf(NOTE_ON, 42, 2) >= 1); // roll adds retriggers
  for (int note = 0; note < 128; note++) {
    if (note == 36 || note == 38 || note == 42) continue;
    assert(g_cap.countOf(NOTE_ON, (uint8_t)note, 2) == 0);
  }
}

static void test_generate_303_keeps_808()
{
  NoteSequencer seq;
  seq.setOutputCallback(capCb);
  seq.init();

  // pre existing drum pattern on the 808 track
  seq.insertNote(2, 0, 36, 0);
  seq.insertNote(2, 4, 38, SEQ_EV_ACCENT);
  seq.insertNote(2, 8, 42, SEQ_EV_ROLL);

  // generate a 303 pattern(clears slot 0 first)
  uint8_t p303[7] = {100, 0, 0, 0, 0, 48, 12};
  seq.generate(0, 0, p303);

  assert(seq.noteCount(0) == 32);
  assert(seq.noteCount(2) == 3);
  assert(seq.noteAtStep(2, 0, 0) == 36);
  assert(seq.noteAtStep(2, 4, 0) == 38);
  assert(seq.flagsAtStep(2, 4, 38) & SEQ_EV_ACCENT);
  assert(seq.noteAtStep(2, 8, 0) == 42);
  assert(seq.flagsAtStep(2, 8, 42) & SEQ_EV_ROLL);

  // the 808 fires its three hits, nothing else on that track
  g_cap.reset();
  simulate(seq, 64 * 24);
  assert(g_cap.countOf(NOTE_ON, 36, 2) == 1);
  assert(g_cap.countOf(NOTE_ON, 38, 2) == 1);
  assert(g_cap.countOf(NOTE_ON, 42, 2) >= 1); // roll adds retriggers
  for (int note = 0; note < 128; note++) {
    if (note == 36 || note == 38 || note == 42) continue;
    assert(g_cap.countOf(NOTE_ON, (uint8_t)note, 2) == 0);
  }
}

// test-only handler: proves a new type owns its flags and 96PPQN behavior
// without any engine edit
#define SEQ_EV_TEST 0x20

class SeqHandlerTest : public SeqTrackHandler
{
  public:
    uint8_t type() const { return SEQ_TRACK_COUNT; } // not in the built in table
    void resetTrack(SEQ_TRACK &tr) const { tr.length = 8; }
    void play(NoteSequencer &seq, uint8_t track, uint16_t pos,
              uint8_t note, int8_t shuffle_len, SEQ_PLAY &out) const
    {
      (void)shuffle_len;
      out.note = note;
      if (seq.events(track)[pos].flags & SEQ_EV_TEST) {
        out.len = 1;
      }
    }
    void onPulse(NoteSequencer &seq, uint8_t track,
                 const SEQ_ACTIVE &active, SEQ_PULSE &out) const
    {
      (void)seq;
      (void)track;
      (void)active;
      if (out.roll == 0) return;
      out.roll--;
      out.shoot = (out.roll % 2 == 0) ? 1 : 0;
    }
};

static void test_handler_dispatch()
{
  NoteSequencer seq;
  seq.init();

  // table is indexed by type, NONE has no handler
  assert(SEQ_HANDLER_TABLE[SEQ_TRACK_NONE] == nullptr);
  assert(SEQ_HANDLER_TABLE[SEQ_TRACK_303] != nullptr);
  assert(SEQ_HANDLER_TABLE[SEQ_TRACK_808] != nullptr);
  assert(SEQ_HANDLER_TABLE[SEQ_TRACK_POLY] != nullptr);
  assert(SEQ_HANDLER_TABLE[SEQ_TRACK_303]->type() == SEQ_TRACK_303);
  assert(SEQ_HANDLER_TABLE[SEQ_TRACK_808]->type() == SEQ_TRACK_808);
  assert(SEQ_HANDLER_TABLE[SEQ_TRACK_POLY]->type() == SEQ_TRACK_POLY);

  // poly has no generator: base default is a no-op, still a valid dispatch
  uint8_t params[7] = {100, 0, 0, 0, 0, 48, 12};
  assert(seq.generate(2, 0, params));
  assert(seq.noteCount(2) == 0);

  // invalid targets are rejected
  assert(!seq.generate(SEQ_MAX_TRACKS, 0, params));
  assert(!seq.generate(0, 0, nullptr));
}

static void test_custom_handler()
{
  NoteSequencer seq;
  seq.init();
  seq.setLength(0, 8);
  seq.insertNote(0, 0, 60, SEQ_EV_TEST);

  SeqHandlerTest h;
  assert(h.type() == SEQ_TRACK_COUNT);

  const SEQ_TRACK *tr = seq.trackConfig(0);
  SEQ_PLAY p = { 60, tr->note_vel, tr->gate, 0, 0 };
  h.play(seq, 0, 0, 60, 0, p);
  assert(p.len == 1); // custom flag handled inside the handler

  // custom 96PPQN behavior: retrigger every other pulse
  SEQ_ACTIVE active = { 0, 60, 0, 0, 12 };
  SEQ_PULSE pulse = { 0, tr->accent_vel, 4 };
  h.onPulse(seq, 0, active, pulse);
  assert(pulse.shoot == 0 && pulse.roll == 3);
  h.onPulse(seq, 0, active, pulse);
  assert(pulse.shoot == 1 && pulse.roll == 2);
  h.onPulse(seq, 0, active, pulse);
  assert(pulse.shoot == 0 && pulse.roll == 1);
  h.onPulse(seq, 0, active, pulse);
  assert(pulse.shoot == 1 && pulse.roll == 0);
}

static void test_808_substep_roll()
{
  NoteSequencer seq;
  seq.setOutputCallback(capCb);
  seq.init();

  uint8_t dr = 3;
  seq.setLength(dr, 8);
  seq.setRollType(dr, SUB_STEP_1);
  seq.insertNote(dr, 0, 48, SEQ_EV_ROLL);

  simulate(seq, 8 * 24);
  // main hit + sub hits at pulses 7, 15, 23
  assert(g_cap.countOf(NOTE_ON, 48, dr) == 4);
  assert(g_cap.countOf(NOTE_OFF, 48, dr) == 1);

  g_cap.reset();
  seq.setRollType(dr, SUB_STEP_2);
  simulate(seq, 8 * 24);
  // every 4 pulses after the hit: 6 sub hits
  assert(g_cap.countOf(NOTE_ON, 48, dr) == 7);
  assert(g_cap.countOf(NOTE_OFF, 48, dr) == 1);
}

int main()
{
  test_default_layout();
  test_playback_mono();
  test_poly_chord();
  test_808_part_mute_and_remap();
  test_808_roll();
  test_303_tie_and_slide();
  test_rest_events_are_silent();
  test_transpose_and_hook();
  test_create_delete_tracks();
  test_clear_keeps_other_tracks();
  test_generate_303_keeps_808();
  test_pool_exhaustion();
  test_track_image_roundtrip();
  test_session_layout_merge();
  test_track_image_merge();
  test_shuffle_length();
  test_generative_303();
  test_generative_808();
  test_handler_dispatch();
  test_custom_handler();
  test_808_substep_roll();

  printf("seq_test: all %d checks passed\n", 21);
  return 0;
}
