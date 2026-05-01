#if !defined(TEST_ASSET_BUILDER_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Michael Chow $
   $Notice: $
   ======================================================================== */

// TODO(chowie): Compress this offline builder to be run with
// hotreloading and RUINENGLASS_INTERNAL for offline

// TODO(chowie): Store strings in here?

// TODO(chowie): Eventually switch to using Windows to read/write

#include "ruinenglass_platform.h"
#include "ruinenglass_file_formats.h"
#include "ruinenglass_intrinsics.h"
#include "ruinenglass_math.h"
#include "ruinenglass_shared.h"

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
// IMPORTANT(chowie): TODO(chowie): Remove Windows for stb_truetype?
#include <windows.h>

//
// Headers
//

#pragma pack(push, 1)

struct bitmap_header
{
    u16 FileType;
    u32 FileSize;
    u16 Reserved1;
    u16 Reserved2;
    u32 BitmapOffset;
    u32 Size;
    v2s Dim;
    u16 Planes;
    u16 BitsPerPixel;
    u32 Compression;
    u32 SizeOfBitmap;
    v2s DimResolution;
    u32 ColoursUsed;
    u32 ColoursImportant;

    v3u ColourMasks;
};

//

// RESOURCE(jblow): Animation File Format, part 1 - https://www.youtube.com/watch?v=sQKAoiMPPOQ
// TODO(chowie): Watch future parts?
#define REPLIGRAM_MAGIC_VALUE FILE_FORMAT_CODE('r', 'e', 'p', 'l')
#define REPLIGRAM_VERSION 1
struct repligram_header
{
    u32 MagicValue;
    u32 Version;

    s32 FramesPerSecond; // STUDY(chowie): JBlow recommends s32 instead of u32 (from prev anim file format). Although you have to check for less than 0!
    v2s KeyframeDuration; // TODO(chowie): Assert typeof(KeyframeDuration == v2s) in case the type changes later!
    b32 IsBookmarked;

    string Title; // COULDDO(chowie): Could move this to a separate annotation format?
    v3u Dim; // NOTE(chowie): Volume max size
    v3 Origin;

    enum8(repligram_anim_type) AnimType;
};

// TODO(chowie): When I feel more comfortable with the file format,
// replace with shared header for normal anim and bones anim file format.
struct repligram_header_shared
{
    u32 MagicValue;
    u32 Version;

    s32 FramesPerSecond; // STUDY(chowie): JBlow recommends s32 instead of u32 (from prev anim file format). Although you have to check for less than 0!
    v2s KeyframeDuration; // TODO(chowie): Assert typeof(KeyframeDuration == v2s) in case the type changes later!
    b32 IsBookmarked;
};

/*
// NOTE(chowie): Bone header doesn't include voxel/mesh info unlike
// the normal header. Keeps inline with other anim formats though.
#define REPLIGRAM_BONE_MAGIC_VALUE FILE_FORMAT_CODE('r', 'e', 'p', 'b')
#define REPLIGRAM_BONE_VERSION 1
struct repligram_header_bone
{
    repligram_header_shared Shared;

    //
    // Content
    //

    u32 BoneIDCount;
    u32 BoneCount;
    u32 ParentIndexCount;
    u32 XformCount;
    u32 XformConstantFlagsCount;
    // TODO(chowie): Make a function "Compare flags of prev/next keyframe if same, or clear"
    // e.g. memcmp(ipos, pos, sizeof(typeof(pos))) flags &= ~TRANSLATION_CONSTANT

    u32 BoneIDOffset; // string[IDCount] or checksum+date because storing many strings can be massive for file size
    u32 BoneOffset; // char *[BoneCount]
    u32 ParentIndexOffset; // s16[ParentIndexCount];
    u32 XformOffset; // render_transform[XformCount]; // TODO(chowie): Change to an array of positions/orientation/scale
    u32 XformConstantFlagsOffset; // xform_constant_flags[XformConstantFlagsCount];
};
*/

#pragma pack(pop)

//
//
//

struct builder_loaded_bitmap
{
    v2s Dim;
    s32 Pitch;
    void *Memory;

    void *Free;
};

struct builder_loaded_font
{
    // NOTE(chowie): Dense
    rui_font_glyph *Glyphs;
    // NOTE(chowie): It is sparse and dense in 1D; don't know how many
    // glyphs the user will add.
    f32 *HorizontalAdvance;

    // NOTE(chowie): Sparse
    u32 MaxGlyphCount;

    // NOTE(chowie): It is sparse and not dense; Unicode -> Index to
    // font; this is translation table. (Instead of array of null
    // pointers to save space).
    u32 OnePastMaxCodePoint;
    u32 GlyphCount;
    u32 *CodePointFromGlyph;
};

struct builder_loaded_repligram
{
    v3u Dim;

    // TODO(chowie): IMPORTANT(chowie): Hash priorities as an ID
    // somehow, so title can change freely. It's safe to hash as it
    // would be a different repligram otherwise.
    u32 PriorityCount;
    u32 VoxelCount;
    u32 StretchCount;

    repligram_priority *Priorities;
    u64 *FinkHashes;
    repligram_timeline_entry *Stretches;

    void *Free;
};

//
//
//

enum builder_asset_type
{
    AssetType_Bitmap,
    AssetType_Font,
    AssetType_FontGlyph,
    AssetType_Repligram,
};

struct builder_asset_source_bitmap
{
    char *FileName;
};

struct builder_asset_source_font
{
    builder_loaded_font *Font;
};

struct builder_asset_source_font_glyph
{
    builder_loaded_font *Font;
    u32 Codepoint;
};

struct builder_asset_source_repligram
{
    char *FileName;
};

// NOTE(chowie): Data only used for processing and not saved in file
struct builder_asset_source
{
    builder_asset_type Type;
    union
    {
        builder_asset_source_bitmap Bitmap;
        builder_asset_source_font Font;
        builder_asset_source_font_glyph Glyph;
        builder_asset_source_repligram Repligram;
    };
};

//
//
//

#define BUILDER_MAX_SIZE 4096

struct loaded_rui
{
    u32 TagCount;
    rui_tag Tags[BUILDER_MAX_SIZE];

    // TODO(chowie): I think I'd like to collapse AssetType with Tags,
    // or just have a general category tag?
    u32 AssetTypeCount;
    rui_asset_type AssetTypes[Asset_Count];

    u32 AssetCount;
    rui_asset Assets[BUILDER_MAX_SIZE]; // NOTE(chowie): Assets in file

    builder_asset_source AssetSources[BUILDER_MAX_SIZE]; // NOTE(chowie): Just markup, never write it out!

    u32 AssetTypeIndex;
    rui_asset_type *DEBUGAssetType;
};

#define TEST_ASSET_BUILDER_H
#endif
