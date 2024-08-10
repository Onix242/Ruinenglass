/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Michael Chow $
   $Notice: $
   ======================================================================== */

#include "ruinenglass.h"
#include "ruinenglass_audio.cpp"

// NOTE(chowie): This is top-down, renderer is now bottom-up
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

internal void
TestRenderPlayer(game_offscreen_buffer *Buffer, v2s PlayerP)
{
    u8 *EndOfBuffer = (u8 *)Buffer->Memory + Buffer->Pitch * Buffer->Dim.Height;
    
    u32 Color = 0xFFFFFFFF;
    int Top = PlayerP.x;
    int Bottom = PlayerP.y+10;
    for(int X = PlayerP.x;
        X < PlayerP.x+10;
        ++X)
    {
        u8 *Pixel = ((u8 *)Buffer->Memory +
                     X*BITMAP_BYTES_PER_PIXEL +
                     Top*Buffer->Pitch);
        for(int Y = Top;
            Y < Bottom;
            ++Y)
        {
            if((Pixel >= Buffer->Memory) &&
               ((Pixel + 4) <= EndOfBuffer))
            {
                *(u32 *)Pixel = Color;
            }

            Pixel += Buffer->Pitch;
        }
    }
}

extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
    // TODO(chowie): Use this!
//    Platform = Memory->PlatformAPI;

    Assert(sizeof(game_state) <= Memory->Permanent.Size);
    game_state *GameState = (game_state *)Memory->Permanent.Base; // STUDY: Cold-cast

    if(!GameState->IsInitialised)
    {
        GameState->Offset = {};

#if 0
        char *FileName = __FILE__;

        debug_read_file_result File = Memory->PlatformAPI.DEBUGReadEntireFile(FileName);
        if(File.Contents)
        {
            Memory->PlatformAPI.DEBUGWriteEntireFile("test.out", File.ContentsSize, File.Contents);
            Memory->PlatformAPI.DEBUGFreeFileMemory(File.Contents);
        }
#endif

        GameState->Player.P = {100, 100};

        // TODO(chowie): Initialise Audio State when there's proper
        // sound! Do I have to rearrange "audio -> game" to prevent
        // audio clipping at the beginning?
//        InitialiseAudioState(&GameState->AudioState);

        GameState->IsInitialised = true; // STUDY(chowie): One less thing the platform layer has to do; the game would always.
    }

    for(u32 ControllerIndex = 0;
        ControllerIndex < ArrayCount(Input->Controllers);
        ++ControllerIndex)
    {
        game_controller_input *Controller = &Input->Controllers[ControllerIndex];
        if(Controller->IsAnalog)
        {
            GameState->Offset.x += 4.0f*(Controller->StickAverage.x);
        }
        else
        {
            if(Controller->MoveLeft.EndedDown)
            {
                --GameState->Offset.x;
            }

            if(Controller->MoveRight.EndedDown)
            {
                ++GameState->Offset.x;
            }
        }

        if(Controller->ActionDown.EndedDown)
        {
            --GameState->Offset.y;
        }

        GameState->Player.P.x += (s32)(4.0f*(Controller->StickAverage.x));
        GameState->Player.P.y += (s32)(4.0f*(Controller->StickAverage.y));
    }

    RenderWeirdGradient(Buffer, GameState->Offset);
    TestRenderPlayer(Buffer, GameState->Player.P);
}

extern "C" GAME_GET_SOUND_SAMPLES(GameGetSoundSamples)
{
    game_state *GameState = (game_state *)Memory->Permanent.Base; // TODO(chowie): Replace with an dedicated Audio.Base

    // TODO(chowie): Allow sample offsets here for more robust
    // platform options!
    // TODO(chowie): OutputPlayingSounds() Mixer
    //TestOutputWilwaDialTone(&GameState->AudioState, SoundBuffer);
}
