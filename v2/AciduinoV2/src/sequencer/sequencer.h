/*!
 *  @file       sequencer.h
 *  Project     Acid Sequencer
 *  @brief      Generic MIDI note based multi-track step sequencer engine
 *
 *  Memory model: one pre-allocated note pool(alloc once and forever) shared by
 *  all tracks. Each track owns a packed range of the pool and its events are
 *  sorted by step. Playback walks each track with a slice-relative cursor, so
 *  reading is O(1) amortized in the clock callback while inserts/delete are
 *  O(pool) and only happen in main loop context.
 *
 *  The engine is transport only: it stores MIDI notes, fires them on the uClock
 *  step/pulse callbacks and handles track level parameters(mute, shift,
 *  transpose, gate), 808 style drum parts(with fast note mute mask) and poly
 *  chords. Per track type flag processing, 96PPQN behavior and generative
 *  engines live in the SeqTrackHandler implementations(seq_track_handler.h).
 *
 *  @version    1.0.0
 *  @author     Romulo Silva
 *  @license    MIT - (c) 2026 - Romulo Silva - contact@midilab.co
 */

#ifndef __SEQ_SEQUENCER_H__
#define __SEQ_SEQUENCER_H__

#include <stdint.h>
#include <stdbool.h>

#include "setup.h"
#include "seq_helpers.h"

//
// note event flags
//
#define SEQ_EV_ACCENT  0x01
#define SEQ_EV_SLIDE   0x02
#define SEQ_EV_TIE     0x04
#define SEQ_EV_ROLL    0x08
#define SEQ_EV_REST    0x10    // step off(keeps step data alive for 303/808 step grids)

//
// track types
//
typedef enum {
  SEQ_TRACK_NONE = 0,
  SEQ_TRACK_303,
  SEQ_TRACK_808,
  SEQ_TRACK_POLY,
  SEQ_TRACK_COUNT   // number of built in types, handler table size
} SEQ_TRACK_TYPE;

//
// 808 roll/flam types: pulse location of the sub hit within a 24 pulse step
//
typedef enum {
  FLAM_1 = 1,
  FLAM_2 = 3,
  FLAM_3 = 6,
  FLAM_4 = 9,
  FLAM_5 = 13,
  SUB_STEP_1,
  SUB_STEP_2,
} ROLL_TYPE;

typedef struct
{
  uint8_t step;    // 0..SEQ_MAX_STEP-1
  uint8_t note;    // 0..127
  uint8_t flags;   // SEQ_EV_*
} SEQ_EVENT;      // 3 bytes per note

typedef struct
{
  uint8_t note;
  uint8_t mute;
  char    name[MAX_PART_NAME_CHARS];
} SEQ_PART;       // 808 drum part

typedef struct
{
  // config
  uint8_t  type;        // SEQ_TRACK_TYPE
  uint8_t  length;      // pattern length in steps (2..SEQ_MAX_STEP)
  int8_t   shift;       // step offset
  uint8_t  mute;
  int8_t   transpose;
  uint8_t  gate;        // default note length in 96PPQN pulses
  uint8_t  note_vel;
  uint8_t  accent_vel;
  uint8_t  tune;        // 303 harmonizer tune (0 = off)
  uint8_t  last_note;   // last note touched on this track (off step editing)
  // shared pool range (packed in slot order)
  uint16_t start;       // first pool index of this track events
  uint16_t count;       // number of events in range
  // playback
  uint16_t cursor;      // slice relative read cursor
  uint8_t  cur_step;    // current local step(playback position)
  // 808
  SEQ_PART parts[PARTS_808];
  uint8_t  roll_type;
} SEQ_TRACK;

// the whole sequencer state is one contiguous image: save/load it as a block
typedef struct
{
  SEQ_TRACK tracks[SEQ_MAX_TRACKS];
  SEQ_EVENT pool[SEQ_MAX_NOTES];
  uint8_t   mask[SEQ_MAX_TRACKS][16];  // 128 bit midi note mute mask(808 parts)
} SEQ_IMAGE;

// runtime active notes(not part of the image)
typedef struct
{
  uint8_t track;
  uint8_t note;    // note that actually fired(after hook/transpose)
  uint8_t keep;    // 1 = suppress note off(303 same pitch gate)
  uint8_t roll;    // pulses left in roll window (0 = none)
  int16_t len;     // gate countdown, -1 = slot empty
} SEQ_ACTIVE;

// note transform hook: return transformed note or 0xFF to suppress the note
typedef uint8_t (*SEQ_NOTE_HOOK)(uint8_t note, uint8_t track);

class NoteSequencer
{
  public:
    NoteSequencer();

    //
    // lifecycle
    //
    void init();                  // default layout: 2x303, 1x808, 1xpoly
    void factoryReset();          // all slots empty

    //
    // track management (input side, can be slow)
    //
    uint8_t createTrack(SEQ_TRACK_TYPE type);   // slot index or 0xFF
    bool    deleteTrack(uint8_t track);
    SEQ_TRACK_TYPE trackType(uint8_t track);
    uint8_t trackCount();
    void    initTrack(uint8_t track);          // factory state, keeps type

    //
    // transport (uClock callbacks, ISR context, fast path)
    //
    void onStep(uint32_t step, int8_t shuffle_len);
    void onPulse(uint32_t tick);
    uint8_t getCurrentStep(uint8_t track);
    void    clearActiveNotes(int8_t track = -1);

    //
    // note data (input side, can be slow)
    //
    bool insertNote(uint8_t track, uint8_t step, uint8_t note, uint8_t flags = 0);
    bool deleteNote(uint8_t track, uint8_t step, uint8_t note = 0xFF); // 0xFF = any
    bool updateNote(uint8_t track, uint8_t step, uint8_t note);        // create REST event if missing
    bool upsertNoteOn(uint8_t track, uint8_t step, uint8_t note, uint8_t extra_flags, bool poly = false);
    void clearNotes(uint8_t track);
    void clearPartNotes(uint8_t track, uint8_t part);

    uint8_t  noteCount(uint8_t track);
    bool     hasNoteAtStep(uint8_t track, uint8_t step, uint8_t note = 0xFF);
    uint8_t  noteAtStep(uint8_t track, uint8_t step, uint8_t idx = 0);   // 0xFF when missing
    uint8_t  flagsAtStep(uint8_t track, uint8_t step, uint8_t note = 0xFF); // 0 when missing
    bool     setRestAtStep(uint8_t track, uint8_t step, bool state, uint8_t default_note, uint8_t target_note = 0xFF);
    bool     setFlagAtStep(uint8_t track, uint8_t step, uint8_t flag, bool state, uint8_t default_note, uint8_t target_note = 0xFF);

    uint16_t notesUsed();
    uint16_t notesFree();

    //
    // track parameters
    //
    uint8_t getLength(uint8_t track);
    void    setLength(uint8_t track, uint8_t length);
    int8_t  getShift(uint8_t track);
    void    setShift(uint8_t track, int8_t shift);
    uint8_t getMute(uint8_t track);
    void    setMute(uint8_t track, uint8_t mute);
    int8_t  getTranspose(uint8_t track);
    void    setTranspose(uint8_t track, int8_t transpose);
    uint8_t getGate(uint8_t track);
    void    setGate(uint8_t track, uint8_t gate);
    uint8_t getNoteVel(uint8_t track);
    void    setNoteVel(uint8_t track, uint8_t vel);
    uint8_t getAccentVel(uint8_t track);
    void    setAccentVel(uint8_t track, uint8_t vel);
    uint8_t getTune(uint8_t track);
    void    setTune(uint8_t track, uint8_t tune);
    uint8_t getLastNote(uint8_t track);

    //
    // 808 parts (voice)
    //
    int8_t  partByNote(uint8_t track, uint8_t note);
    uint8_t partNote(uint8_t track, uint8_t part);
    bool    setPartNote(uint8_t track, uint8_t part, uint8_t note); // remaps track events
    void    setPartMute(uint8_t track, uint8_t part, uint8_t mute); // rebuilds fast mute mask
    uint8_t partMute(uint8_t track, uint8_t part);
    void    setPartName(uint8_t track, uint8_t part, const char *name);
    const char * partName(uint8_t track, uint8_t part);
    uint8_t getRollType(uint8_t track);
    void    setRollType(uint8_t track, uint8_t type);

    //
    // storage (whole image is contiguous)
    //
    void *    image();
    uint16_t  imageSize();
    uint32_t  trackImageOffset(uint8_t track); // byte offset of the track slice inside the image
    bool      saveTrackImage(uint8_t track, void *buf);     // blob: [u16 size][SEQ_TRACK][events]
    bool      loadTrackImage(uint8_t track, const void *buf);

    //
    // generative engines(dispatched to the track type handler)
    //
    bool generate(uint8_t track, uint8_t voice, const uint8_t *params);

    //
    // handler support(read only, used by SeqTrackHandler implementations)
    //
    const SEQ_TRACK * trackConfig(uint8_t track) const;
    const SEQ_EVENT * events(uint8_t track) const;
    bool isNoteActive(uint8_t track, uint8_t note) const;
    bool isNoteMuted(uint8_t track, uint8_t note) const;

    //
    // output
    //
    void setOutputCallback(void (*cb)(uint8_t msg, uint8_t note, uint8_t vel, uint8_t track));
    void setNoteHook(SEQ_NOTE_HOOK hook);

  private:
    SEQ_IMAGE   _img;
    SEQ_ACTIVE  _active[SEQ_MAX_ACTIVE];
    void (*_out)(uint8_t msg, uint8_t note, uint8_t vel, uint8_t track);
    SEQ_NOTE_HOOK _hook;

    uint16_t poolTotal();
    void     rebuildStarts();
    void     rebuildMask(uint8_t track);
    SEQ_EVENT * slice(uint8_t track);
    int16_t  findEvent(uint8_t track, uint8_t step, uint8_t note); // slice relative, -1 when missing
    void     removeEventAt(uint8_t track, uint16_t slice_pos);
    void     fireEvent(uint8_t track, uint16_t pos, int8_t shuffle_len);
    bool     allocActive(uint8_t track, uint8_t note, int16_t len, uint8_t roll, uint8_t keep);
};

#endif
