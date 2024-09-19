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

struct world_chunk
{
    v3s Chunk;
    v3 Offset_;
};

struct world
{
    memory_arena Arena;

    world_chunk Chunk;
};

extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
    Assert(sizeof(game_state) <= Memory->Permanent.Size);
    game_state *GameState = (game_state *)Memory->Permanent.Base; // STUDY(chowie): Cold-cast

    if(!GameState->IsInitialised)
    {
        GameState->Offset = V2(500.0f, 500.0f);

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
    PushCircle(RenderGroup, V3(600, 600, 0), 50.0f, 24, V4(0.5f, 1.0f, 0.5f, 0.5f));

#define TilesPerHeight 4
#define TilesPerWidth 8

    /*
    // NOTE(chowie): Y is up rendered
    u32 TileMap[2*TilesPerHeight][2*TilesPerWidth] =
        {
            {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1},
            {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1},
            {1, 0, 0, 1,  0, 0, 0, 0,  0, 0, 0, 0,  1, 0, 0, 1},

            {1, 0, 0, 1,  0, 0, 0, 0,  0, 0, 0, 0,  1, 0, 0, 1},
            {1, 0, 0, 1,  0, 0, 0, 0,  0, 0, 0, 0,  1, 0, 0, 1},
            {1, 1, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 1, 1},

            {1, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 0,  0, 0, 0, 1},
            {1, 0, 0, 0,  1, 1, 1, 1,  1, 1, 1, 1,  0, 0, 0, 1},
        };
    */

    // TODO(chowie): Express this as a ratio, clamp01?
    v3 Offset = V3(0, 0, 0);

    v2 CornerDim = V2(20.0f, 20.0f);
    v2 TileDim = V2(60.0f, 75.0f);
    v2 SEdgeDim = V2(TileDim.x, CornerDim.y);
    v2 WEdgeDim = V2(CornerDim.x, TileDim.y);

    // RESOURCE: On Naming - https://en.wikipedia.org/wiki/List_of_Euclidean_uniform_tilings
    v2 TesselationDim = TileDim + CornerDim;

    // TODO(chowie): Colour linear to srgb conversion!
    for(u32 Row = 0;
        Row < 2*TilesPerHeight;
        ++Row)
    {
        for(u32 Column = 0;
            Column < 2*TilesPerWidth;
            ++Column)
        {
            v4 BaseColour = {};
            if(Odd(Row) && Odd(Column))
            {
                // NOTE(chowie): Tile = Violet
                BaseColour = V4(0.3f, 0.3f, 0.7f, 1);
            }
            else if(Odd(Row) && !Odd(Column))
            {
                // NOTE(chowie): W Edge = Green
                BaseColour = V4(0.5f, 0.7f, 0.5f, 1);
            }
            else if(!Odd(Row) && Odd(Column))
            {
                // NOTE(chowie): S Edge = Pastel
                BaseColour = V4(0.7f, 0.3f, 0.3f, 1);
            }
            else
            {
                // NOTE(chowie): Corner = Grey
                BaseColour = V4(0.1f, 0.1f, 0.1f, 1);
            }

            // TODO(chowie): Tesselate for n sized group tiles
            v2 Min;
            Min.x = ((Column - 1*Odd(Column)) / 2)*TesselationDim.x + CornerDim.x*Odd(Column);
            Min.y = ((Row - 1*Odd(Row)) / 2)*TesselationDim.y + CornerDim.y*Odd(Row);

            v2 Max;
            Max.x = Min.x + CornerDim.x*!Odd(Column) + SEdgeDim.x*Odd(Column);
            Max.y = Min.y + CornerDim.y*!Odd(Row) + WEdgeDim.y*Odd(Row);

            /*
            if(!Odd(Row))
            {
                Min.y = (Row / 2)*TesselationDim.y;
                Max.y = Min.y + CornerDim.y;
            }
            else
            {
                Min.y = ((Row - 1) / 2)*TesselationDim.y + CornerDim.y;
                Max.y = Min.y + WEdgeDim.y;
            }

            if(!Odd(Column))
            {
                Min.x = (Column / 2)*TesselationDim.x;
                Max.x = Min.x + CornerDim.x;
            }
            else
            {
                Min.x = ((Column - 1) / 2)*TesselationDim.x + CornerDim.x;
                Max.x = Min.x + SEdgeDim.x;
            }
            */

            PushRect(RenderGroup, Offset, RectMinMax(Min, Max), BaseColour);
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
            ConPlayer->dP *= 64.0f;

            // TODO(chowie): Normalise diagonal movement!
            GameState->Offset += Input->dtForFrame*ConPlayer->dP;
        }
    }

    TileDim = V2(75.0f, 75.0f);
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
