/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Michael Chow $
   $Notice: $
   ======================================================================== */

#include "ruinenglass.h"
#include "ruinenglass_renderer.cpp"
#include "ruinenglass_audio.cpp"

extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
    Assert(sizeof(game_state) <= Memory->Permanent.Size);
    game_state *GameState = (game_state *)Memory->Permanent.Base; // STUDY: Cold-cast

    if(!GameState->IsInitialised)
    {
        GameState->Offset = {};

        // TODO(chowie): Initialise Audio State when there's proper
        // sound! Do I have to rearrange "audio -> game" to prevent
        // audio clipping at the beginning?
//        InitialiseAudioState(&GameState->AudioState);

        GameState->IsInitialised = true; // STUDY(chowie): One less thing the platform layer has to do; the game would always.
    }

    // STUDY(chowie): Shortcut to avoid call with "&" all of the time
    render_group RenderGroup_ = BeginRenderGroup(RenderCommands);
    render_group *RenderGroup = &RenderGroup_;

    //
    // NOTE: World Init & Generation
    //

    PushClear(RenderGroup, V4(0.25f, 0.25f, 0.25f, 0.0f));

//    PushRect(RenderGroup, V3(100, 100, 0), V2(100, 100), V4(1.0f, 0.5f, 0.5f, 1));
//    PushCircle(RenderGroup, V3(500, 500, 0), 50.0f, 0.5f, V4(0.5f, 1.0f, 0.5f, 1));

#define TilesPerHeight 8
#define TilesPerWidth 16

    // NOTE(chowie): Y is up rendered
    u32 TileMap[TilesPerHeight][TilesPerWidth] =
        {
            {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1},
            {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1},
            {1, 0, 0, 1,  0, 0, 0, 0,  0, 0, 0, 0,  1, 0, 0, 1},

            {1, 0, 0, 1,  0, 0, 0, 0,  0, 0, 0, 0,  1, 0, 0, 1},
            {1, 0, 0, 1,  0, 0, 0, 0,  0, 0, 0, 0,  1, 0, 0, 1},
            {1, 1, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 1, 1},

            {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1},
            {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1},
        };

    v3 Offset = V3(0, 0, 0);
    v2 TileDim = V2(75.0f, 75.0f);

    // TODO(chowie): Colour linear to srgb conversion!
    for(u32 Row = 0;
        Row < TilesPerHeight;
        ++Row)
    {
        for(u32 Column = 0;
            Column < TilesPerWidth;
            ++Column)
        {
            u32 TileID = TileMap[Row][Column];

            v4 BaseColour = V4(0.1f, 0.1f, 0.1f, 1);
            if(TileID == 1)
            {
                BaseColour = V4(0.5f, 0.7f, 0.5f, 1);
            }

            v2 Min = V2((r32)Column*TileDim.Width, (r32)Row*TileDim.Height);
            v2 Max = V2(Min.x + TileDim.Width, Min.y + TileDim.Height);

            rect2 FullTileDim = RectMinMax(Min, Max);
            PushRect(RenderGroup, Offset, FullTileDim, BaseColour);
        }
    }

    //
    // NOTE: Play World
    //

    for(u32 ControllerIndex = 0;
        ControllerIndex < ArrayCount(Input->Controllers);
        ++ControllerIndex)
    {
        game_controller_input *Controller = &Input->Controllers[ControllerIndex];
        controlled_player *ConPlayer = GameState->ControlledPlayer + ControllerIndex;

        if(Controller->IsAnalog)
        {
            ConPlayer->dP += Controller->StickAverage;
        }
        else
        {
            ZeroStruct(*ConPlayer); // NOTE(chowie): Pixels/Second
            if(IsDown(Controller->MoveUp))
            {
                ++ConPlayer->dP.y;
            }
            if(IsDown(Controller->MoveDown))
            {
                --ConPlayer->dP.y;
            }
            if(IsDown(Controller->MoveLeft))
            {
                --ConPlayer->dP.x;
            }
            if(IsDown(Controller->MoveRight))
            {
                ++ConPlayer->dP.x;
            }
            ConPlayer->dP.x *= 64.0f;
            ConPlayer->dP.y *= 64.0f;

            // TODO(chowie): Normalise diagonal movement!
            GameState->Offset += Input->dtForFrame*ConPlayer->dP;
        }
    }

    v4 HeroColour = V4(0.5f, 0.2f, 0.5f, 1);
    v2 PlayerDim = V2(0.75f*TileDim.Width, TileDim.Height);
    PushRect(RenderGroup, V3(GameState->Offset, 0), PlayerDim, HeroColour);
}

extern "C" GAME_GET_SOUND_SAMPLES(GameGetSoundSamples)
{
    game_state *GameState = (game_state *)Memory->Permanent.Base; // TODO(chowie): Replace with an dedicated Audio.Base

    // TODO(chowie): Allow sample offsets here for more robust
    // platform options!
    // TODO(chowie): OutputPlayingSounds() Mixer
    //TestOutputWilwaDialTone(&GameState->AudioState, SoundBuffer);
}

/*
internal void
RenderWeirdGradient(game_offscreen_buffer *Buffer, v2 Offset)
{
    // STUDY(chowie): Pixels are 32-bit wide, little-endian
    // architecture.
    // Memory Order:   BB GG RR xx
    // Register Order: xx RR GG BB

    u8 *Row = (u8 *)Buffer->Memory;
    for(u32 Y = 0;
        Y < Buffer->Dim.Height;
        ++Y)
    {
        u32 *Pixel = (u32 *)Row;
        for(u32 X = 0;
            X < Buffer->Dim.Width;
            ++X)
        {
            u8 Red = 0;
            u8 Blue = (u8)(X + Offset.x);
            u8 Green = (u8)(Y + Offset.y);

            // STUDY(chowie): Remember pointer arithmetic advances by
            // 4-bytes, an entire u32!
            *Pixel++ = ((Green << 16) |
                        (Blue << 0));
        }

        Row += Buffer->Pitch;
    }
}
*/
