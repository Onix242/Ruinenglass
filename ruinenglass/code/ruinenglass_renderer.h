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

//
// XForms
//

struct render_xform
{
    v3 P;
    v3 X;
    v3 Y;
    v3 Z;
    m4x4_inv Proj; // NOTE(chowie): World's camera + Proj Matrix combined
};

enum camera_xform_flag
{
    Camera_IsOrtho = BitSet(1),
    Camera_IsPersp = BitSet(2),
    Camera_IsDebug = BitSet(3),
};
struct camera_xform
{
    b32x Orthographic;
    f32 FocalLength; // NOTE(chowie): How far away from monitor
    v3 CameraP; // NOTE(chowie): Offset in relation to ground etc.
};

//
// Textures
//

struct render_hook
{
    rect2i ClipRect;
    m4x4 Proj;
    v3 CameraP;
};

// NOTE(chowie): UV coords are packed into vertex - if not altering
// 0-1 UV, don't need to call this
// TODO(chowie): Instead of having individual textured quads, we have
// two separate array (SOA). Bulk vertex buffer from platform layer
// for the frame flip.
struct render_entry_textured_quads
{
    render_hook Hook;

    u32 QuadCount;
    u32 VertexArrayOffset; // NOTE(chowie): 4 vertices per quad
    u32 IndexArrayOffset; // NOTE(chowie): 6 indices per quad
};

// STUDY(chowie): GPU's are good at unaligned format, no need to manually add padding!
// STUDY(chowie): Textured vertices are provided to us by the system.
struct textured_vertex
{
    v4 P;
    v2 UV; // COULDDO(chowie): If textured coord will be 0-1 all of the time, you may not need to include it! They could just be inferred!
    u32 Colour; // NOTE(chowie): Packed RGBA in memory order (ABGR in little-endian)
};

//
// Push Buffer Types
//

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

// NOTE(chowie): Render hooks

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

//
// To Render
//

struct game_render_commands
{
    v2u OSWindowDim;
    v2u OSDrawRegionDim; // NOTE(chowie): Subsection of window

    memory_arena CommandsArena;
};
#define RenderCommandStruct(OSWindowDim, OSDrawRegionDim, MaxPushBufferSize, PushBuffer)  \
    {OSWindowDim, OSDrawRegionDim, {MaxPushBufferSize, (u8 *)PushBuffer, 0}}

struct render_group
{
    struct game_assets *GameAssets;
    game_render_commands *Commands;

    render_xform GameXForm;
    render_xform DebugXForm;

    textured_vertex *VertexArray;

};

#define RUINENGLASS_RENDERER_H
#endif
