#if !defined(RUINENGLASS_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Michael Chow $
   $Notice: $
   ======================================================================== */

//
// NOTE(chowie): Services the platform layer provides to the game
//

//
// NOTE(chowie): Services that the game provides to the platform;
// FOUR HORSEMAN - timing for a flexible framerate,
//               - controller/Keyboard input,
//               - bitmap buffer to use,
//               - sound buffer to use,
//

//
//
//

#include "ruinenglass_platform.h"
#include "ruinenglass_shared.h"
#include "ruinenglass_audio.h"
#include "ruinenglass_random.h"

// NOTE(chowie): Historical linguist
struct controlled_player
{
    v2 dP;
};

struct game_state
{
    v2 Offset; // TODO(chowie): Remove!

    audio_state AudioState;

    controlled_player ControlledPlayer[sizeof(game_input::Controllers)];

    b32x IsInitialised;
};

// STUDY(chowie): Accessors Get/Set allows for bounds-checking for an
// array-like format (passed in however you like).

#define RUINENGLASS_H
#endif
