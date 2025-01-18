#if !defined(RUINENGLASS_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Michael Chow $
   $Notice: $
   ======================================================================== */

#include "ruinenglass_platform.h"
#include "ruinenglass_shared.h"
#include "ruinenglass_random.h"

#include "ruinenglass_world.h"
#include "ruinenglass_entity.h"
#include "ruinenglass_audio.h"

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

struct controlled_player
{
    v2 ddP;
    u32 EntityIndex;
};

// TODO(chowie): Different modes? game_mode_adventure? _creative?
struct game_state
{
    controlled_player ControlledPlayer[sizeof(game_input::Controllers)];

    u32 CameraFollowingEntityIndex;
    world_pos CameraP;

    audio_state AudioState;

    // TODO(chowie): Put arena in world?
    memory_arena WorldArena;
    world *World;

    u32 EntityCount;
    entity Entities[256];

    v2 dP;
    v2 Offset; // TODO(chowie): Remove! Offset into the chunk

    b32x IsInitialised;
};

struct transient_state
{
    memory_arena TranArena;

    b32x IsInitialised;
};

global platform_api Platform;

// STUDY(chowie): Accessors Get/Set allows for bounds-checking for an
// array-like format (passed in however you like).

#define RUINENGLASS_H
#endif
