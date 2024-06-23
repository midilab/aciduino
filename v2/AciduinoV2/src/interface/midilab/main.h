#ifndef __MIDILAB_INTERFACE_H__
#define __MIDILAB_INTERFACE_H__

// interface controls definitions
#include "consts.h"
// shared common gui functions used around
#include "components/shared_gui_funcs.hpp"
// page components
#include "components/topbar.hpp"
#include "components/track_length.hpp"
#include "components/sequence_shift.hpp"
#include "components/generative_fill_amount.hpp"
#include "components/track_tune_303.hpp"
#include "components/transpose.hpp"
#include "components/voice_select_808.hpp"
#include "components/voice_config_808.hpp"
#include "components/roll_type.hpp"
#include "components/midi_cc_controller.hpp"
#include "components/pattern_grid.hpp"
#include "components/pattern_mute_grid.hpp"
#include "components/generative_accent_amount.hpp"
#include "components/generative_slide_amount.hpp"
#include "components/generative_tie_amount.hpp"
#include "components/generative_tones_number.hpp"
#include "components/generative_low_octave.hpp"
#include "components/generative_range_octave.hpp"
#include "components/generative_scale_selector.hpp"
#include "components/generative_roll_amount.hpp"
#include "components/system_rec_input_src.hpp"
#include "components/system_track_output.hpp"
#include "components/sequencer_step_grid.hpp"
#include "components/system_rec_mode.hpp"
#include "components/system_resources.hpp"
#include "components/system_session_config.hpp"
#include "components/system_clock_source.hpp"
#include "components/system_shuffle.hpp"
#include "components/system_midi_channel.hpp"
#include "components/system_tempo_transport.hpp"
// pages
#include "pages/page_generative.h"
#include "pages/page_midi.h"
#include "pages/page_pattern.h"
#include "pages/page_sequencer.h"
#include "pages/page_system.h"

#endif