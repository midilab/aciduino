/*!
 *  @file       seq_track_handler.h
 *  Project     Acid Sequencer
 *  @brief      Per track type handler: flags, 96PPQN behavior and generative engine
 *
 *  One handler class owns everything that differs between track types. The
 *  engine calls a handler only from its transport callbacks:
 *    - play()    from onStep, once per fired event(flag processing)
 *    - onPulse() from on96PPQN, once per active note per pulse
 *    - generate() from the generative engine entry point
 *
 *  Handlers are const singletons in flash, referenced by SEQ_HANDLER_TABLE
 *  indexed by track type. No heap and no per track handler pointer.
 *
 *  Adding a track type:
 *    1. add its id to SEQ_TRACK_TYPE in sequencer.h(before SEQ_TRACK_COUNT)
 *    2. add handler_xxx.h/.cpp implementing SeqTrackHandler
 *    3. add the instance and its table slot in seq_track_handler.cpp
 *
 *  Event flags 0x20/0x40/0x80 are unused by the engine and free for custom
 *  handlers to interpret in play().
 *
 *  @version    1.0.0
 *  @author     Romulo Silva
 *  @license    MIT - (c) 2026 - Romulo Silva - contact@midilab.co
 */

#ifndef __SEQ_TRACK_HANDLER_H__
#define __SEQ_TRACK_HANDLER_H__

#include "sequencer.h"

// decision produced by a handler for one fired event
typedef struct
{
  uint8_t note;    // 0xFF = drop the event
  uint8_t vel;
  int16_t len;     // gate in 96PPQN pulses
  uint8_t roll;    // sub hit window in pulses, 0 = none
  uint8_t keep;    // 1 = suppress note off
} SEQ_PLAY;

// decision produced by a handler on each 96PPQN pulse of an active note
typedef struct
{
  uint8_t shoot;   // 1 = extra NOTE_ON on this pulse
  uint8_t vel;
  uint8_t roll;    // remaining roll pulses, handler owns the countdown
} SEQ_PULSE;

class SeqTrackHandler
{
  public:
    virtual ~SeqTrackHandler() {}

    // track type id, must match this handler table slot
    virtual uint8_t type() const = 0;

    // type defaults on track create/init(length, 808 parts, roll type)
    virtual void resetTrack(SEQ_TRACK &tr) const = 0;

    // flag processing for one fired event, note is after hook and transpose
    virtual void play(NoteSequencer &seq, uint8_t track, uint16_t pos,
                      uint8_t note, int8_t shuffle_len, SEQ_PLAY &out) const = 0;

    // 96PPQN per active note behavior
    // a handler that returns roll > 0 from play() must count it down here
    virtual void onPulse(NoteSequencer &seq, uint8_t track,
                         const SEQ_ACTIVE &active, SEQ_PULSE &out) const
    {
      (void)seq;
      (void)track;
      (void)active;
      (void)out;
    }

    // generative engine, params[0] = fill, params[1..6] handler defined
    virtual void generate(NoteSequencer &seq, uint8_t track, uint8_t voice,
                          const uint8_t *params) const
    {
      (void)seq;
      (void)track;
      (void)voice;
      (void)params;
    }
};

// 303/poly shared gate: tie lookahead, slide overlap, shuffle
void seq_acid_gate(NoteSequencer &seq, uint8_t track, uint16_t pos,
                   const SEQ_EVENT &ev, int8_t shuffle_len, SEQ_PLAY &out);

// 303/poly shared play behavior
class SeqAcidHandler : public SeqTrackHandler
{
  public:
    void play(NoteSequencer &seq, uint8_t track, uint16_t pos,
              uint8_t note, int8_t shuffle_len, SEQ_PLAY &out) const;
};

extern const SeqTrackHandler * const SEQ_HANDLER_TABLE[SEQ_TRACK_COUNT];

#endif
