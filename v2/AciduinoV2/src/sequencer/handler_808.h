/*!
 *  @file       handler_808.h
 *  Project     Acid Sequencer
 *  @brief      TR808 track handler: parts, part mute mask, roll and generator
 *  @version    1.0.0
 *  @author     Romulo Silva
 *  @license    MIT - (c) 2026 - Romulo Silva - contact@midilab.co
 */

#ifndef __SEQ_HANDLER_808_H__
#define __SEQ_HANDLER_808_H__

#include "seq_track_handler.h"

class SeqHandler808 : public SeqTrackHandler
{
  public:
    uint8_t type() const;
    void resetTrack(SEQ_TRACK &tr) const;
    void play(NoteSequencer &seq, uint8_t track, uint16_t pos,
              uint8_t note, int8_t shuffle_len, SEQ_PLAY &out) const;
    // roll window countdown and flam/sub step retrigger on 96PPQN
    void onPulse(NoteSequencer &seq, uint8_t track,
                 const SEQ_ACTIVE &active, SEQ_PULSE &out) const;
    // params: fill, accent, roll
    void generate(NoteSequencer &seq, uint8_t track, uint8_t voice,
                  const uint8_t *params) const;
};

#endif
