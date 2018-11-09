//
// MODE temperament
//
// MAJOR MODES
uint8_t _ionian[8] = { 0, 2, 4, 5, 7, 9, 11, 12 };
uint8_t _dorian[8] = { 0, 2, 3, 5, 7, 9, 10, 12 };
uint8_t _phrygian[8] = { 0, 1, 3, 5, 7, 8, 10, 12 };
uint8_t _lydian[8] = { 0, 2, 4, 6, 7, 9, 11, 12 };
uint8_t _mixolidian[8] = { 0, 2, 4, 5, 7, 9, 10, 12 };
uint8_t _aeolian[8] = { 0, 2, 3, 5, 7, 8, 10, 12 };
uint8_t _locrian[8] = { 0, 1, 3, 5, 6, 8, 10, 12 };

// MINOR MODES
// ascending melodic minor
uint8_t _melodic[8] = { 0, 2, 3, 5, 7, 9, 11, 12 };
// phrygian ♮6 (dorian ♭2)
uint8_t _phrygian6[8] = { 0, 1, 3, 5, 7, 9, 10, 12 };
// lydian augmented (lydian ♯5)
uint8_t _lydian5[8] = { 0, 2, 4, 6, 8, 9, 11, 12 };
// lydian dominant (also, "lydian ♭7", acoustic scale, or mixolydian ♯4)
uint8_t _lydian7[8] = { 0, 2, 4, 6, 7, 9, 10, 12 };
// mixolydian ♭6 (or melodic major or simply "fifth mode")
uint8_t _mixolydian6[8] = { 0, 2, 4, 5, 7, 8, 10, 12 };
// locrian ♮2 (also known as "half-diminished" scale)
uint8_t _locrian2[8] = { 0, 2, 3, 5, 6, 8, 10, 12 };
// super Locrian (also "altered dominant scale", or "altered scale")
uint8_t _super_locrian[8] = { 0, 1, 3, 4, 6, 8, 10, 12 };

uint8_t * _mode[] = {
  // MAJOR MODES
  _ionian,
  _dorian,
  _phrygian,
  _lydian,
  _mixolidian,
  _aeolian,
  _locrian,
  // MINOR MODES
  _melodic,
  _phrygian6,
  _lydian5,
  _lydian7,
  _mixolydian6,
  _locrian2,
  _super_locrian
};

#define MODES_NUMBER (sizeof(_mode) / sizeof(uint16_t)) // its array pointer we are holding here

uint8_t harmonizer(uint8_t note)
{
  uint8_t octave, interval;

  octave = floor(note/12);
  interval = floor((note%12)/1.5);
  
  return (octave*12) + _mode[_selected_mode][interval];
}

