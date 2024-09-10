/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Michael Chow $
   $Notice: $
   ======================================================================== */

#include "ruinenglass_renderer.h"

inline render_group
BeginRenderGroup(game_render_commands *Commands)
{
    render_group Result = {};
    Result.Commands = Commands;

    return(Result);
}

struct push_buffer_result
{
    render_group_entry_header *Header;
};
inline push_buffer_result
PushRenderBuffer(render_group *RenderGroup, u32 DataSize)
{
    game_render_commands *Commands = RenderGroup->Commands;
    push_buffer_result Result = {};

    u8 *PushBufferEnd = Commands->PushBufferBase + Commands->MaxPushBufferSize;
    if((Commands->PushBufferDataAt + DataSize) <= PushBufferEnd)
    {
        Result.Header = (render_group_entry_header *)Commands->PushBufferDataAt;
        Commands->PushBufferDataAt += DataSize;
    }
    else
    {
        // NOTE(chowie): At this point, push buffer would overflow
        InvalidCodePath;
    }

    return(Result);
}

#define PushRenderElement(Group, type) (type *)PushRenderElement_(Group, sizeof(type), RenderGroupEntryType_##type)
inline void *
PushRenderElement_(render_group *RenderGroup, u32 Size, render_group_entry_type Type)
{
    game_render_commands *Commands = RenderGroup->Commands;
    void *Result = 0;

    Size += sizeof(render_group_entry_header);
    push_buffer_result Push = PushRenderBuffer(RenderGroup, Size);
    if(Push.Header)
    {
        render_group_entry_header *Header = Push.Header;
        Header->Type = (u16)Type;
        Result = (u8 *)Header + sizeof(*Header);
    }
    else
    {
        // NOTE(chowie): At this point, push buffer would overflow
        InvalidCodePath;
    }

    return(Result);
}

inline void
PushClear(render_group *RenderGroup, v4 Colour)
{
    render_entry_clear *Clear = PushRenderElement(RenderGroup, render_entry_clear);
    if(Clear)
    {
        Clear->Colour = Colour;
    }
}

inline void
PushCircle(render_group *RenderGroup,
           v3 Offset, r32 Radius, r32 Error = 0.5f, v4 Colour = V4(1, 1, 1, 1))
{
    render_entry_circle *Circle = PushRenderElement(RenderGroup, render_entry_circle);
    if(Circle)
    {
        Circle->P = Offset;
        Circle->Radius = Radius;
        Circle->Error = Error;
        Circle->Colour = Colour;
    }
}

// TODO(chowie): Distinguish between UI blockout rect and textured quads?
inline void
PushRect(render_group *RenderGroup,
         v3 Offset, v2 Dim, v4 Colour = V4(1, 1, 1, 1))
{
    render_entry_rect *Rect = PushRenderElement(RenderGroup, render_entry_rect);
    if(Rect)
    {
        v3 P = (Offset - V3(0.5f*Dim, 0));
        Rect->P = P;
        Rect->Dim = Dim;
        Rect->Colour = Colour;
    }
}

inline void
PushRect(render_group *RenderGroup,
         v3 Offset, rect2 Rect, v4 Colour = V4(1, 1, 1, 1))
{
    PushRect(RenderGroup, Offset + V3(GetCenter(Rect), 0), GetDim(Rect), Colour);
}

inline void
PushRectOutline(render_group *RenderGroup,
                v3 Offset, v2 Dim, v4 Colour = V4(1, 1, 1, 1), r32 BorderThickness = 0.1f)
{
    // NOTE(chowie): Top and bottom
    PushRect(RenderGroup, Offset - V3(0, 0.5f*Dim.y, 0), V2(Dim.x - BorderThickness - 0.01f, BorderThickness), Colour);
    PushRect(RenderGroup, Offset + V3(0, 0.5f*Dim.y, 0), V2(Dim.x - BorderThickness - 0.01f, BorderThickness), Colour);

    // NOTE(chowie): Left and right
    PushRect(RenderGroup, Offset - V3(0.5f*Dim.x, 0, 0), V2(BorderThickness, Dim.y + BorderThickness), Colour);
    PushRect(RenderGroup, Offset + V3(0.5f*Dim.x, 0, 0), V2(BorderThickness, Dim.y + BorderThickness), Colour);
}

inline void
PushRectOutline(render_group *RenderGroup,
                rect2 Rect, v4 Colour = V4(1, 1, 1, 1), r32 BorderThickness = 0.1f)
{
    PushRectOutline(RenderGroup, V3(GetCenter(Rect), 0), GetDim(Rect), Colour, BorderThickness);
}

