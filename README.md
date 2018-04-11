# Aciduino

Cheap Roland TB303 Step Sequencer clone aimed for live interaction. Your acid lines for a very low cost. 

# Features

## 303 Step Sequencer clone
2 Tracks of famous Roland TB303 sequencer. Programming using rest, glide and accent on each step. The addition of step length per track you can choose from 1 step to 16 steps of pattern length to make some unsual sequences.

## Generative Engine
Generate automatic harmonized acid lines by pressing a single button and twist some harmonic parameter knobs. Just pick up a harmonic mode(scale), the low and high notes to fit the sequence and you have infinite sequences by pressing random it button!

## Midi Controller
Controls your synthetizer acid midi parameters per track up to 6 parameters.

# TB303 Step Sequencer engine clone

With some user interface changes for cheap construction of a functional TB303 engine we present you the interface:

![Image of aciduino on protoboard](https://raw.githubusercontent.com/midilab/aciduino/master/Hardware/acid_step_sequencer-protoboard-v001.png)

## User Interface

4 knobs and 6 buttons divided into 3 pages

### [midi controller]
knobs: cutoff freq./decay, resonance/accent, env mod/tunning, tempo

buttons: track 1, track 2, ctrl A, ctrl B, wave form, play/stop

### [generative]
knobs: ramdon low note, ramdon high note, harmonic mode, tempo

buttons: track 1, track 2, ramdomize it, transpose -, transpose +, play/stop

### [step edit]
knobs: octave, note, sequence length, tempo 

buttons: prev step, next step, rest, glide, accent, play/stop

### [page select]: press button1 and button2 together
knobs: none, none, none, tempo

buttons: track 1, track 2, [midi controller], [generative], [step edit], play/stop

# Dependencies

This clone is based on uClock library to generate sequencer BPM clock. The library is now part of project to get DIYers a easy compile time.

You can check more info for this library on: https://github.com/midilab/uClock

# Soon 

More documentation to come along with videos and usage examples. Get back soon!
