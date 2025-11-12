#if !defined(RUINENGLASS_RENDERER_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Michael Chow $
   $Notice: $
   ======================================================================== */

/* IMPORTANT(chowie):

   - Renderer is bottom-up / Y-is-up rendered, X is to the right.
     Consider this when reading world data on disk!

   - V4 Colours specified to the renderer are non-premultiplied alpha.
*/

// TODO(chowie): Textures?

// STUDY(chowie): Compact discriminated unions
enum render_group_entry_type
{
    RenderGroupEntryType_render_entry_clear,
    RenderGroupEntryType_render_entry_rect,
    RenderGroupEntryType_render_entry_circle,
    RenderGroupEntryType_render_entry_bitmap, // TODO(chowie): Temporary, remove for a generic quad!
};
struct render_group_entry_header
{
    u16 Type;
};

//
// NOTE(chowie): Render hooks
//

struct render_entry_clear
{
    v4 Colour;
};

struct render_entry_circle
{
    v4 Colour;
    v3 P;
    f32 Radius;
    u32 Tris;
    f32 Circumference;
};

struct render_entry_rect
{
    v4 Colour;
    v3 P;
    v2 Dim;
};

struct render_entry_bitmap
{
    loaded_bitmap *Bitmap;
    v4 PremulColour;
    v3 P;
};

/*
// STUDY(chowie): "Inheritance is compression"
// TODO(chowie): Explicit shapes & combine plain entry_type into one?
// RESOURCE(azmr): https://github.com/azmr/geometer/blob/master/geometer_filetype.h#L60
// RESOURCE(bbkane): https://www.bbkane.com/blog/polymorphism-in-handmade-hero/
struct plain_shape
{
    v4 Colour;
    v3 P;
    union
    {
        render_entry_rect Rect;
        render_entry_circle Circle;
    };
};
*/

struct game_render_commands
{
    v2u Dim;

    memory_arena CommandsArena;
};
#define RenderCommandStruct(Dim, MaxPushBufferSize, PushBuffer)  \
    {Dim, {MaxPushBufferSize, (u8 *)PushBuffer, 0}}

struct render_group
{
    struct game_assets *GameAssets;

    v2 ScreenDim;
    game_render_commands *Commands;
};

#define RUINENGLASS_RENDERER_H
#endif
