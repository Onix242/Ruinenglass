#if !defined(RUINENGLASS_RENDERER_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Michael Chow $
   $Notice: $
   ======================================================================== */

//
// NOTE(chowie): Bitmaps was top-down, renderer is bottom-up.
//

// STUDY(chowie): Compact discriminated unions
enum render_group_entry_type
{
    RenderGroupEntryType_render_entry_clear,
    RenderGroupEntryType_render_entry_rect,
    RenderGroupEntryType_render_entry_circle,
    // TODO(chowie): Texture?
};
struct render_group_entry_header
{
    u16 Type;
};

struct render_entry_circle
{
    v4 Colour;
    v3 P;
    r32 Radius;
    r32 Error; // NOTE(chowie): Smaller values = smoother & more tris
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

    // STUDY(chowie): Instead of storing the index of an array of
    // same-size structs, store a byte index that says where the
    // struct starts, and that handles everything. Basically the same
    // as using pointers, only you don't need 64-bits to store, use
    // 32- as everything is based off the same base pointer in memory.
    u32 MaxPushBufferSize;
    u8 *PushBufferBase;
    u8 *PushBufferDataAt;
};
#define RenderCommandStruct(Dim, MaxPushBufferSize, PushBuffer)  \
    {Dim, MaxPushBufferSize, (u8 *)PushBuffer, (u8 *)PushBuffer};

struct render_group
{
    game_render_commands *Commands;
};

#define RUINENGLASS_RENDERER_H
#endif
