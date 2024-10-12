/*!
 *  @file       harmonizer.h
 *  Project     Acid Sequencer
 *  @brief      Step Sequencer harmonizer for melodic lines
 *  @version    0.8.0
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

#ifndef __HARMONIZER_H__
#define __HARMONIZER_H__

#include <Arduino.h>

//
// MODE temperament:
//
// Eg. IONIAN WWhWWWh. = B11011100 (W) = Whole Tone (h) = half tone

// MAJOR MODES
#define IONIAN      B11011100
#define DORIAN      B10111010
#define PHRYGIAN    B01110110
#define LYDIAN      B11101100
#define MIXOLYDIAN  B11011010
#define AEOLIAN     B10110110
#define LOCRIAN     B01101110

// MINOR MODES
// I 	ascending melodic minor: WhWWWWh
#define MELODIC_MINOR	B10111100
// II 	Phrygian ♮6 (or Dorian ♭2): hWWWWhW
#define PHRYGIAN6      	B01111010
// III 	Lydian augmented (Lydian ♯5): WWWWhWh
#define LYDIAN5    		B11110100
// IV 	Lydian dominant (also, "Lydian ♭7", Acoustic scale, or Mixolydian ♯4): WWWhWhW
#define LYDIAN7      	B11101010
// V 	Mixolydian ♭6 (or melodic major or simply "fifth mode"): WWhWhWW
#define MIXOLYDIAN6  	B11010110
// VI 	Locrian ♮2 (also known as "half-diminished" scale): WhWhWWW
#define LOCRIAN2     	B10101110
// VII 	Super Locrian (also "altered dominant scale", or "altered scale"): hWhWWWW
#define SUPER_LOCRIAN  	B01011110

typedef struct
{
	uint8_t temperament; // MODE temperament: eg. Major Scale(IONIAN) WWhWWWh. = B11011100 
	const char * name;
} HARMONY_DATA; 

class HarmonizerClass
{
    public:

        HarmonizerClass() 
            {
                // temperament id 0 = IONIAN
                setTemperament(0);
            }

        uint8_t getNoteByMaxNumOfTones(uint8_t note);
        uint8_t harmonizer(uint8_t note);
        const char * getTemperamentName(uint8_t temperament_id);
        void setTemperament(uint8_t temperament_id);
        uint8_t getTemperamentId();

    private:
        uint8_t _scale[8] = {0};
        uint8_t _temperament_id = 0;
        HARMONY_DATA _harmony_mode_table[14] = {
                    // major modes
                    {IONIAN, "ionian"},
                    {DORIAN, "dorian"},
                    {PHRYGIAN, "phrygian"},         
                    {LYDIAN, "lydian"},
                    {MIXOLYDIAN, "mixolydian"},
                    {AEOLIAN, "aeolian"},    
                    {LOCRIAN, "locrian"},    
                    // minor modes
                    {MELODIC_MINOR, "melodic minor"},
                    {PHRYGIAN6, "phrygian 6"},
                    {LYDIAN5, "lydian 5"},         
                    {LYDIAN7, "lydian 7"},
                    {MIXOLYDIAN6, "mixolydian 6"},
                    {LOCRIAN2, "locrian 2"},    
                    {SUPER_LOCRIAN, "super locrian"}  
                };
};

extern HarmonizerClass Harmonizer;

#endif