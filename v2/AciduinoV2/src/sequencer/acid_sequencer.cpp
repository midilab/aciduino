/*!
 *  @file       acid_sequencer.cpp
 *  Project     Acid Sequencer
 *  @brief      Acid step sequencer facade over the note based sequencer engine
 *  @version    1.0.0
 *  @author     Romulo Silva
 *  @date       22/09/2022
 *  @license    MIT - (c) 2022 - Romulo Silva - contact@midilab.co
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include "acid_sequencer.h"

AcidSequencerClass *AcidSequencerClass::_self = nullptr;

AcidSequencerClass::AcidSequencerClass()
{
  _self = this;
  _out = nullptr;

  // default layout: 2x 303 tracks, 1x poly track, 1x 808 track
  _seq.init();

  // 303 harmonizer runs on note out, transparent for the other track types
  _seq.setNoteHook(noteHook);
}

// 303 harmonizer: same transform the old engines applied at read time
uint8_t AcidSequencerClass::noteHook(uint8_t note, uint8_t track)
{
  if (_self == nullptr) {
    return note;
  }
  if (_self->_seq.trackType(track) == SEQ_TRACK_303) {
    uint8_t tune = _self->_seq.getTune(track);
    if (tune > 0) {
      return (uint8_t)(Harmonizer.harmonizer(note) + (tune - 1));
    }
  }
  return note;
}

//
// transport
//

// The callback function which will be called by uClock each Pulse of 16PPQN clock resolution. Each call represents exactly one step.
void AcidSequencerClass::onStep(uint32_t step, int8_t shuffle_length_ctrl)
{
  _seq.onStep(step, shuffle_length_ctrl);
}

// The callback function which will be called by uClock each Pulse of 96PPQN clock resolution.
void AcidSequencerClass::on96PPQN(uint32_t tick)
{
  ATOMIC(_tick = tick);
  _seq.onPulse(tick);
}

void AcidSequencerClass::clearStackNote(int8_t track)
{
  _seq.clearActiveNotes(track);
}

//
// realtime rec and step rec modes support
//

void AcidSequencerClass::setRecMode(REC_MODE rec_mode)
{
  ATOMIC(_rec_mode = rec_mode);
}

REC_MODE AcidSequencerClass::getRecMode()
{
  return (REC_MODE)_rec_mode;
}

void AcidSequencerClass::setRecStatus(bool record)
{
  ATOMIC(_rec_status = record);
}

bool AcidSequencerClass::getRecStatus()
{
  return _rec_status;
}

void AcidSequencerClass::input(uint8_t track, uint8_t msg, uint8_t data1, uint8_t data2, uint8_t interrupted)
{
  SEQ_TRACK_TYPE type = _seq.trackType(track);
  if (type == SEQ_TRACK_NONE || track >= SEQ_MAX_TRACKS) {
    return;
  }

  bool accent = false;
  uint8_t note_vel = _seq.getNoteVel(track);
  uint8_t accent_vel = _seq.getAccentVel(track);

  // pass message thru for live play on selected track
  if (msg == NOTE_ON) {
    // check for accent(303 only accepts the top velocity, others the base one up)
    if (data2 >= ((type == SEQ_TRACK_303) ? ACCENT_VELOCITY_303 : NOTE_VELOCITY_808)) {
      accent = true;
    }

    // send note preview
    if (_out != nullptr) {
      _out(NOTE_ON, data1, accent ? accent_vel : note_vel, track);
    }
  } else if (msg == NOTE_OFF) {
    // send note off
    if (_out != nullptr) {
      _out(NOTE_OFF, data1, 0, track);
    }
  }

  if (_rec_status == false || _rec_mode == STEP) {
    return;
  }

  //
  // REALTIME REC MODE with quantization(24 96PPQN pulses per step)
  //
  uint8_t length = _seq.getLength(track);
  uint8_t quantized_step = _seq.getCurrentStep(track);

  uint32_t tick = 0;
  ATOMIC(tick = _tick);

  // 80% of the step(24 * 0.8 = 19.2): a hit past the boundary quantizes ahead
  if (tick % 24 > 19) {
    quantized_step = (quantized_step + 1) % length;
    // in case quantization ahead of play pointer the note only plays on the
    // next loop pass(cursor is slice relative and only advances forward)
  }

  if (msg == NOTE_ON) {
    if (type == SEQ_TRACK_808) {
      // check wich part we should record
      int8_t part = _seq.partByNote(track, data1);
      if (part >= 0) {
        setTrackVoice(track, (uint8_t)part);
        _seq.upsertNoteOn(track, quantized_step, _seq.partNote(track, (uint8_t)part), accent ? SEQ_EV_ACCENT : 0, true);
      }
    } else if (type == SEQ_TRACK_POLY) {
      // chord: accumulate notes, dedupe the same note on the same step
      _seq.upsertNoteOn(track, quantized_step, data1, accent ? SEQ_EV_ACCENT : 0, true);
    } else {
      // 303 mono: replace the note of this step, keep slide/tie
      _seq.upsertNoteOn(track, quantized_step, data1, accent ? SEQ_EV_ACCENT : 0, false);
    }
  }

  // NOTE_OFF: no explicit gate recording on the step based model
  // (poly gate from performance is a future input mode)
}

//
// pattern storage
//

void * AcidSequencerClass::getPatternImage()
{
  return _seq.image();
}

uint16_t AcidSequencerClass::getPatternImageSize()
{
  return _seq.imageSize();
}

bool AcidSequencerClass::saveTrackImage(uint8_t track, void *buf)
{
  return _seq.saveTrackImage(track, buf);
}

bool AcidSequencerClass::loadTrackImage(uint8_t track, const void *buf)
{
  return _seq.loadTrackImage(track, buf);
}

bool AcidSequencerClass::saveTrackImageFrom(const SEQ_IMAGE *img, uint8_t track, void *buf)
{
  return _seq.saveTrackImageFrom(img, track, buf);
}

bool AcidSequencerClass::loadTrackImageTo(SEQ_IMAGE *img, uint8_t track, const void *buf)
{
  return _seq.loadTrackImageTo(img, track, buf);
}

void AcidSequencerClass::copyTrackConfigs(SEQ_TRACK *out)
{
  _seq.copyTrackConfigs(out);
}

void AcidSequencerClass::applyTrackLayout(const SEQ_TRACK *layout, uint16_t reset_mask)
{
  _seq.applyTrackLayout(layout, reset_mask);
}

void AcidSequencerClass::applyTrackSetup(uint8_t track, const SEQ_TRACK &setup)
{
  _seq.applyTrackSetup(track, setup);
}

void AcidSequencerClass::applyLayoutToImage(SEQ_IMAGE *img, const SEQ_TRACK *layout, uint16_t reset_mask)
{
  _seq.applyLayoutToImage(img, layout, reset_mask);
}

void AcidSequencerClass::resetImage(SEQ_IMAGE *img)
{
  _seq.resetImage(img);
}

bool AcidSequencerClass::exportTrackImage(uint8_t track, SEQ_IMAGE *img)
{
  return _seq.exportTrackImage(track, img);
}

void AcidSequencerClass::initPatternData(int8_t track)
{
  if (track == -1) {
    // factory state for all active tracks
    for (uint8_t t = 0; t < SEQ_MAX_TRACKS; t++) {
      if (_seq.trackType(t) != SEQ_TRACK_NONE) {
        _seq.initTrack(t);
      }
    }
    return;
  }

  _seq.initTrack((uint8_t)track);
}

//
// track management
//

uint8_t AcidSequencerClass::createTrack(uint8_t type)
{
  return _seq.createTrack((SEQ_TRACK_TYPE)type);
}

bool AcidSequencerClass::deleteTrack(uint8_t track)
{
  return _seq.deleteTrack(track);
}

uint8_t AcidSequencerClass::trackType(uint8_t track)
{
  return (uint8_t)_seq.trackType(track);
}

uint8_t AcidSequencerClass::getTrackNumber()
{
  return _seq.trackCount();
}

bool AcidSequencerClass::is303(uint8_t track)
{
  return _seq.trackType(track) == SEQ_TRACK_303;
}

bool AcidSequencerClass::is808(uint8_t track)
{
  return _seq.trackType(track) == SEQ_TRACK_808;
}

bool AcidSequencerClass::isPoly(uint8_t track)
{
  return _seq.trackType(track) == SEQ_TRACK_POLY;
}

uint16_t AcidSequencerClass::getNotesUsed()
{
  return _seq.notesUsed();
}

uint16_t AcidSequencerClass::getNotesFree()
{
  return _seq.notesFree();
}

//
// generic step grid access
//

void AcidSequencerClass::rest(uint8_t track, uint8_t step, bool state)
{
  if (is808(track)) {
    // part targeted: step data of the selected part
    _seq.setRestAtStep(track, step, state, _seq.partNote(track, _voice), _seq.partNote(track, _voice));
  } else {
    _seq.setRestAtStep(track, step, state, _seq.getLastNote(track));
  }
}

bool AcidSequencerClass::stepOn(uint8_t track, uint8_t step)
{
  if (is808(track)) {
    return _seq.hasNoteAtStep(track, step, _seq.partNote(track, _voice));
  }
  return _seq.hasNoteAtStep(track, step, 0xFF);
}

void AcidSequencerClass::setStepData(uint8_t track, uint8_t step, uint8_t data)
{
  if (is808(track)) {
    // legacy behavior: editing the step note reconfigures the whole part pitch
    setTrackVoiceConfig(track, data);
  } else {
    _seq.updateNote(track, step, data);
  }
}

uint8_t AcidSequencerClass::getStepData(uint8_t track, uint8_t step)
{
  if (is808(track)) {
    // harmonizer on?
    return _seq.partNote(track, _voice);
  }

  uint8_t note = _seq.noteAtStep(track, step, 0);
  // off step with no stored note yet: show the last touched note
  if (note == 0xFF) {
    note = _seq.getLastNote(track);
  }

  // harmonizer on?
  if (is303(track) && _seq.getTune(track) > 0) {
    return (uint8_t)(Harmonizer.harmonizer(note) + (_seq.getTune(track) - 1));
  }

  return note;
}

bool AcidSequencerClass::accentOn(uint8_t track, uint8_t step)
{
  if (is808(track)) {
    return (_seq.flagsAtStep(track, step, _seq.partNote(track, _voice)) & SEQ_EV_ACCENT) != 0;
  }
  return (_seq.flagsAtStep(track, step, 0xFF) & SEQ_EV_ACCENT) != 0;
}

void AcidSequencerClass::setAccent(uint8_t track, uint8_t step, bool state)
{
  if (is808(track)) {
    _seq.setFlagAtStep(track, step, SEQ_EV_ACCENT, state, _seq.partNote(track, _voice), _seq.partNote(track, _voice));
  } else {
    _seq.setFlagAtStep(track, step, SEQ_EV_ACCENT, state, _seq.getLastNote(track));
  }
}

void AcidSequencerClass::setSlide(uint8_t track, uint8_t step, bool state)
{
  if (is303(track) || isPoly(track)) {
    _seq.setFlagAtStep(track, step, SEQ_EV_SLIDE, state, _seq.getLastNote(track));
  }
}

void AcidSequencerClass::setTie(uint8_t track, uint8_t step, bool state)
{
  if (is303(track) || isPoly(track)) {
    _seq.setFlagAtStep(track, step, SEQ_EV_TIE, state, _seq.getLastNote(track));
  }
}

bool AcidSequencerClass::slideOn(uint8_t track, uint8_t step)
{
  if (is303(track) || isPoly(track)) {
    return (_seq.flagsAtStep(track, step, 0xFF) & SEQ_EV_SLIDE) != 0;
  }
  return false;
}

bool AcidSequencerClass::tieOn(uint8_t track, uint8_t step)
{
  if (is303(track) || isPoly(track)) {
    return (_seq.flagsAtStep(track, step, 0xFF) & SEQ_EV_TIE) != 0;
  }
  return false;
}

uint8_t AcidSequencerClass::getCurrentStep(uint8_t track)
{
  return _seq.getCurrentStep(track);
}

uint16_t AcidSequencerClass::getTrackMaxLength(uint8_t track)
{
  return SEQ_MAX_STEP;
}

uint16_t AcidSequencerClass::getTrackLength(uint8_t track)
{
  return _seq.getLength(track);
}

void AcidSequencerClass::setTrackLength(uint8_t track, uint16_t length)
{
  _seq.setLength(track, (uint8_t)length);
}

void AcidSequencerClass::setShiftPos(uint8_t track, int8_t shift)
{
  _seq.setShift(track, shift);
}

int8_t AcidSequencerClass::getShiftPos(uint8_t track)
{
  return _seq.getShift(track);
}

void AcidSequencerClass::clearTrack(uint8_t track)
{
  if (is808(track)) {
    // legacy: clear the selected part only
    _seq.clearPartNotes(track, _voice);
  } else {
    _seq.clearNotes(track);
  }
}

void AcidSequencerClass::acidRandomize(uint8_t track, uint8_t fill, uint8_t param_1, uint8_t param_2, uint8_t param_3, uint8_t param_4, uint8_t param_5, uint8_t param_6)
{
  // params[0] = fill, params[1..6] are handler defined
  uint8_t params[7] = { fill, param_1, param_2, param_3, param_4, param_5, param_6 };
  _seq.generate(track, _voice, params);
}

void AcidSequencerClass::setMute(uint8_t track, uint8_t mute)
{
  _seq.setMute(track, mute);
}

uint8_t AcidSequencerClass::getMute(uint8_t track)
{
  return _seq.getMute(track);
}

//
// 303/poly specific
//

uint8_t AcidSequencerClass::getTune(uint8_t track)
{
  return _seq.getTune(track);
}

void AcidSequencerClass::setTune(uint8_t track, uint8_t tune)
{
  _seq.setTune(track, tune);
}

int8_t AcidSequencerClass::getTranspose(uint8_t track)
{
  return _seq.getTranspose(track);
}

void AcidSequencerClass::setTranspose(uint8_t track, int8_t transpose)
{
  _seq.setTranspose(track, transpose);
}

uint8_t AcidSequencerClass::getGate(uint8_t track)
{
  return _seq.getGate(track);
}

void AcidSequencerClass::setGate(uint8_t track, uint8_t gate)
{
  _seq.setGate(track, gate);
}

const char * AcidSequencerClass::getTemperamentName(uint8_t temperament_id)
{
  return Harmonizer.getTemperamentName(temperament_id);
}

void AcidSequencerClass::setTemperament(uint8_t temperament_id)
{
  Harmonizer.setTemperament(temperament_id);
}

uint8_t AcidSequencerClass::getTemperamentId()
{
  return Harmonizer.getTemperamentId();
}

const char * AcidSequencerClass::getNoteString(uint8_t note)
{
  static char szBuffer[6];
  uint8_t octave, interval;

  octave = floor(note / 12);
  interval = note % 12;

  sprintf(szBuffer, "%s%d", _note_string_table[interval], octave + 1);
  return szBuffer;
}

//
// 808 specific(parts/voices)
//

void AcidSequencerClass::setRoll(uint8_t track, uint8_t step, bool state)
{
  if (is808(track)) {
    _seq.setFlagAtStep(track, step, SEQ_EV_ROLL, state, _seq.partNote(track, _voice), _seq.partNote(track, _voice));
  }
}

void AcidSequencerClass::setRollType(uint8_t track, uint8_t type)
{
  _seq.setRollType(track, type);
}

uint8_t AcidSequencerClass::getRollType(uint8_t track)
{
  return _seq.getRollType(track);
}

bool AcidSequencerClass::rollOn(uint8_t track, uint8_t step)
{
  if (is808(track)) {
    return (_seq.flagsAtStep(track, step, _seq.partNote(track, _voice)) & SEQ_EV_ROLL) != 0;
  }
  return false;
}

void AcidSequencerClass::setTrackVoice(uint8_t track, uint8_t voice)
{
  if (voice < PARTS_808) {
    _voice = voice;
  }
}

uint8_t AcidSequencerClass::getTrackVoice(uint8_t track)
{
  return _voice;
}

void AcidSequencerClass::setTrackVoiceConfig(uint8_t track, uint8_t note)
{
  // remaps every event of the selected part to the new note
  _seq.setPartNote(track, _voice, note);
}

uint8_t AcidSequencerClass::getTrackVoiceConfig(uint8_t track)
{
  return _seq.partNote(track, _voice);
}

int8_t AcidSequencerClass::getTrackVoiceByNote(uint8_t track, uint8_t note)
{
  return _seq.partByNote(track, note);
}

const char * AcidSequencerClass::getTrackVoiceName(uint8_t track, uint8_t voice)
{
  return _seq.partName(track, voice);
}

void AcidSequencerClass::setTrackVoiceName(uint8_t track, uint8_t voice, const char *name)
{
  _seq.setPartName(track, voice, name);
}

void AcidSequencerClass::setMute(uint8_t track, uint8_t voice, uint8_t mute)
{
  // part mute rebuilds the fast 128 bit note mask used on playback
  _seq.setPartMute(track, voice, mute);
}

uint8_t AcidSequencerClass::getMute(uint8_t track, uint8_t voice)
{
  return _seq.partMute(track, voice);
}

//
// output
//

void AcidSequencerClass::setOutputCallback(void (*callback)(uint8_t msg_type, uint8_t note, uint8_t velocity, uint8_t track))
{
  _out = callback;
  _seq.setOutputCallback(callback);
}
