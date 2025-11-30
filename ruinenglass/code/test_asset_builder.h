#if !defined(TEST_ASSET_BUILDER_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Michael Chow $
   $Notice: $
   ======================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include "ruinenglass_platform.h"
#include "ruinenglass_file_formats.h"
#include "ruinenglass_intrinsics.h"
#include "ruinenglass_math.h"

// TODO(chowie): Am I able to remove Windows for stb_truetype?
#include <windows.h>

// TODO(chowie): Compress this offline builder to be run with
// hotreloading and RUINENGLASS_INTERNAL for offline

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
    HFONT Win32Handle;
    TEXTMETRIC TextMetric;

    // NOTE(chowie): Dense
    rui_font_glyph *Glyphs;
    // NOTE(chowie): It is sparse and dense in 1D; don't know how many
    // glyphs the user will add.
    f32 *HorizontalAdvance;

    // NOTE(chowie): Contiguous lookup in table!
    v2u CodePoint;

    // NOTE(chowie): Sparse
    u32 MaxGlyphCount;
    u32 GlyphCount;

    // NOTE(chowie): It is sparse and not dense; Unicode -> Index to
    // font; this is translation table. (Instead of array of null
    // pointers to save space).
    u32 *GlyphIndexFromCodePoint;
    u32 OnePastHighestCodePoint;
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

struct builder_game_assets
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
