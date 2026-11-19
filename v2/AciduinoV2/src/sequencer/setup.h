#ifndef __ACID_SEQUENCER_SETUP_H__
#define __ACID_SEQUENCER_SETUP_H__

//
// User interface config
//
#define SEQUENCER_MIN_BPM       1
#define SEQUENCER_MAX_BPM       200

//
// Note based sequencer config (shared note pool, alloc once and forever)
//
// SEQ_MAX_TRACKS: compile-time track config slots. Tracks are created/deleted
//                 at runtime from these slots (dynamic layout).
// SEQ_MAX_NOTES:  total notes shared by ALL tracks. Memory is measured per
//                 note(3 bytes), not per step: a track only costs the notes
//                 it actually holds.
// Port headers may override before including setup.h to fit low RAM boards.
//
#ifndef SEQ_MAX_TRACKS
#define SEQ_MAX_TRACKS          4 // 8
#endif
#ifndef SEQ_MAX_NOTES
#define SEQ_MAX_NOTES           64 // 512
#endif
#ifndef SEQ_MAX_ACTIVE
#define SEQ_MAX_ACTIVE          24    // simultaneous sounding notes(all tracks)
#endif

// pattern length per track is stored as a single step byte
#define SEQ_MAX_STEP            255

// 808 drum parts per 808 track
#define PARTS_808               11
#define MAX_PART_NAME_CHARS     3

// default note gate in 96PPQN pulses(24 pulses per step)
#define SEQ_DEFAULT_GATE        12    // 50% of a step, same as original tb303/tr808
#define SEQ_DEFAULT_NOTE_VEL    70
#define SEQ_DEFAULT_ACCENT_VEL  127

// default pattern lengths when a track is created
#define STEP_DEFAULT_LENGTH_303   32
#define STEP_DEFAULT_LENGTH_808   64
#define STEP_DEFAULT_LENGTH_POLY  64

//
// Default layout counts(see NoteSequencer::init).
// Tracks can be created/deleted at runtime in any slot order; the UI works
// from the live track list, not from these positions.
//
#define TRACK_NUMBER_303        2
#define TRACK_NUMBER_808        1
#define TRACK_NUMBER_POLY       1
#define VOICE_MAX_SIZE_808      PARTS_808
#define STEP_MAX_SIZE_303       STEP_DEFAULT_LENGTH_303
#define STEP_MAX_SIZE_808       STEP_DEFAULT_LENGTH_808
#define NOTE_LENGTH_303         SEQ_DEFAULT_GATE
#define NOTE_LENGTH_808         SEQ_DEFAULT_GATE
#define NOTE_VELOCITY_303       SEQ_DEFAULT_NOTE_VEL
#define NOTE_VELOCITY_808       SEQ_DEFAULT_NOTE_VEL
#define ACCENT_VELOCITY_303     SEQ_DEFAULT_ACCENT_VEL
#define ACCENT_VELOCITY_808     SEQ_DEFAULT_ACCENT_VEL
#define NOTE_STACK_SIZE_303     SEQ_MAX_ACTIVE
#define MAX_VOICE_NAME_CHARS    MAX_PART_NAME_CHARS

#endif
