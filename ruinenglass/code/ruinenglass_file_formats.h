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

// TODO(chowie): IMPORTANT(chowie): Combine type_id into tag_id!

enum asset_tag_id
{
    Tag_None,

    Tag_FacingDirection,

    Tag_Count,
};

enum asset_type_id
{
    Asset_None,

    //
    // Fonts
    //

    Asset_UnicodeCodepoint,
    Asset_FontTypeface,
    Asset_FontWeight,

    Asset_BasicCategory, // NOTE(chowie): Null category

    //
    // Character Expression
    //

    // RESOURCE(): https://www.reddit.com/r/coolguides/comments/1e21slu/a_cool_guide_to_japanese_emoticons/
    // RESOURCE(): https://en.wikipedia.org/wiki/List_of_emoticons
    Asset_Neutral, // IxI

    Asset_Happy, // ^x^
    Asset_Amazed, // *x*
    Asset_Relaxed, // .~x~.

    Asset_Alarmed, // ¡x!
    Asset_Confused, // ¿x?
    Asset_Dizzy, // @x@

    Asset_Sad, // .x.
    Asset_Tears, // ;x;
    Asset_Crying, // »x«

    Asset_SleepyTired, // =x= or -x-zzz or animate between them
    Asset_Ouch, // >x< or annoyed
    Asset_Guilty, // ¬x¬

    //
    // Non-Character
    //

    Asset_DEBUG_Bush,
    Asset_DEBUG_Lilypad,
    Asset_DEBUG_Lotus,

    Asset_Count,
};

/* NOTE: Move Asset_id to here
enum asset_basic_category
{
    Category_None,

    Category_Expressions,
    Category_Teeth,

    Category_Font,
    Category_FontGlyph,

    Category_DEBUG,

    Category_Count,
};
*/

struct asset_match_vector
{
    f32 E[Asset_Count];
};

//
//
//

#pragma pack(push, 1)

#define RUI_MAGIC_VALUE FILE_FORMAT_CODE('r', 'u', 'i', 'f')
#define RUI_VERSION 1
struct rui_header
{
    u32 MagicValue;
    u32 Version;

    u32 TagCount;
    u32 AssetCount;

    u32 AssetTypeCount;

    u64 TagsOffset; // rui_tag[TagCount]
    u64 AssetsOffset; // rui_asset[AssetCount]

    u64 AssetTypesOffset; // rui_asset_type[AssetTypeCount]
};

struct rui_tag
{
    enum32(asset_tag_id) ID;
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

enum asset_header_type // TODO(chowie): Use?
{
    HeaderType_None,

    HeaderType_Bitmap,
    HeaderType_Font,

    HeaderType_Count,
};

struct rui_asset
{
    u64 DataOffset;
    u32 FirstTagIndex;
    u32 OnePastLastTagIndex;
    enum32(asset_header_type) Type; // TODO(chowie): Use?
    union
    {
        rui_bitmap Bitmap;
        rui_font Font;
    };
};

#pragma pack(pop)

//
//
//

#define RUINENGLASS_FILE_FORMATS_H
#endif
