// mixolidian
uint8_t _scale[8] = { 0, 2, 4, 5, 7, 9, 10, 12 };
// phrygian
//uint8_t _scale[8] = { 0, 1, 3, 5, 7, 8, 10, 12 };

uint8_t harmonizer(uint8_t note)
{
  uint8_t octave, note_octave;
  
  octave = floor(note/12);
  note_octave = note%12;
  return (octave*12) + _scale[note_octave];
}

