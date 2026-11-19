/*!
 *  @file       sequencer.cpp
 *  Project     Acid Sequencer
 *  @brief      Generic MIDI note based multi-track step sequencer engine
 *  @version    1.0.0
 *  @author     Romulo Silva
 *  @license    MIT - (c) 2026 - Romulo Silva - contact@midilab.co
 */

#include <string.h>
#include <stddef.h>

#include "sequencer.h"
#include "seq_track_handler.h"

NoteSequencer::NoteSequencer()
{
  memset(&_img, 0, sizeof(_img));
  for (uint8_t i = 0; i < SEQ_MAX_ACTIVE; i++) {
    _active[i].len = -1;
  }
  _out = nullptr;
  _hook = nullptr;
}

//
// lifecycle
//

void NoteSequencer::factoryReset()
{
  for (uint8_t t = 0; t < SEQ_MAX_TRACKS; t++) {
    _img.tracks[t].type = SEQ_TRACK_NONE;
    _img.tracks[t].count = 0;
    _img.tracks[t].cursor = 0;
  }
  rebuildStarts();
  clearActiveNotes(-1);
}

void NoteSequencer::init()
{
  factoryReset();
  // default layout: 303s, then poly, then 808
  for (uint8_t i = 0; i < TRACK_NUMBER_303; i++) {
    createTrack(SEQ_TRACK_303);
  }
  for (uint8_t i = 0; i < TRACK_NUMBER_POLY; i++) {
    createTrack(SEQ_TRACK_POLY);
  }
  for (uint8_t i = 0; i < TRACK_NUMBER_808; i++) {
    createTrack(SEQ_TRACK_808);
  }
}

//
// track management (input side)
//

uint8_t NoteSequencer::createTrack(SEQ_TRACK_TYPE type)
{
  if (type == SEQ_TRACK_NONE || type >= SEQ_TRACK_COUNT) {
    return 0xFF;
  }

  const SeqTrackHandler *h = SEQ_HANDLER_TABLE[type];
  if (h == nullptr) {
    return 0xFF;
  }

  for (uint8_t t = 0; t < SEQ_MAX_TRACKS; t++) {
    if (_img.tracks[t].type != SEQ_TRACK_NONE) {
      continue;
    }

    SEQ_TRACK *tr = &_img.tracks[t];
    memset(tr, 0, sizeof(SEQ_TRACK));
    tr->type = type;
    tr->gate = SEQ_DEFAULT_GATE;
    tr->note_vel = SEQ_DEFAULT_NOTE_VEL;
    tr->accent_vel = SEQ_DEFAULT_ACCENT_VEL;
    tr->last_note = 36;
    h->resetTrack(*tr);

    rebuildStarts();
    return t;
  }

  return 0xFF;
}

bool NoteSequencer::deleteTrack(uint8_t track)
{
  if (track >= SEQ_MAX_TRACKS || _img.tracks[track].type == SEQ_TRACK_NONE) {
    return false;
  }

  clearActiveNotes(track);
  // compact the pool through the single clear path, then free the slot
  clearNotes(track);
  _img.tracks[track].type = SEQ_TRACK_NONE;
  rebuildMask(track);
  return true;
}

SEQ_TRACK_TYPE NoteSequencer::trackType(uint8_t track)
{
  if (track >= SEQ_MAX_TRACKS) {
    return SEQ_TRACK_NONE;
  }
  return (SEQ_TRACK_TYPE)_img.tracks[track].type;
}

uint8_t NoteSequencer::trackCount()
{
  uint8_t count = 0;
  for (uint8_t t = 0; t < SEQ_MAX_TRACKS; t++) {
    if (_img.tracks[t].type != SEQ_TRACK_NONE) {
      count++;
    }
  }
  return count;
}

void NoteSequencer::initTrack(uint8_t track)
{
  if (track >= SEQ_MAX_TRACKS) {
    return;
  }

  SEQ_TRACK *tr = &_img.tracks[track];
  uint8_t type = tr->type;
  if (type == SEQ_TRACK_NONE || type >= SEQ_TRACK_COUNT) {
    return;
  }

  const SeqTrackHandler *h = SEQ_HANDLER_TABLE[type];
  if (h == nullptr) {
    return;
  }

  clearActiveNotes(track);
  clearNotes(track);

  tr->shift = 0;
  tr->mute = 0;
  tr->transpose = 0;
  tr->gate = SEQ_DEFAULT_GATE;
  tr->note_vel = SEQ_DEFAULT_NOTE_VEL;
  tr->accent_vel = SEQ_DEFAULT_ACCENT_VEL;
  tr->tune = 0;
  tr->last_note = 36;
  tr->cursor = 0;
  tr->cur_step = 0;
  h->resetTrack(*tr);
  rebuildMask(track);
}

//
// pool helpers
//

uint16_t NoteSequencer::poolTotal()
{
  uint16_t total = 0;
  for (uint8_t t = 0; t < SEQ_MAX_TRACKS; t++) {
    total += _img.tracks[t].count;
  }
  return total;
}

void NoteSequencer::rebuildStarts()
{
  uint16_t off = 0;
  for (uint8_t t = 0; t < SEQ_MAX_TRACKS; t++) {
    _img.tracks[t].start = off;
    off += _img.tracks[t].count;
  }
}

SEQ_EVENT * NoteSequencer::slice(uint8_t track)
{
  return &_img.pool[_img.tracks[track].start];
}

int16_t NoteSequencer::findEvent(uint8_t track, uint8_t step, uint8_t note)
{
  SEQ_TRACK *tr = &_img.tracks[track];
  SEQ_EVENT *ev = slice(track);

  for (uint16_t i = 0; i < tr->count; i++) {
    if (ev[i].step != step) {
      continue;
    }
    if (note == 0xFF || ev[i].note == note) {
      return (int16_t)i;
    }
  }

  return -1;
}

//
// note data (input side)
//

bool NoteSequencer::insertNote(uint8_t track, uint8_t step, uint8_t note, uint8_t flags)
{
  if (track >= SEQ_MAX_TRACKS || _img.tracks[track].type == SEQ_TRACK_NONE) {
    return false;
  }
  if (step >= _img.tracks[track].length || note > 127) {
    return false;
  }
  if (poolTotal() >= SEQ_MAX_NOTES) {
    // pool full: note is dropped
    return false;
  }

  SEQ_TRACK *tr = &_img.tracks[track];
  SEQ_EVENT *ev = slice(track);

  // keep the slice sorted by step: insert before the first event with a bigger step
  uint16_t p = tr->count;
  for (uint16_t i = 0; i < tr->count; i++) {
    if (ev[i].step > step) {
      p = i;
      break;
    }
  }

  // shift this slice tail and all later slices up by one event
  uint32_t tail = (uint32_t)(poolTotal() - (tr->start + p));
  if (tail > 0) {
    memmove(&ev[p + 1], &ev[p], tail * sizeof(SEQ_EVENT));
  }

  ev[p].step = step;
  ev[p].note = note;
  ev[p].flags = flags;
  tr->count++;
  tr->last_note = note;

  if (tr->cursor > tr->count) {
    tr->cursor = tr->count;
  }

  rebuildStarts();
  return true;
}

bool NoteSequencer::deleteNote(uint8_t track, uint8_t step, uint8_t note)
{
  if (track >= SEQ_MAX_TRACKS) {
    return false;
  }

  int16_t p = findEvent(track, step, note);
  if (p < 0) {
    return false;
  }

  removeEventAt(track, (uint16_t)p);
  return true;
}

void NoteSequencer::removeEventAt(uint8_t track, uint16_t p)
{
  SEQ_TRACK *tr = &_img.tracks[track];
  if (p >= tr->count) {
    return;
  }

  SEQ_EVENT *ev = slice(track);

  // shift this slice tail and all later slices down by one event
  uint32_t tail = (uint32_t)(poolTotal() - (tr->start + p + 1));
  if (tail > 0) {
    memmove(&ev[p], &ev[p + 1], tail * sizeof(SEQ_EVENT));
  }

  tr->count--;

  if (tr->cursor > tr->count) {
    tr->cursor = tr->count;
  }

  rebuildStarts();
}

bool NoteSequencer::updateNote(uint8_t track, uint8_t step, uint8_t note)
{
  if (track >= SEQ_MAX_TRACKS || note > 127 || step >= _img.tracks[track].length) {
    return false;
  }

  SEQ_TRACK *tr = &_img.tracks[track];

  // 303 is mono: touch the first event of the step
  int16_t p = findEvent(track, step, 0xFF);
  if (p >= 0) {
    SEQ_EVENT *ev = slice(track);
    ev[p].note = note;
    tr->last_note = note;
    return true;
  }

  // no event yet on this step: keep it off with the new note ready
  return insertNote(track, step, note, SEQ_EV_REST);
}

bool NoteSequencer::upsertNoteOn(uint8_t track, uint8_t step, uint8_t note, uint8_t extra_flags, bool poly)
{
  if (track >= SEQ_MAX_TRACKS || _img.tracks[track].type == SEQ_TRACK_NONE) {
    return false;
  }
  if (step >= _img.tracks[track].length || note > 127) {
    return false;
  }

  SEQ_TRACK *tr = &_img.tracks[track];

  if (poly) {
    // chord: dedupe the same note on the same step, otherwise append
    int16_t p = findEvent(track, step, note);
    if (p >= 0) {
      SEQ_EVENT *ev = slice(track);
      ev[p].flags = (uint8_t)((ev[p].flags & ~(SEQ_EV_REST | SEQ_EV_ACCENT)) | (extra_flags & SEQ_EV_ACCENT));
      return true;
    }
    return insertNote(track, step, note, extra_flags);
  }

  int16_t p = findEvent(track, step, 0xFF);
  if (p >= 0) {
    SEQ_EVENT *ev = slice(track);
    ev[p].note = note;
    ev[p].flags = (uint8_t)((ev[p].flags & ~(SEQ_EV_ACCENT | SEQ_EV_REST)) | (extra_flags & SEQ_EV_ACCENT));
    tr->last_note = note;
    return true;
  }

  return insertNote(track, step, note, extra_flags);
}

void NoteSequencer::clearNotes(uint8_t track)
{
  if (track >= SEQ_MAX_TRACKS) {
    return;
  }

  SEQ_TRACK *tr = &_img.tracks[track];

  // slices are packed: drop this range and pull the later slices down,
  // otherwise rebuildStarts() would point them at this track's stale events
  if (tr->count > 0) {
    uint32_t tail = (uint32_t)(poolTotal() - (tr->start + tr->count));
    if (tail > 0) {
      memmove(&_img.pool[tr->start], &_img.pool[tr->start + tr->count], tail * sizeof(SEQ_EVENT));
    }
    tr->count = 0;
  }

  tr->cursor = 0;
  rebuildStarts();
}

void NoteSequencer::clearPartNotes(uint8_t track, uint8_t part)
{
  if (track >= SEQ_MAX_TRACKS || part >= PARTS_808 || _img.tracks[track].type != SEQ_TRACK_808) {
    return;
  }

  uint8_t note = _img.tracks[track].parts[part].note;
  SEQ_TRACK *tr = &_img.tracks[track];

  // walk the slice from the end and drop every event that carries this part note
  while (tr->count > 0) {
    SEQ_EVENT *ev = slice(track);
    int16_t i = (int16_t)tr->count - 1;
    while (i >= 0 && ev[i].note != note) {
      i--;
    }
    if (i < 0) {
      break;
    }
    removeEventAt(track, (uint16_t)i);
  }
}

bool NoteSequencer::hasNoteAtStep(uint8_t track, uint8_t step, uint8_t note)
{
  if (track >= SEQ_MAX_TRACKS) {
    return false;
  }

  SEQ_TRACK *tr = &_img.tracks[track];
  SEQ_EVENT *ev = slice(track);

  for (uint16_t i = 0; i < tr->count; i++) {
    if (ev[i].step == step && (note == 0xFF || ev[i].note == note)) {
      return (ev[i].flags & SEQ_EV_REST) == 0;
    }
  }

  return false;
}

uint8_t NoteSequencer::noteAtStep(uint8_t track, uint8_t step, uint8_t idx)
{
  if (track >= SEQ_MAX_TRACKS) {
    return 0xFF;
  }

  SEQ_TRACK *tr = &_img.tracks[track];
  SEQ_EVENT *ev = slice(track);
  uint8_t found = 0;

  for (uint16_t i = 0; i < tr->count; i++) {
    if (ev[i].step == step && (ev[i].flags & SEQ_EV_REST) == 0) {
      if (found == idx) {
        return ev[i].note;
      }
      found++;
    }
  }

  return 0xFF;
}

uint8_t NoteSequencer::flagsAtStep(uint8_t track, uint8_t step, uint8_t note)
{
  if (track >= SEQ_MAX_TRACKS) {
    return 0;
  }

  int16_t p = findEvent(track, step, note);
  if (p < 0) {
    return 0;
  }

  return slice(track)[p].flags;
}

bool NoteSequencer::setRestAtStep(uint8_t track, uint8_t step, bool state, uint8_t default_note, uint8_t target_note)
{
  if (track >= SEQ_MAX_TRACKS || _img.tracks[track].type == SEQ_TRACK_NONE) {
    return false;
  }
  if (step >= _img.tracks[track].length) {
    return false;
  }

  int16_t p = findEvent(track, step, target_note);
  if (p < 0) {
    // create the step data so flags/note survive a rest cycle
    return insertNote(track, step, target_note != 0xFF ? target_note : default_note, state ? SEQ_EV_REST : 0);
  }

  SEQ_EVENT *ev = slice(track);
  ev[p].flags = state ? (uint8_t)(ev[p].flags | SEQ_EV_REST) : (uint8_t)(ev[p].flags & ~SEQ_EV_REST);
  return true;
}

bool NoteSequencer::setFlagAtStep(uint8_t track, uint8_t step, uint8_t flag, bool state, uint8_t default_note, uint8_t target_note)
{
  if (track >= SEQ_MAX_TRACKS || _img.tracks[track].type == SEQ_TRACK_NONE) {
    return false;
  }
  if (step >= _img.tracks[track].length) {
    return false;
  }

  int16_t p = findEvent(track, step, target_note);
  if (p < 0) {
    // data on an off step: create it resting so it is not played until turned on
    if (!state) {
      return true;
    }
    return insertNote(track, step, target_note != 0xFF ? target_note : default_note, (uint8_t)(SEQ_EV_REST | flag));
  }

  SEQ_EVENT *ev = slice(track);
  ev[p].flags = state ? (uint8_t)(ev[p].flags | flag) : (uint8_t)(ev[p].flags & ~flag);
  return true;
}

uint8_t NoteSequencer::noteCount(uint8_t track)
{
  if (track >= SEQ_MAX_TRACKS) {
    return 0;
  }
  return (uint8_t)_img.tracks[track].count;
}

uint16_t NoteSequencer::notesUsed()
{
  return poolTotal();
}

uint16_t NoteSequencer::notesFree()
{
  uint16_t used = poolTotal();
  return (uint16_t)(used >= SEQ_MAX_NOTES ? 0 : SEQ_MAX_NOTES - used);
}

//
// track parameters
//

uint8_t NoteSequencer::getLength(uint8_t track)
{
  return (track < SEQ_MAX_TRACKS) ? _img.tracks[track].length : 0;
}

void NoteSequencer::setLength(uint8_t track, uint8_t length)
{
  if (track >= SEQ_MAX_TRACKS) {
    return;
  }
  ATOMIC(_img.tracks[track].length = (length < 2) ? 2 : (length > SEQ_MAX_STEP ? SEQ_MAX_STEP : length));
  // events beyond the new length stay in the pool but never fire(same as the old step grids)
}

int8_t NoteSequencer::getShift(uint8_t track)
{
  return (track < SEQ_MAX_TRACKS) ? _img.tracks[track].shift : 0;
}

void NoteSequencer::setShift(uint8_t track, int8_t shift)
{
  if (track < SEQ_MAX_TRACKS) {
    ATOMIC(_img.tracks[track].shift = shift);
  }
}

uint8_t NoteSequencer::getMute(uint8_t track)
{
  return (track < SEQ_MAX_TRACKS) ? _img.tracks[track].mute : 0;
}

void NoteSequencer::setMute(uint8_t track, uint8_t mute)
{
  if (track < SEQ_MAX_TRACKS) {
    ATOMIC(_img.tracks[track].mute = mute ? 1 : 0);
  }
}

int8_t NoteSequencer::getTranspose(uint8_t track)
{
  return (track < SEQ_MAX_TRACKS) ? _img.tracks[track].transpose : 0;
}

void NoteSequencer::setTranspose(uint8_t track, int8_t transpose)
{
  if (track < SEQ_MAX_TRACKS) {
    ATOMIC(_img.tracks[track].transpose = transpose);
  }
}

uint8_t NoteSequencer::getGate(uint8_t track)
{
  return (track < SEQ_MAX_TRACKS) ? _img.tracks[track].gate : SEQ_DEFAULT_GATE;
}

void NoteSequencer::setGate(uint8_t track, uint8_t gate)
{
  if (track >= SEQ_MAX_TRACKS || gate == 0) {
    return;
  }
  ATOMIC(_img.tracks[track].gate = (gate > 24) ? 24 : gate);
}

uint8_t NoteSequencer::getNoteVel(uint8_t track)
{
  return (track < SEQ_MAX_TRACKS) ? _img.tracks[track].note_vel : SEQ_DEFAULT_NOTE_VEL;
}

void NoteSequencer::setNoteVel(uint8_t track, uint8_t vel)
{
  if (track < SEQ_MAX_TRACKS) {
    ATOMIC(_img.tracks[track].note_vel = vel);
  }
}

uint8_t NoteSequencer::getAccentVel(uint8_t track)
{
  return (track < SEQ_MAX_TRACKS) ? _img.tracks[track].accent_vel : SEQ_DEFAULT_ACCENT_VEL;
}

void NoteSequencer::setAccentVel(uint8_t track, uint8_t vel)
{
  if (track < SEQ_MAX_TRACKS) {
    ATOMIC(_img.tracks[track].accent_vel = vel);
  }
}

uint8_t NoteSequencer::getTune(uint8_t track)
{
  return (track < SEQ_MAX_TRACKS) ? _img.tracks[track].tune : 0;
}

void NoteSequencer::setTune(uint8_t track, uint8_t tune)
{
  if (track < SEQ_MAX_TRACKS) {
    ATOMIC(_img.tracks[track].tune = tune);
  }
}

uint8_t NoteSequencer::getLastNote(uint8_t track)
{
  return (track < SEQ_MAX_TRACKS) ? _img.tracks[track].last_note : 36;
}

//
// 808 parts
//

int8_t NoteSequencer::partByNote(uint8_t track, uint8_t note)
{
  if (track >= SEQ_MAX_TRACKS || _img.tracks[track].type != SEQ_TRACK_808) {
    return -1;
  }

  for (uint8_t p = 0; p < PARTS_808; p++) {
    if (_img.tracks[track].parts[p].note == note) {
      return (int8_t)p;
    }
  }

  return -1;
}

uint8_t NoteSequencer::partNote(uint8_t track, uint8_t part)
{
  if (track >= SEQ_MAX_TRACKS || part >= PARTS_808) {
    return 0;
  }
  return _img.tracks[track].parts[part].note;
}

bool NoteSequencer::setPartNote(uint8_t track, uint8_t part, uint8_t note)
{
  if (track >= SEQ_MAX_TRACKS || part >= PARTS_808 || _img.tracks[track].type != SEQ_TRACK_808) {
    return false;
  }
  if (note > 127) {
    return false;
  }

  // shortcut: parts must keep distinct notes, a remap would be ambiguous
  for (uint8_t p = 0; p < PARTS_808; p++) {
    if (p != part && _img.tracks[track].parts[p].note == note) {
      return false;
    }
  }

  uint8_t old_note = _img.tracks[track].parts[part].note;
  ATOMIC(_img.tracks[track].parts[part].note = note);

  // rewrite all events that belong to this part
  if (old_note != note) {
    SEQ_TRACK *tr = &_img.tracks[track];
    SEQ_EVENT *ev = slice(track);
    for (uint16_t i = 0; i < tr->count; i++) {
      if (ev[i].note == old_note) {
        ev[i].note = note;
      }
    }
    tr->last_note = note;
  }

  rebuildMask(track);
  return true;
}

void NoteSequencer::setPartMute(uint8_t track, uint8_t part, uint8_t mute)
{
  if (track >= SEQ_MAX_TRACKS || part >= PARTS_808 || _img.tracks[track].type != SEQ_TRACK_808) {
    return;
  }
  ATOMIC(_img.tracks[track].parts[part].mute = mute ? 1 : 0);
  rebuildMask(track);
}

uint8_t NoteSequencer::partMute(uint8_t track, uint8_t part)
{
  if (track >= SEQ_MAX_TRACKS || part >= PARTS_808) {
    return 0;
  }
  return _img.tracks[track].parts[part].mute;
}

void NoteSequencer::setPartName(uint8_t track, uint8_t part, const char *name)
{
  if (track >= SEQ_MAX_TRACKS || part >= PARTS_808 || name == nullptr) {
    return;
  }

  SEQ_PART *p = &_img.tracks[track].parts[part];
  p->name[0] = name[0];
  p->name[1] = (name[1]) ? name[1] : 0;
  p->name[2] = 0;
}

const char * NoteSequencer::partName(uint8_t track, uint8_t part)
{
  if (track >= SEQ_MAX_TRACKS || part >= PARTS_808) {
    return "";
  }
  return (const char *)_img.tracks[track].parts[part].name;
}

uint8_t NoteSequencer::getRollType(uint8_t track)
{
  return (track < SEQ_MAX_TRACKS) ? _img.tracks[track].roll_type : SUB_STEP_1;
}

void NoteSequencer::setRollType(uint8_t track, uint8_t type)
{
  if (track < SEQ_MAX_TRACKS) {
    ATOMIC(_img.tracks[track].roll_type = type);
  }
}

//
// transport (uClock callbacks, fast path)
//

void NoteSequencer::onStep(uint32_t step, int8_t shuffle_len)
{
  for (uint8_t t = 0; t < SEQ_MAX_TRACKS; t++) {
    SEQ_TRACK *tr = &_img.tracks[t];
    if (tr->type == SEQ_TRACK_NONE) {
      continue;
    }

    // local step position for this track
    uint8_t s = (uint8_t)(((int32_t)step + tr->shift) % tr->length);
    tr->cur_step = s;

    if (tr->count == 0) {
      continue;
    }

    uint16_t cur = tr->cursor;
    if (cur > tr->count) {
      cur = tr->count;
    }
    // loop wrap: the cursor restarts at the beginning of the slice
    if (s == 0) {
      cur = 0;
    }

    SEQ_EVENT *ev = slice(t);

    // skip steps that are behind the playhead(amortized O(1) per pass)
    while (cur < tr->count && ev[cur].step < s) {
      cur++;
    }

    // fire every event of this step(mono single event, poly chord)
    while (cur < tr->count && ev[cur].step == s) {
      fireEvent(t, cur, shuffle_len);
      cur++;
    }

    tr->cursor = cur;
  }
}

void NoteSequencer::fireEvent(uint8_t track, uint16_t pos, int8_t shuffle_len)
{
  SEQ_TRACK *tr = &_img.tracks[track];
  SEQ_EVENT *e = &slice(track)[pos];

  // off step or muted track
  if ((e->flags & SEQ_EV_REST) != 0 || tr->mute != 0) {
    return;
  }

  // note transform hook(303 harmonizer lives in the application layer)
  int16_t note = e->note;
  if (_hook != nullptr) {
    note = (int16_t)_hook((uint8_t)note, track);
    if (note == 0xFF) {
      return;
    }
  }

  // transpose and midi range barrier
  note += tr->transpose;
  if (note < 0 || note > 127) {
    return;
  }

  // per track type flag processing lives in the handler
  uint8_t type = tr->type;
  if (type >= SEQ_TRACK_COUNT) {
    return;
  }
  const SeqTrackHandler *h = SEQ_HANDLER_TABLE[type];
  if (h == nullptr) {
    return;
  }

  SEQ_PLAY p;
  p.note = (uint8_t)note;
  p.vel = ((e->flags & SEQ_EV_ACCENT) != 0) ? tr->accent_vel : tr->note_vel;
  p.len = tr->gate;
  p.roll = 0;
  p.keep = 0;

  h->play(*this, track, pos, (uint8_t)note, shuffle_len, p);
  if (p.note == 0xFF) {
    return;
  }

  if (!allocActive(track, p.note, p.len, p.roll, p.keep)) {
    return; // no voice available: drop instead of firing an uncontrolled note
  }

  if (_out != nullptr) {
    _out(NOTE_ON, p.note, p.vel, track);
  }
}

void NoteSequencer::onPulse(uint32_t tick)
{
  (void)tick;

  for (uint8_t i = 0; i < SEQ_MAX_ACTIVE; i++) {
    SEQ_ACTIVE *a = &_active[i];
    if (a->len == -1) {
      continue;
    }

    SEQ_TRACK *tr = &_img.tracks[a->track];
    uint8_t type = tr->type;
    const SeqTrackHandler *h = (type < SEQ_TRACK_COUNT) ? SEQ_HANDLER_TABLE[type] : nullptr;

    // 96PPQN per type behavior(retriggers, sub hits)
    if (h != nullptr) {
      SEQ_PULSE p;
      p.shoot = 0;
      p.vel = tr->accent_vel;
      p.roll = a->roll;
      h->onPulse(*this, a->track, *a, p);
      a->roll = p.roll;
      if (p.shoot && _out != nullptr) {
        _out(NOTE_ON, a->note, p.vel, a->track);
      }
    }

    // gate countdown
    if (a->len > 0) {
      a->len--;
    } else if (a->len == 0) {
      // hold the slot while the roll window is open(retriggers outrun the gate)
      if (a->roll == 0) {
        if (!a->keep && _out != nullptr) {
          _out(NOTE_OFF, a->note, 0, a->track);
        }
        a->len = -1;
        a->keep = 0;
      }
    }
  }
}

bool NoteSequencer::allocActive(uint8_t track, uint8_t note, int16_t len, uint8_t roll, uint8_t keep)
{
  for (uint8_t i = 0; i < SEQ_MAX_ACTIVE; i++) {
    if (_active[i].len == -1) {
      _active[i].track = track;
      _active[i].note = note;
      _active[i].len = len;
      _active[i].roll = roll;
      _active[i].keep = keep;
      return true;
    }
  }
  return false;
}

void NoteSequencer::clearActiveNotes(int8_t track)
{
  for (uint8_t i = 0; i < SEQ_MAX_ACTIVE; i++) {
    SEQ_ACTIVE *a = &_active[i];
    if (a->len == -1) {
      continue;
    }
    if (track != -1 && a->track != (uint8_t)track) {
      continue;
    }
    if (!a->keep && _out != nullptr) {
      _out(NOTE_OFF, a->note, 0, a->track);
    }
    a->len = -1;
    a->roll = 0;
    a->keep = 0;
  }
}

uint8_t NoteSequencer::getCurrentStep(uint8_t track)
{
  static uint8_t step;
  ATOMIC(step = (track < SEQ_MAX_TRACKS) ? _img.tracks[track].cur_step : 0);
  return step;
}

//
// generative engines
//

bool NoteSequencer::generate(uint8_t track, uint8_t voice, const uint8_t *params)
{
  if (track >= SEQ_MAX_TRACKS || params == nullptr) {
    return false;
  }

  uint8_t type = _img.tracks[track].type;
  if (type == SEQ_TRACK_NONE || type >= SEQ_TRACK_COUNT) {
    return false;
  }

  const SeqTrackHandler *h = SEQ_HANDLER_TABLE[type];
  if (h == nullptr) {
    return false;
  }

  h->generate(*this, track, voice, params);
  return true;
}

//
// handler support
//

const SEQ_TRACK * NoteSequencer::trackConfig(uint8_t track) const
{
  return (track < SEQ_MAX_TRACKS) ? &_img.tracks[track] : nullptr;
}

const SEQ_EVENT * NoteSequencer::events(uint8_t track) const
{
  return &_img.pool[_img.tracks[track].start];
}

bool NoteSequencer::isNoteActive(uint8_t track, uint8_t note) const
{
  for (uint8_t i = 0; i < SEQ_MAX_ACTIVE; i++) {
    if (_active[i].len != -1 && _active[i].track == track && _active[i].note == note) {
      return true;
    }
  }
  return false;
}

bool NoteSequencer::isNoteMuted(uint8_t track, uint8_t note) const
{
  if (track >= SEQ_MAX_TRACKS || note > 127) {
    return false;
  }
  return GET_BIT(_img.mask[track][note >> 3], note & 7) != 0;
}

//
// storage
//

void * NoteSequencer::image()
{
  return (void *)&_img;
}

uint16_t NoteSequencer::imageSize()
{
  return (uint16_t)sizeof(SEQ_IMAGE);
}

bool NoteSequencer::saveTrackImage(uint8_t track, void *buf)
{
  if (track >= SEQ_MAX_TRACKS || buf == nullptr) {
    return false;
  }

  SEQ_TRACK *tr = &_img.tracks[track];
  // header holds the whole blob size so the reader can sanity check it
  uint16_t size = (uint16_t)(2 + sizeof(SEQ_TRACK) + tr->count * sizeof(SEQ_EVENT));

  uint16_t *hdr = (uint16_t *)buf;
  *hdr = size;
  memcpy((uint8_t *)buf + 2, tr, sizeof(SEQ_TRACK));
  memcpy((uint8_t *)buf + 2 + sizeof(SEQ_TRACK), slice(track), tr->count * sizeof(SEQ_EVENT));

  return true;
}

bool NoteSequencer::loadTrackImage(uint8_t track, const void *buf)
{
  if (track >= SEQ_MAX_TRACKS || buf == nullptr) {
    return false;
  }

  const uint8_t *raw = (const uint8_t *)buf;
  uint16_t size = raw[0] | ((uint16_t)raw[1] << 8);
  uint16_t count = (uint16_t)((size - 2 - sizeof(SEQ_TRACK)) / sizeof(SEQ_EVENT));

  // size sanity: never trust a blob bigger than one track config + the whole pool
  if (size < 2 + sizeof(SEQ_TRACK) || size > 2 + sizeof(SEQ_TRACK) + SEQ_MAX_NOTES * sizeof(SEQ_EVENT)) {
    return false;
  }
  if (count > (uint16_t)(SEQ_MAX_NOTES - (poolTotal() - _img.tracks[track].count))) {
    return false; // no room for the events
  }

  SEQ_TRACK *tr = &_img.tracks[track];
  SEQ_TRACK saved;
  memcpy(&saved, raw + 2, sizeof(SEQ_TRACK));

  // a blob may only be restored into a track of the same known type
  if (saved.type == SEQ_TRACK_NONE || saved.type >= SEQ_TRACK_COUNT || saved.type != tr->type) {
    return false;
  }

  // mute before touching the slice so nothing fires mid swap
  ATOMIC(tr->mute = 1);
  clearActiveNotes(track);

  uint16_t old_count = tr->count;
  uint16_t p = tr->start;

  // move later slices out of the way, write the new slice, move them back
  uint32_t tail = (uint32_t)(poolTotal() - (tr->start + old_count));
  if (tail > 0) {
    memmove(&_img.pool[tr->start + count], &_img.pool[tr->start + old_count], tail * sizeof(SEQ_EVENT));
  }
  memcpy(&_img.pool[p], raw + 2 + sizeof(SEQ_TRACK), count * sizeof(SEQ_EVENT));

  *tr = saved;
  tr->start = p;
  tr->count = count;
  tr->cursor = 0;

  rebuildStarts();
  if (tr->type == SEQ_TRACK_808) {
    rebuildMask(track);
  }
  ATOMIC(tr->mute = 0);
  return true;
}

bool NoteSequencer::saveTrackImageFrom(const SEQ_IMAGE *img, uint8_t track, void *buf)
{
  if (img == nullptr || track >= SEQ_MAX_TRACKS || buf == nullptr) {
    return false;
  }

  const SEQ_TRACK *tr = &img->tracks[track];
  // stored pool is packed in slot order: derive the slice from the counts
  uint16_t start = 0;
  for (uint8_t t = 0; t < track; t++) {
    start += img->tracks[t].count;
  }
  if ((uint32_t)start + tr->count > SEQ_MAX_NOTES) {
    return false;
  }

  uint16_t size = (uint16_t)(2 + sizeof(SEQ_TRACK) + tr->count * sizeof(SEQ_EVENT));

  uint16_t *hdr = (uint16_t *)buf;
  *hdr = size;
  memcpy((uint8_t *)buf + 2, tr, sizeof(SEQ_TRACK));
  if (tr->count > 0) {
    memcpy((uint8_t *)buf + 2 + sizeof(SEQ_TRACK), &img->pool[start], tr->count * sizeof(SEQ_EVENT));
  }

  return true;
}

bool NoteSequencer::loadTrackImageTo(SEQ_IMAGE *img, uint8_t track, const void *buf)
{
  if (img == nullptr || track >= SEQ_MAX_TRACKS || buf == nullptr) {
    return false;
  }

  const uint8_t *raw = (const uint8_t *)buf;
  uint16_t size = raw[0] | ((uint16_t)raw[1] << 8);
  if (size < 2 + sizeof(SEQ_TRACK) || size > 2 + sizeof(SEQ_TRACK) + SEQ_MAX_NOTES * sizeof(SEQ_EVENT)) {
    return false;
  }
  uint16_t count = (uint16_t)((size - 2 - sizeof(SEQ_TRACK)) / sizeof(SEQ_EVENT));

  uint16_t img_total = 0;
  uint16_t old_start = 0;
  for (uint8_t t = 0; t < SEQ_MAX_TRACKS; t++) {
    if (t < track) {
      old_start += img->tracks[t].count;
    }
    img_total += img->tracks[t].count;
  }

  SEQ_TRACK *dst = &img->tracks[track];
  uint16_t old_count = dst->count;
  if (img_total > SEQ_MAX_NOTES) {
    return false;
  }
  if ((uint32_t)old_start + old_count > img_total) {
    return false;
  }
  if ((uint32_t)old_start + count > SEQ_MAX_NOTES) {
    return false;
  }

  uint32_t tail = (uint32_t)(img_total - (old_start + old_count));
  if (tail > 0 && count != old_count) {
    memmove(&img->pool[old_start + count], &img->pool[old_start + old_count], tail * sizeof(SEQ_EVENT));
  }
  if (count > 0) {
    memcpy(&img->pool[old_start], raw + 2 + sizeof(SEQ_TRACK), count * sizeof(SEQ_EVENT));
  }

  memcpy(dst, raw + 2, sizeof(SEQ_TRACK));
  dst->start = old_start;
  dst->count = count;
  dst->cursor = 0;
  dst->cur_step = 0;

  uint16_t off = 0;
  for (uint8_t t = 0; t < SEQ_MAX_TRACKS; t++) {
    img->tracks[t].start = off;
    off += img->tracks[t].count;
  }

  memset(img->mask[track], 0, 16);
  if (dst->type == SEQ_TRACK_808) {
    for (uint8_t p = 0; p < PARTS_808; p++) {
      if (dst->parts[p].mute) {
        SET_BIT(img->mask[track][dst->parts[p].note >> 3], dst->parts[p].note & 7);
      }
    }
  }

  return true;
}

//
// session track layout
//

static void seqApplySetup(SEQ_TRACK &tr, const SEQ_TRACK &in)
{
  tr.length = (in.length < 2) ? 2 : in.length;
  tr.shift = in.shift;
  tr.transpose = in.transpose;
  tr.gate = (in.gate == 0 || in.gate > 24) ? SEQ_DEFAULT_GATE : in.gate;
  tr.note_vel = in.note_vel;
  tr.accent_vel = in.accent_vel;
  tr.tune = in.tune;
  tr.roll_type = in.roll_type;
  for (uint8_t p = 0; p < PARTS_808; p++) {
    tr.parts[p].note = in.parts[p].note;
    memcpy(tr.parts[p].name, in.parts[p].name, MAX_PART_NAME_CHARS);
  }
}

// 808 events store the part note: follow a part remap coming from the session
static void seqRemap808Events(SEQ_EVENT *ev, uint16_t count, const SEQ_PART *old_parts, const SEQ_PART *new_parts)
{
  for (uint16_t i = 0; i < count; i++) {
    for (uint8_t p = 0; p < PARTS_808; p++) {
      if (old_parts[p].note == ev[i].note) {
        ev[i].note = new_parts[p].note;
        break;
      }
    }
  }
}

void NoteSequencer::copyTrackConfigs(SEQ_TRACK *out) const
{
  if (out == nullptr) {
    return;
  }

  for (uint8_t t = 0; t < SEQ_MAX_TRACKS; t++) {
    out[t] = _img.tracks[t];
    // setup only: runtime and performance state is not session data
    out[t].start = 0;
    out[t].count = 0;
    out[t].cursor = 0;
    out[t].cur_step = 0;
    out[t].mute = 0;
    out[t].last_note = 36;
    for (uint8_t p = 0; p < PARTS_808; p++) {
      out[t].parts[p].mute = 0;
    }
  }
}

void NoteSequencer::applyTrackSetup(uint8_t track, const SEQ_TRACK &setup)
{
  if (track >= SEQ_MAX_TRACKS) {
    return;
  }

  SEQ_TRACK *tr = &_img.tracks[track];
  if (setup.type == SEQ_TRACK_NONE || setup.type != tr->type) {
    return;
  }

  SEQ_PART old_parts[PARTS_808];
  bool remap = (tr->type == SEQ_TRACK_808);
  if (remap) {
    memcpy(old_parts, tr->parts, sizeof(old_parts));
  }
  seqApplySetup(*tr, setup);
  if (remap) {
    seqRemap808Events(slice(track), tr->count, old_parts, tr->parts);
  }
  rebuildMask(track);
}

void NoteSequencer::applyLayoutToImage(SEQ_IMAGE *img, const SEQ_TRACK *layout)
{
  if (img == nullptr || layout == nullptr) {
    return;
  }

  uint16_t off = 0;
  uint16_t src_off = 0;
  for (uint8_t t = 0; t < SEQ_MAX_TRACKS; t++) {
    SEQ_TRACK *tr = &img->tracks[t];
    uint8_t desired = layout[t].type;
    bool valid = (desired > SEQ_TRACK_NONE && desired < SEQ_TRACK_COUNT);

    // stored pool is packed in slot order: derive the slice from the counts
    uint16_t src_count = tr->count;
    if ((uint32_t)src_off + src_count > SEQ_MAX_NOTES) {
      src_count = 0;
    }
    uint16_t keep = 0;

    if (valid && tr->type == desired) {
      // same track type: keep its notes and take the session setup
      keep = src_count;
      if (keep > 0) {
        memmove(&img->pool[off], &img->pool[src_off], keep * sizeof(SEQ_EVENT));
      }
      SEQ_PART old_parts[PARTS_808];
      bool remap = (tr->type == SEQ_TRACK_808);
      if (remap) {
        memcpy(old_parts, tr->parts, sizeof(old_parts));
      }
      seqApplySetup(*tr, layout[t]);
      if (remap && keep > 0) {
        seqRemap808Events(&img->pool[off], keep, old_parts, tr->parts);
      }
    } else {
      // new or dropped track: reset the slot, no events
      memset(tr, 0, sizeof(SEQ_TRACK));
      if (valid) {
        tr->type = desired;
        tr->gate = SEQ_DEFAULT_GATE;
        tr->note_vel = SEQ_DEFAULT_NOTE_VEL;
        tr->accent_vel = SEQ_DEFAULT_ACCENT_VEL;
        tr->last_note = 36;
        const SeqTrackHandler *h = SEQ_HANDLER_TABLE[desired];
        if (h != nullptr) {
          h->resetTrack(*tr);
        }
        seqApplySetup(*tr, layout[t]);
      }
    }

    tr->start = off;
    tr->count = keep;
    tr->cursor = 0;
    tr->cur_step = 0;
    // a same type track keeps its mute state; new/dropped slots were memset
    src_off += src_count;
    off += keep;

    memset(img->mask[t], 0, 16);
    if (tr->type == SEQ_TRACK_808) {
      for (uint8_t p = 0; p < PARTS_808; p++) {
        if (tr->parts[p].mute) {
          SET_BIT(img->mask[t][tr->parts[p].note >> 3], tr->parts[p].note & 7);
        }
      }
    }
  }
}

void NoteSequencer::applyTrackLayout(const SEQ_TRACK *layout)
{
  if (layout == nullptr) {
    return;
  }

  clearActiveNotes(-1);
  applyLayoutToImage(&_img, layout);
  rebuildStarts();
}

void NoteSequencer::resetImage(SEQ_IMAGE *img)
{
  if (img == nullptr) {
    return;
  }

  memset(img, 0, sizeof(SEQ_IMAGE));
  for (uint8_t t = 0; t < SEQ_MAX_TRACKS; t++) {
    img->tracks[t] = _img.tracks[t];
    img->tracks[t].start = 0;
    img->tracks[t].count = 0;
    img->tracks[t].cursor = 0;
    img->tracks[t].cur_step = 0;
    img->tracks[t].mute = 0;
    for (uint8_t p = 0; p < PARTS_808; p++) {
      img->tracks[t].parts[p].mute = 0;
    }
  }
}

bool NoteSequencer::exportTrackImage(uint8_t track, SEQ_IMAGE *img)
{
  if (track >= SEQ_MAX_TRACKS || img == nullptr) {
    return false;
  }

  const SEQ_TRACK *live = &_img.tracks[track];
  SEQ_TRACK *dst = &img->tracks[track];

  uint16_t img_total = 0;
  uint16_t old_start = 0;
  for (uint8_t t = 0; t < SEQ_MAX_TRACKS; t++) {
    if (t < track) {
      old_start += img->tracks[t].count;
    }
    img_total += img->tracks[t].count;
  }

  uint16_t old_count = dst->count;
  uint16_t new_count = live->count;

  if (img_total > SEQ_MAX_NOTES) {
    return false;
  }
  if ((uint32_t)old_start + old_count > img_total) {
    return false;
  }
  if ((uint32_t)old_start + new_count > SEQ_MAX_NOTES) {
    return false;
  }

  // shift the stored tail, write the live slice, then fix the image starts
  uint32_t tail = (uint32_t)(img_total - (old_start + old_count));
  if (tail > 0 && new_count != old_count) {
    memmove(&img->pool[old_start + new_count], &img->pool[old_start + old_count], tail * sizeof(SEQ_EVENT));
  }
  if (new_count > 0) {
    memcpy(&img->pool[old_start], &_img.pool[live->start], new_count * sizeof(SEQ_EVENT));
  }

  SEQ_TRACK saved = *live;
  saved.start = old_start;
  saved.count = new_count;
  saved.cursor = 0;
  saved.cur_step = 0;
  *dst = saved;

  uint16_t off = 0;
  for (uint8_t t = 0; t < SEQ_MAX_TRACKS; t++) {
    img->tracks[t].start = off;
    off += img->tracks[t].count;
  }

  memset(img->mask[track], 0, 16);
  if (dst->type == SEQ_TRACK_808) {
    for (uint8_t p = 0; p < PARTS_808; p++) {
      if (dst->parts[p].mute) {
        SET_BIT(img->mask[track][dst->parts[p].note >> 3], dst->parts[p].note & 7);
      }
    }
  }

  return true;
}

void NoteSequencer::rebuildMask(uint8_t track)
{
  SEQ_TRACK *tr = &_img.tracks[track];
  memset(_img.mask[track], 0, 16);

  if (tr->type != SEQ_TRACK_808) {
    return;
  }

  for (uint8_t p = 0; p < PARTS_808; p++) {
    if (tr->parts[p].mute) {
      SET_BIT(_img.mask[track][tr->parts[p].note >> 3], tr->parts[p].note & 7);
    }
  }
}

//
// output
//

void NoteSequencer::setOutputCallback(void (*cb)(uint8_t msg, uint8_t note, uint8_t vel, uint8_t track))
{
  _out = cb;
}

void NoteSequencer::setNoteHook(SEQ_NOTE_HOOK hook)
{
  _hook = hook;
}
