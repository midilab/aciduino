#ifndef __ACID_SEQUENCER_SETUP_H__
#define __ACID_SEQUENCER_SETUP_H__

// setup how many 303 and 808s you need
// 303 its indexed as 0 to TRACK_NUMBER_303-1, so 808 follows the index
#define TRACK_NUMBER_303        4
#define TRACK_NUMBER_808        1

//
// User interface config
//
#define SEQUENCER_MIN_BPM       1
#define SEQUENCER_MAX_BPM       200

//
// Sequencer config
//
// 303
#define STEP_MAX_SIZE_303       32 // UI is prepared for max 256 steps, but you can go further until the memory blow up
#define NOTE_LENGTH_303         12 // min: 1 max: 23 DO NOT EDIT BEYOND!!! 12 = 50% on 96ppqn, same as original tb303. 62.5% for triplets time signature

#define NOTE_VELOCITY_303       70
#define ACCENT_VELOCITY_303     127
#define NOTE_STACK_SIZE_303     3
// 808
#define STEP_MAX_SIZE_808       64 // max 64 steps always! do not go beyond!
#define VOICE_MAX_SIZE_808      11 // max 11! DO NOT GO BEYOND!!!

#define NOTE_LENGTH_808         12 // min: 1 max: 23 DO NOT EDIT BEYOND!!! 96PPQN has 24 pulses in between notes
#define NOTE_VELOCITY_808       70
#define ACCENT_VELOCITY_808     127
#define MAX_VOICE_NAME_CHARS    3
//#define GLOBAL_ACCENT           // if you choose to go with global accent you can save some ram and epprom/sdcard in size for pattern

#endif

