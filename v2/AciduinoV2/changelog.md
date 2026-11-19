# Changelog

## 2026-11-19 — Per track type handlers

Split every track type specific behavior out of the engine into one handler
class per type, so a new type is a handler file plus one table slot.

**New files** (`v2/AciduinoV2/src/sequencer/`):

- **`seq_track_handler.h/.cpp`** — `SeqTrackHandler` base(`play`, `onPulse`,
  `generate`, `resetTrack`), `SEQ_PLAY`/`SEQ_PULSE` decisions, the shared
  303/poly acid gate and the compile time `SEQ_HANDLER_TABLE` indexed by
  `SEQ_TRACK_TYPE`. Handlers are const singletons(flash, no heap, no per track
  pointer).
- **`handler_303.h/.cpp`** — tie/slide/shuffle gate, mono same-pitch `keep`,
  and the 303 generator(was `generative_303`).
- **`handler_808.h/.cpp`** — part defaults, part mute mask check, roll flag,
  the 96PPQN roll/flam/sub step retrigger, and the Bjorklund generator(was
  `generative_808`).
- **`handler_poly.h/.cpp`** — acid gate shared with 303, no generator.

**Engine changes**:

- `NoteSequencer` only calls handlers from `onStep`(per fired event) and
  `on96PPQN`(per active note). `fireEvent` keeps the generic work(REST, track
  mute, hook, transpose, voice alloc, MIDI out); all flag and roll logic lives
  in the handler.
- `createTrack`/`initTrack` type switches → `handler->resetTrack()`.
- `generate(track, voice, params)` dispatches to the handler;
  `params[0] = fill`, `params[1..6]` are handler defined.
- `loadTrackImage` rejects a blob whose type does not match the target track
  (the type byte is now a handler table index).

**Deleted**: `generative_303.*`, `generative_808.*`(folded into the handlers).

**Verified**: `tests/seq_test.cpp` — 19 host test groups, all passing at both
512 and 64 note pools, including handler table dispatch, a test-only custom
handler with its own flag and 96PPQN behavior, and SUB_STEP roll timing.
Facade syntax checked with and without `ARDUINO`.

## 2026-11-19 — Note based sequencer (shared pool architecture)

Replaced the two step grid engines(Engine303/Engine808) with one generic MIDI
note based multi track step sequencer, a shared pre allocated note pool
(alloc once and forever), dynamic track create/delete, 303/808/poly track
types, and the generative engines ported to note producers.

### What was built

**New files** (`v2/AciduinoV2/src/sequencer/`):

- **`sequencer.h/.cpp`** — `NoteSequencer`: the generic engine. One
  pre-allocated shared pool of 512 × 3-byte note events
  (`{step, note, flags}`), packed in slot order; 8 compile time track slots
  (`SEQ_MAX_TRACKS`) with dynamic create/delete(no data movement, no heap, no
  fragmentation). Playback = per track sorted slice + monotonic cursor:
  O(1) amortized in the ISR. Flags: accent/slide/tie/roll/**rest**(off steps
  keep their data alive, exactly like the old step cells). 808 parts live in
  the track config; part mute = 16 byte 128 bit note mask(one byte read + AND
  per event at playback — the low cost part mute). `setPartNote` remaps all
  matching events + rebuilds the mask. A `noteHook` callback keeps the generic
  engine clean of 303 specific logic(the harmonizer rides on it).
- **`generative_303.h/.cpp`** — ported `acidRandomize`(fill/scale
  fix/accent/slide/tie probabilities) → note events.
- **`generative_808.h/.cpp`** — ported Bjorklund distribution + accent/roll
  per part → note events(length > 64 spreads a 64 step distribution, same
  density).
- **`seq_helpers.h`** — bit macros, ATOMIC(mirrors uClock's ESP32 mutex / AVR
  noInterrupts), portable `seq_random`.

**Rewritten**:

- `acid_sequencer.h/.cpp` — facade keeps every signature the UI calls
  (verified by grep) plus the new API:
  `createTrack/deleteTrack/trackType/is808/isPoly/getNotesUsed/getNotesFree/setGate/setTrackVoiceName`…
- `setup.h` — one knob: `SEQ_MAX_NOTES` + `SEQ_MAX_TRACKS`(legacy macros kept
  as aliases).
- `aciduino.cpp/.hpp` — storage is now one contiguous 2256 byte image +
  72 byte mute grid; the existing layout marker auto wipes/migrates old
  patterns; single track "remix" save/load via validated variable size blobs
  with factory fallback.

**Deleted**: `engine_303.*`, `engine_808.*`, `engine.h`(−1900 lines).

**Verified**: `tests/seq_test.cpp` — 14 host test groups(g++, Arduino free
engine), all passing at both 512 and 64 note pools: playback order/wrap, poly
chords, 808 part mute mask + remap, roll/flam sub hit timing(tick accurate vs
old math), tie/slide gate formulas, rest event data survival, transpose range
barrier, hook suppression, create/delete with pool repack, pool exhaustion,
storage blob round trip, shuffle length + clamp, both generators. Facade
syntax checked against an Arduino stub.

### Memory

Image 2.2 KB + active notes 144 B + scratch blob 1.6 KB ≈ **5.6 KB** static
(Teensy/ESP32: trivial).

**AVR Mega: ~1 pattern in EEPROM only mode(was 5)** — shrink via
`SEQ_MAX_NOTES`/`PARTS_808` overrides in the port header if that is
unacceptable; SD card mode is unaffected.

### Behavior changes to know (UI rework inputs)

1. **Default layout order is 303, 303, 808, poly**(slots 0–3) — chosen so the
   legacy UI's `track - TRACK_NUMBER_303` 808 math keeps working; poly
   (slot 3) has no legacy UI yet.
2. 808 per voice shift/length → track level(the step grid's shift/length
   pages now move the whole drum track).
3. Live record quantize was silently broken(`tick % 6` on a 24 tick step, + an
   out of bounds write risk at the last step) — now true 80%-of-step
   quantization with wrap.
4. Roll notes now get a proper gate NOTE_OFF(old code never sent one — hung
   note ons on poly synths).
5. `setPartNote` rejects a note another part already uses(ambiguous remap).
6. Fixed an OOB struct write in `midi_cc_controller.hpp`(`pot_map` now sized
   `SEQ_MAX_TRACKS`).
7. All old patterns are invalidated by the size marker on first boot → EEPROM
   re init / SD session factory recreate(existing migration path).

### Skipped / not checked

- On device compilation(no Teensy/AVR/ESP32 toolchain in the dev environment —
  host tests + facade syntax check only).
- UI page rework for poly/dynamic tracks(deferred, next step).
- STEP rec mode(API ready, empty branch as before).
- Per track shuffle(uClock supports it; session storage schema would change).
