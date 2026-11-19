/*!
 *  @file       handler_303.cpp
 *  Project     Acid Sequencer
 *  @brief      TB303 track handler: acid gate, mono keep and generative engine
 *  @version    1.0.0
 *  @author     Romulo Silva
 *  @license    MIT - (c) 2026 - Romulo Silva - contact@midilab.co
 */

#include "handler_303.h"

uint8_t SeqHandler303::type() const
{
  return SEQ_TRACK_303;
}

void SeqHandler303::resetTrack(SEQ_TRACK &tr) const
{
  tr.length = STEP_DEFAULT_LENGTH_303;
}

void SeqHandler303::play(NoteSequencer &seq, uint8_t track, uint16_t pos,
                         uint8_t note, int8_t shuffle_len, SEQ_PLAY &out) const
{
  SeqAcidHandler::play(seq, track, pos, note, shuffle_len, out);

  // mono: slide into the same pitch is a single gate, drop the note off of the
  // still sounding note and let the new event own the release
  if (seq.isNoteActive(track, note)) {
    out.keep = 1;
  }
}

void SeqHandler303::generate(NoteSequencer &seq, uint8_t track, uint8_t voice,
                             const uint8_t *params) const
{
  (void)voice;

  if (params == nullptr || seq.trackType(track) != SEQ_TRACK_303) {
    return;
  }

  uint8_t fill = params[0];
  uint8_t accent_probability = params[1];
  uint8_t slide_probability = params[2];
  uint8_t tie_probability = params[3];
  uint8_t number_of_tones = params[4];
  uint8_t lower_note = params[5];
  uint8_t range_note = params[6];

  // initialize as full tone scale array(root c)
  uint8_t fix_tones[12] = {0};

  // prepare the allowed notes for number of tones parameter
  // fix notes outside number of tones choosed and pre computed
  if (number_of_tones > 0) {
    // add offset to fix number of tones requested
    // [0, -1, -2, -3, -4, 0, -1, -2, -3, -4, -5, -6]
    int8_t fix_counter = 0;
    int8_t fix_mod = 12 / number_of_tones;
    for (uint8_t i = 0; i < 12; i++) {
      if (!(i % fix_mod) || i == 0) {
        fix_tones[i] = 0;
        fix_counter = 0;
      } else {
        --fix_counter;
        fix_tones[i] = fix_counter;
      }
    }
  }

  uint8_t length = seq.getLength(track);

  // mute while regenerating so nothing fires mid rewrite
  seq.setMute(track, 1);
  seq.clearNotes(track);

  for (uint16_t step = 0; step < length; step++) {

    // step on/off
    bool rest = seq_random(0, 100) < fill ? false : true;

    // random tie and reset accent and slide in case of a rest
    if (rest) {
      uint8_t last_step = (step == 0) ? length - 1 : (uint8_t)(step - 1);

      // only tie probability when last step has a note or another tie event
      bool last_note = seq.hasNoteAtStep(track, last_step, 0xFF);
      bool last_tie = (seq.flagsAtStep(track, last_step, 0) & SEQ_EV_TIE) != 0;
      if (last_note || last_tie) {
        if (seq_random(0, 100) < tie_probability) {
          seq.setFlagAtStep(track, (uint8_t)step, SEQ_EV_TIE, true, seq.getLastNote(track));
        }
      }
      continue;
    }

    uint8_t high_note = lower_note + range_note;
    if (high_note > 127) {
      high_note = 127;
    }

    uint8_t note = seq_random(lower_note, high_note);
    // fix notes outside number of tones choosed and pre computed
    if (number_of_tones > 0) {
      note += fix_tones[note % 12];
      if (note > 127) {
        note = 127;
      }
    }

    uint8_t flags = 0;
    if (seq_random(0, 100) < accent_probability) {
      flags |= SEQ_EV_ACCENT;
    }
    if (seq_random(0, 100) < slide_probability) {
      flags |= SEQ_EV_SLIDE;
    }

    seq.insertNote(track, (uint8_t)step, note, flags);
  }

  seq.setMute(track, 0);
}
