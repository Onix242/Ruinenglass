#if !defined(RUINENGLASS_RENDERER_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Michael Chow $
   $Notice: $
   ======================================================================== */

//
// IMPORTANT(chowie): Remember renderer is bottom-up / Y-is-up
// rendered. Consider this when reading world data on disk!
//

// TODO(chowie): Textures?

// STUDY(chowie): Compact discriminated unions
enum render_group_entry_type
{
    RenderGroupEntryType_render_entry_clear,
    RenderGroupEntryType_render_entry_rect,
    RenderGroupEntryType_render_entry_circle,
};
struct render_group_entry_header
{
    u16 Type;
};

//
// NOTE(chowie): Render hooks
//

struct render_entry_circle
{
    v4 Colour;
    v3 P;
    r32 Radius;
    u32 Tris;
    r32 Circumference;
};

struct render_entry_rect
{
    v4 Colour;
    v3 P;
    v2 Dim;
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

struct render_entry_clear
{
    v4 Colour;
};

struct game_render_commands
{
    v2u Dim;

    // TODO(chowie): Put arena here!
    // STUDY(chowie): Instead of storing the index of an array of
    // same-size structs, store a byte index that says where the
    // struct starts, and that handles everything. Basically the same
    // as using pointers, only you don't need 64-bits to store, use
    // 32- as everything is based off the same base pointer in memory.
    umm MaxPushBufferSize;
    u8 *PushBufferBase;
    umm PushBufferSize;
};
#define RenderCommandStruct(Dim, MaxPushBufferSize, PushBuffer)  \
    {Dim, MaxPushBufferSize, (u8 *)PushBuffer, 0}

struct render_group
{
    rect2 ScreenArea;
    game_render_commands *Commands;
};

#define RUINENGLASS_RENDERER_H
#endif
