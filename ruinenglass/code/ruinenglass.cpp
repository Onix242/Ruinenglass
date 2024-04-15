/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Michael Chow $
   $Notice: $
   ======================================================================== */

#include "ruinenglass.h"

internal void
RenderWeirdGradient(game_offscreen_buffer *Buffer,
                    v2 Offset)
{
    // STUDY(chowie): Pixels are 32-bit wide, little-endian
    // architecture.
    // Memory Order:   BB GG RR xx
    // Register Order: xx RR GG BB

    u8 *Row = (u8 *)Buffer->Memory;
    for(s32 Y = 0;
        Y < Buffer->Height;
        ++Y)
    {
        u32 *Pixel = (u32 *)Row;
        for(s32 X = 0;
            X < Buffer->Width;
            ++X)
        {
            u8 Red = 0;
            u8 Blue = (u8)(X + Offset.x);
            u8 Green = (u8)(Y + Offset.y);

            // STUDY(chowie): Remember pointer arithmetic advances by
            // 4-bytes, an entire u32!
            *Pixel++ = ((Red << 16) | (Green << 8) | Blue);
        }
    
        Row += Buffer->Pitch;
    }
}

internal void
GetSoundSamples(game_sound_output_buffer *Buffer)
{
}

internal void
GameUpdateAndRender(game_offscreen_buffer *Buffer,
                    v2 Offset)
{
    // TODO(chowie): Allow sample offsets here for more robust platform options
    //GetSoundSamples(SampleCountToOutput, SoundBuffer);
    RenderWeirdGradient(Buffer, Offset);
}
