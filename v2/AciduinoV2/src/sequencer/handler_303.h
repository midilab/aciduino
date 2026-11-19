/*!
 *  @file       handler_303.h
 *  Project     Acid Sequencer
 *  @brief      TB303 track handler: acid gate, mono keep and generative engine
 *  @version    1.0.0
 *  @author     Romulo Silva
 *  @license    MIT - (c) 2026 - Romulo Silva - contact@midilab.co
 */

#ifndef __SEQ_HANDLER_303_H__
#define __SEQ_HANDLER_303_H__

#include "seq_track_handler.h"

class SeqHandler303 : public SeqAcidHandler
{
  public:
    uint8_t type() const;
    void resetTrack(SEQ_TRACK &tr) const;
    void play(NoteSequencer &seq, uint8_t track, uint16_t pos,
              uint8_t note, int8_t shuffle_len, SEQ_PLAY &out) const;
    // params: fill, accent, slide, tie, tones, lower note, range
    void generate(NoteSequencer &seq, uint8_t track, uint8_t voice,
                  const uint8_t *params) const;
};

#endif
