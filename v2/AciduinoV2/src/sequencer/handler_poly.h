/*!
 *  @file       handler_poly.h
 *  Project     Acid Sequencer
 *  @brief      Polyphonic track handler: acid gate shared with 303, no generator
 *  @version    1.0.0
 *  @author     Romulo Silva
 *  @license    MIT - (c) 2026 - Romulo Silva - contact@midilab.co
 */

#ifndef __SEQ_HANDLER_POLY_H__
#define __SEQ_HANDLER_POLY_H__

#include "seq_track_handler.h"

class SeqHandlerPoly : public SeqAcidHandler
{
  public:
    uint8_t type() const;
    void resetTrack(SEQ_TRACK &tr) const;
};

#endif
