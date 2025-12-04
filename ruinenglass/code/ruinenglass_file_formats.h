#if !defined(RUINENGLASS_FILE_FORMATS_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Michael Chow $
   $Notice: $
   ======================================================================== */

//
// NOTE(chowie): Current game content architecture
//

// | Brains Entity Behaviour ---------------------->| Entity Master  ------------------------------->|
// | Hot-Reload Code                                | Hot-Reload Code                                |
//                                                                                                   |
// | Terrain + Character Pieces Art --------------->| Terrain + Character Builder    --------------->|
// | Dotgrid .svg > Krita .svg > Birdfont .ttf      | Custom Asset Packer Format .rui                |
//                                                  | (+ Tile set + Animation                        |
//                                                  |  + Conlang Object/Name Metadata)               |
//                                                  |                                                |
// | Conlang Gameplay   -->| Narrative Storylets -->| Narrative Staging + Parser     --------------->| Level (Game Runtime)
// | Custom Format .klp    | Custom Format .lhl     | Custom Asset Packer Format .rui                | In-engine voxel + level editor
//                         | + .klp heredocs        | (words are the smallest atomic                 | In-engine conlang text parser
//                         | + (optional) generated |  units not characters for conlang)             | Spatial linguistic map metadata
//                         | file for localisation  | + GUI to rearrange linear & node events        |
//                                                  | + generate sentences on the fly                |
//                                                  |                                                |
//                                                  | UI Rect Cuts Layout            --------------->|
//                                                  | Custom Asset Packer Format .rui                |
//                                                  |                                                |
// | Audio (TBD)                                 -->|                                --------------->|

// NOTE(chowie):
// Game Code    | Asset Table
// - categories | - bitmaps
// - tags       |

// Asset Table has:
// enums | Asset Type  | ->       | Asset Table         | ->  | Tags
//   0   | First Range | First  0 | Tags  Info/Metadata | tag | s32 f32 
//   1   |   4     3   |        1 |                     |     |
//                              2 |
//                              3 |
//                              4 | -1-
//                              5 | -1-
//                              6 | -1-
// enum = assets like shadow, head, body
// NOTE(chowie): Merge enum on load (load, interpret, compression). On disc will be stored differently + help with incremental linking

// IMPORTANT(chowie): For font files, based on how these assets are
// constructed (with overlapping geometry/contours in the font
// file). OTF intersects font glyphs, and programs don't play nice
// either. I figured out (+ looking online says) TTF supports
// overlapping contours!
// TODO(chowie): Load ".lhl" writing format and ".klp" conlang format

//
//
//

#define RUI_MAGIC_VALUE FILE_FORMAT_CODE('r', 'u', 'i', 'f')
#define RUI_VERSION 1
struct rui_header
{
    u32 MagicValue;
    u32 Version;

    u32 TagCount;
    u32 AssetCount;
    u32 AssetTypeCount;

    u64 Tags; // rui_tag[TagCount]
    u64 Assets; // rui_asset[AssetCount]
    u64 AssetTypes; // rui_asset_type[AssetTypeCount]
};

struct rui_tag
{
    u32 ID;
    f32 Value;
};

struct rui_bitmap
{
    v2u Dim;
    v2 AlignPercentage;
};

struct rui_font_glyph
{
    u32 UnicodeCodepoint;
    bitmap_id ID;
};
struct rui_font
{
    u32 OnePastMaxCodepoint;
    u32 GlyphCount;
    f32 AscenderHeight;
    f32 DescenderHeight;
    f32 ExternalLeading;
};

// NOTE(chowie): References continguous range in a separate table
// (array). Allows multiple asset files to be merged contiguously.
struct rui_asset_type
{
    u32 TypeID;
    u32 FirstAssetIndex;
    u32 OnePastLastAssetIndex;
};

struct rui_asset
{
    u64 DataOffset;
    u32 FirstTagIndex;
    u32 OnePastLastTagIndex;
    union
    {
        rui_bitmap Bitmap;
        rui_font Font;
    };
};

// TODO(chowie): What to do with rui_font?
struct loaded_font
{
    rui_font_glyph *Glyphs;
    f32 *HorizontalAdvance;
    u32 BitmapIDOffset;
    u16 *UnicodeMap;
};

//
//
//

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
    FontTypeface_Figtree,  // NOTE(chowie): Header + UI
    FontTypeface_LINESeed, // NOTE(chowie): Body
    // TODO(chowie): Find a new font, LINESeed doesn't support Simplified Chinese, only Traditional.

    // NOTE(chowie): Special header fonts to be left untranslated
    FontTypeface_Kalnia,
    FontTypeface_Fraunces,

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
    // Fonts
    //

    Tag_UnicodeCodepoint,
    Tag_FontTypeface,
    Tag_FontWeight,

    //
    // Character Expression
    //

    // RESOURCE(): https://www.reddit.com/r/coolguides/comments/1e21slu/a_cool_guide_to_japanese_emoticons/
    // RESOURCE(): https://en.wikipedia.org/wiki/List_of_emoticons
    Tag_Neutral, // IxI

    Tag_Happy, // ^x^
    Tag_Amazed, // *x*
    Tag_Relaxed, // .~x~.

    Tag_Alarmed, // ¡x!
    Tag_Confused, // ¿x?
    Tag_Dizzy, // @x@

    Tag_Sad, // .x.
    Tag_Tears, // ;x;
    Tag_Crying, // »x«

    Tag_SleepyTired, // =x= or -x-zzz or animate between them
    Tag_Ouch, // >x< or annoyed
    Tag_Guilty, // ¬x¬

    //
    //
    //

    Tag_Count,
};

struct asset_match_vector
{
    f32 E[Tag_Count];
};

#define RUINENGLASS_FILE_FORMATS_H
#endif
