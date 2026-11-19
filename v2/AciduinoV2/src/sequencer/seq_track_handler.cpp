/*!
 *  @file       seq_track_handler.cpp
 *  Project     Acid Sequencer
 *  @brief      Shared acid gate and the compile time handler table
 *  @version    1.0.0
 *  @author     Romulo Silva
 *  @license    MIT - (c) 2026 - Romulo Silva - contact@midilab.co
 */

#include "seq_track_handler.h"
#include "handler_303.h"
#include "handler_808.h"
#include "handler_poly.h"

// note length from gate + tie/slide lookahead + shuffle(303/poly)
void seq_acid_gate(NoteSequencer &seq, uint8_t track, uint16_t pos,
                   const SEQ_EVENT &e, int8_t shuffle_len, SEQ_PLAY &out)
{
  const SEQ_TRACK *tr = seq.trackConfig(track);
  const SEQ_EVENT *ev = seq.events(track);
  int16_t len = tr->gate;

  if ((e.flags & SEQ_EV_TIE) != 0) {
    // extend across the consecutive REST+TIE events right after this one
    uint8_t d = 0;
    uint8_t next_step = (uint8_t)(e.step + 1);
    for (uint16_t i = pos + 1; i < tr->count; i++) {
      if (ev[i].step == next_step && (ev[i].flags & SEQ_EV_REST) && (ev[i].flags & SEQ_EV_TIE)) {
        d++;
        next_step = (uint8_t)(next_step + 1);
      } else {
        break;
      }
    }
    len += 24 * d;
  } else if ((e.flags & SEQ_EV_SLIDE) != 0) {
    // slide into the next played note: overlap it by 20 pulses
    int16_t d = 0;
    for (uint16_t i = pos + 1; i < tr->count; i++) {
      if ((ev[i].flags & SEQ_EV_REST) == 0) {
        d = (int16_t)((ev[i].step - e.step + tr->length) % tr->length);
        break;
      }
    }
    // no next playable note in the slice: the slide just fills the loop tail
    if (d == 0 && tr->count > 1) {
      for (uint16_t i = 0; i < pos; i++) {
        if ((ev[i].flags & SEQ_EV_REST) == 0) {
          d = (int16_t)((ev[i].step - e.step + tr->length) % tr->length);
          break;
        }
      }
    }
    if (d > 1) {
      len += 24 * (d - 1) + 20;
    } else if (d == 1) {
      len += 20;
    }
  }

  if (shuffle_len != 0) {
    len += shuffle_len;
    if (len <= 0) {
      len = 1;
    }
  }

  out.len = len;
}

void SeqAcidHandler::play(NoteSequencer &seq, uint8_t track, uint16_t pos,
                          uint8_t note, int8_t shuffle_len, SEQ_PLAY &out) const
{
  seq_acid_gate(seq, track, pos, seq.events(track)[pos], shuffle_len, out);
  out.note = note;
}

//
// handler table: add a new type by adding its instance and slot here
//
static const SeqHandler303  _handler_303;
static const SeqHandler808  _handler_808;
static const SeqHandlerPoly _handler_poly;

const SeqTrackHandler * const SEQ_HANDLER_TABLE[SEQ_TRACK_COUNT] = {
  nullptr,        // SEQ_TRACK_NONE
  &_handler_303,  // SEQ_TRACK_303
  &_handler_808,  // SEQ_TRACK_808
  &_handler_poly  // SEQ_TRACK_POLY
};
