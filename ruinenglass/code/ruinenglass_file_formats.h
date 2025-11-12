#if !defined(RUINENGLASS_FILE_FORMATS_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Michael Chow $
   $Notice: $
   ======================================================================== */

enum asset_font_weight
{
    FontWeight_Semibold, // NOTE(chowie): Asian fonts don't typically support semibold
    FontWeight_Regular,
    FontWeight_Thin,
};

enum asset_font_typeface
{
    // NOTE(chowie): Western Font
    FontTypeface_Geologica, // NOTE(chowie): Header + UI
    FontTypeface_AtkinsonHyperlegible, // NOTE(chowie): Body

    // NOTE(chowie): Eastern Font
    // NOTE(chowie): Figtree has no Asian characters, for English titles left untranslated
    FontTypeface_Figtree, // NOTE(chowie): Header + UI
    FontTypeface_LINESeed, // NOTE(chowie): Body
    // TODO(chowie): LINESeed doesn't support Simplified Chinese, only Traditional.

    //
    // NOTE(chowie): Strictly for gameplay (world space text)
    //

    FontTypeface_Eurolang, // NOTE(chowie): Includes Cistercian clock numbers, maps to ~1234567890-=
    FontTypeface_Asialang,
    FontTypeface_Calclang, // NOTE(chowie): Includes 7-segment display numbers

    // NOTE(chowie): Punctuation Form Banks / Morphotheeks
    FontTypeface_PunctBlock,
    FontTypeface_PunctBlank,
    FontTypeface_PunctBar,
    FontTypeface_PunctSlab, // NOTE(chowie): Includes animated eyes/faces
};

enum asset_tag_id
{
    Tag_None,

    //
    // NOTE(chowie): Fonts
    //

    Tag_UnicodeCodepoint,
    Tag_FontTypeface,
    Tag_FontWeight,

    //
    //
    //

    Tag_Count,
};

//
//
//

struct rui_header
{
#define RUI_MAGIC_VALUE FILE_FORMAT_CODE('r', 'u', 'i', 'f')
    u32 TagCount;
};

struct rui_tag
{
    u32 ID;
    f32 Value;
};

struct rui_font_glyph
{
    u32 UnicodePoint;
    bitmap_id ID;
};

//
//
//

#define RUINENGLASS_FILE_FORMATS_H
#endif
