/*!
 *  @file       handler_808.cpp
 *  Project     Acid Sequencer
 *  @brief      TR808 track handler: parts, part mute mask, roll and generator
 *  @version    1.0.0
 *  @author     Romulo Silva
 *  @license    MIT - (c) 2026 - Romulo Silva - contact@midilab.co
 */

#include <math.h>

#include "handler_808.h"
#include "bjorklund.h"

// default 808 part setup (general midi drum map)
static const struct { const char *name; uint8_t note; } _default_parts_808[PARTS_808] = {
  {"bd", 36}, {"sd", 38}, {"lt", 41}, {"mt", 45}, {"ht", 48},
  {"rs", 37}, {"cp", 39}, {"cb", 51}, {"cy", 49}, {"oh", 46}, {"ch", 42}
};

uint8_t SeqHandler808::type() const
{
  return SEQ_TRACK_808;
}

void SeqHandler808::resetTrack(SEQ_TRACK &tr) const
{
  tr.length = STEP_DEFAULT_LENGTH_808;
  tr.roll_type = SUB_STEP_1;

  for (uint8_t p = 0; p < PARTS_808; p++) {
    tr.parts[p].note = _default_parts_808[p].note;
    tr.parts[p].mute = 0;
    tr.parts[p].name[0] = _default_parts_808[p].name[0];
    tr.parts[p].name[1] = _default_parts_808[p].name[1];
    tr.parts[p].name[2] = 0;
  }
}

void SeqHandler808::play(NoteSequencer &seq, uint8_t track, uint16_t pos,
                         uint8_t note, int8_t shuffle_len, SEQ_PLAY &out) const
{
  (void)shuffle_len;

  const SEQ_EVENT &e = seq.events(track)[pos];

  // part mute: one byte check on the 128 bit note mask
  if (seq.isNoteMuted(track, e.note)) {
    out.note = 0xFF;
    return;
  }

  // roll: extra sub step hits inside the 24 pulse window
  if ((e.flags & SEQ_EV_ROLL) != 0) {
    out.roll = 24;
  }

  out.note = note;
}

void SeqHandler808::onPulse(NoteSequencer &seq, uint8_t track,
                            const SEQ_ACTIVE &active, SEQ_PULSE &out) const
{
  (void)active;

  if (out.roll == 0) {
    return;
  }

  out.roll--;

  uint8_t rt = seq.trackConfig(track)->roll_type;
  if (rt <= FLAM_5) {
    out.shoot = (out.roll == rt) ? 1 : 0;
  } else if (rt == SUB_STEP_1) {
    out.shoot = (out.roll % 8 == 0) ? 1 : 0;
  } else if (rt == SUB_STEP_2) {
    out.shoot = (out.roll % 4 == 0) ? 1 : 0;
  }
}

void SeqHandler808::generate(NoteSequencer &seq, uint8_t track, uint8_t voice,
                             const uint8_t *params) const
{
  if (params == nullptr || seq.trackType(track) != SEQ_TRACK_808 || voice >= PARTS_808) {
    return;
  }

  uint8_t fill = params[0];
  uint8_t accent_probability = params[1];
  uint8_t roll_probability = params[2];

  Bjorklund<uint64_t, 10> bjorklund;

  uint8_t length = seq.getLength(track);
  // Bjorklund is bounded to 64 step distributions: longer tracks spread a
  // 64 step distribution across the full length
  uint8_t dist_length = (length > 64) ? 64 : length;
  uint8_t pulses = (uint8_t)(ceil(dist_length * (float)(fill / 100.0)));

  uint64_t bits = bjorklund.compute((int8_t)dist_length, (int8_t)pulses);

  uint8_t note = seq.partNote(track, voice);

  // mute while regenerating so nothing fires mid rewrite
  seq.setMute(track, 1);
  seq.clearPartNotes(track, voice);

  for (uint16_t i = 0; i < dist_length; i++) {
    if (GET_BIT(bits, i) == 0) {
      continue;
    }

    uint8_t step = (length > 64) ? (uint8_t)(i * length / 64) : (uint8_t)i;

    uint8_t flags = 0;
    if (seq_random(0, 100) < accent_probability) {
      flags |= SEQ_EV_ACCENT;
    }
    if (seq_random(0, 100) < roll_probability) {
      flags |= SEQ_EV_ROLL;
    }

    // dedupe by (step, part note): other parts can share the same step
    seq.upsertNoteOn(track, step, note, flags, true);
  }

  seq.setMute(track, 0);
}
