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

struct game_state
{
    v2 Offset;

    audio_state AudioState;
};

#define RUINENGLASS_H
#endif
