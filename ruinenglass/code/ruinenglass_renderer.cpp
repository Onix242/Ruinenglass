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
    Result.ScreenArea = RectMinDim(V2(0, 0), V2i(Commands->Dim));

    return(Result);
}

// TODO(chowie): Push with an arena? See PushString/PushStringZ
#define PushRenderElement(RenderGroup, type) (type *)PushRenderElement_(RenderGroup, sizeof(type), RenderGroupEntryType_##type)
inline void *
PushRenderElement_(render_group *RenderGroup, umm Size, render_group_entry_type Type)
{
    game_render_commands *Commands = RenderGroup->Commands;
    void *Result = 0;

    Size += sizeof(render_group_entry_header);
    if((Commands->PushBufferSize + Size) < Commands->MaxPushBufferSize)
    {
        render_group_entry_header *Header = (render_group_entry_header *)(Commands->PushBufferBase + Commands->PushBufferSize);
        Header->Type = (u16)Type;
        Result = (u8 *)Header + sizeof(*Header);
        // NOTE(chowie): Next byte after the header is full sized.
        // Will do arithmetic for us if we had a sized pointer,
        // whatever the byte location of header + its size. (Header +
        // 1) would be the alternative way to write it.
        Commands->PushBufferSize += Size;
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

// STUDY(chowie): 1) Error-based circles vs 2) Tricount circles.
// 1) Hard to understand correlation of smoothness vs tris. E.g.
// Bevelling, usually error-based. But same resolution can be enforced
// for any radius.
// 2) Same problem but more tris aware. Smoothness is still difficult,
// I'd rather not comment "Smaller values = smoother & more tris", it
// should be more obvious/transparent than errors by _px_ threshold &
// to get the tricount more easily in API).
// TODO(chowie): Pass rotation
inline void
PushCircle(render_group *RenderGroup,
           v3 Offset, r32 Radius, u32 TriCount, v4 Colour = V4(1, 1, 1, 1), r32 Circumference = Tau32)
{
    render_entry_circle *Circle = PushRenderElement(RenderGroup, render_entry_circle);
    if(Circle)
    {
        Circle->P = Offset;
        Circle->Radius = Radius;
        Circle->Tris = TriCount;
        Circle->Colour = Colour;
        Circle->Circumference = Circumference;
    }
}

/*
inline void
PushCircle(render_group *RenderGroup,
           v3 Offset, r32 Radius, r32 Error, v4 Colour = V4(1, 1, 1, 1))
{
    r32 Theta = (r32)acos(1 - Error / Radius);
    r32 Tris = Ceil(Pi32 / Theta);
    PushCircle(RenderGroup, Offset, Radius, Tris, Colour);
}
*/

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

