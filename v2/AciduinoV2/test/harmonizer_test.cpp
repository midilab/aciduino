// Host test. From v2/AciduinoV2:
//   c++ -std=c++17 -I test/shim -I src/sequencer test/harmonizer_test.cpp src/sequencer/harmonizer.cpp -o /tmp/harmonizer_test && /tmp/harmonizer_test
#include "harmonizer.h"
#include <cstdio>

int main()
{
    int failures = 0;

    for (uint8_t mode = 0; mode < 14; ++mode) {
        Harmonizer.setTemperament(mode);

        for (int note = 0; note < 120; ++note) {
            const int out = Harmonizer.harmonizer(note);
            const int octave_root = (note / 12) * 12;

            // result stays within its octave, allowing the next octave's root
            if (out < octave_root || out > octave_root + 12) {
                std::printf("FAIL %s note %d -> %d out of octave\n", Harmonizer.getTemperamentName(mode), note, out);
                ++failures;
            }

            // a higher input never maps to a lower output
            if (note > 0 && out < Harmonizer.harmonizer(note - 1)) {
                std::printf("FAIL %s note %d -> %d below note %d -> %d\n", Harmonizer.getTemperamentName(mode), note, out, note - 1, Harmonizer.harmonizer(note - 1));
                ++failures;
            }
        }
    }

    std::printf(failures ? "%d failure(s)\n" : "ok\n", failures);
    return failures != 0;
}
