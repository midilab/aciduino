/*!
 *  @file       handler_poly.cpp
 *  Project     Acid Sequencer
 *  @brief      Polyphonic track handler: acid gate shared with 303, no generator
 *  @version    1.0.0
 *  @author     Romulo Silva
 *  @license    MIT - (c) 2026 - Romulo Silva - contact@midilab.co
 */

#include "handler_poly.h"

uint8_t SeqHandlerPoly::type() const
{
  return SEQ_TRACK_POLY;
}

void SeqHandlerPoly::resetTrack(SEQ_TRACK &tr) const
{
  tr.length = STEP_DEFAULT_LENGTH_POLY;
}
