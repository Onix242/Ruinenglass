/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Michael Chow $
   $Notice: $
   ======================================================================== */

inline render_group
BeginRenderGroup(game_assets *GameAssets, game_render_commands *Commands)
{
    render_group Result = {};

    Result.GameAssets = GameAssets;
    Result.Commands = Commands;
    Result.ScreenDim = V2i(Commands->Dim);

    return(Result);
}

#define PushRenderElement(RenderGroup, type) (type *)PushRenderElement_(RenderGroup, sizeof(type), RenderGroupEntryType_##type)
inline void *
PushRenderElement_(render_group *RenderGroup, umm Size, render_group_entry_type Type)
{
    game_render_commands *Commands = RenderGroup->Commands;
    void *Result = 0;

    Size += sizeof(render_group_entry_header);
    render_group_entry_header *Header = (render_group_entry_header *)PushSize(&Commands->CommandsArena, Size, Align(0));
    Header->Type = (u16)Type;
    Result = (u8 *)Header + sizeof(*Header);

    return(Result);
}

inline void
PushClear(render_group *RenderGroup, v4 Colour)
{
    render_entry_clear *Entry = PushRenderElement(RenderGroup, render_entry_clear);
    if(Entry)
    {
        Entry->Colour = Colour;
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
           v3 Offset, f32 Radius, u32 TriCount, v4 Colour = V4(1, 1, 1, 1), f32 Circumference = Tau32)
{
    render_entry_circle *Entry = PushRenderElement(RenderGroup, render_entry_circle);
    if(Entry)
    {
        Entry->P = Offset;
        Entry->Radius = Radius;
        Entry->Tris = TriCount;
        Entry->Colour = Colour;
        Entry->Circumference = Circumference;
    }
}

/*
inline void
PushCircle(render_group *RenderGroup,
           v3 Offset, f32 Radius, f32 Error, v4 Colour = V4(1, 1, 1, 1))
{
    f32 Theta = (f32)acos(1 - Error / Radius);
    f32 Tris = Ceil(Pi32 / Theta);
    PushCircle(RenderGroup, Offset, Radius, Tris, Colour);
}
*/

// TODO(chowie): Distinguish between UI blockout rect and textured quads?
inline void
PushRect(render_group *RenderGroup,
         v3 Offset, v2 Dim, v4 Colour = V4(1, 1, 1, 1))
{
    render_entry_rect *Entry = PushRenderElement(RenderGroup, render_entry_rect);
    if(Entry)
    {
        v3 P = (Offset - V3(0.5f*Dim, 0));
        Entry->P = P;
        Entry->Dim = Dim;
        Entry->Colour = Colour;
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
                v3 Offset, v2 Dim, v4 Colour = V4(1, 1, 1, 1), f32 BorderThickness = 0.1f)
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
                rect2 Rect, v4 Colour = V4(1, 1, 1, 1), f32 BorderThickness = 0.1f)
{
    PushRectOutline(RenderGroup, V3(GetCenter(Rect), 0), GetDim(Rect), Colour, BorderThickness);
}

