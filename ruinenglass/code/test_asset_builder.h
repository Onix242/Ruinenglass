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

#include "ruinenglass_platform.h"
#include "ruinenglass_file_formats.h"
#include "ruinenglass_intrinsics.h"
#include "ruinenglass_math.h"
#include "ruinenglass_shared.h"

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
// TODO(chowie): Am I able to remove Windows for stb_truetype?
#include <windows.h>

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
#pragma pack(pop)

#define BITMAP_BYTES_PER_PIXEL 4
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

//
//
//

enum builder_asset_type
{
    AssetType_Bitmap,
    AssetType_Font,
    AssetType_FontGlyph,
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

// NOTE(chowie): Data only used for processing and not saved in file
struct builder_asset_source
{
    builder_asset_type Type;
    union
    {
        builder_asset_source_bitmap Bitmap;
        builder_asset_source_font Font;
        builder_asset_source_font_glyph Glyph;
    };
};

//
//
//

#define BUILDER_MAX_SIZE 4096

struct loaded_rui
{
    u32 AssetCount;
    rui_asset Assets[BUILDER_MAX_SIZE];

    u32 TagCount;
    rui_tag Tags[BUILDER_MAX_SIZE];

    builder_asset_source AssetSources[BUILDER_MAX_SIZE];

    u32 AssetTypeIndex;
    rui_asset_type *AssetType;
};

#define TEST_ASSET_BUILDER_H
#endif
