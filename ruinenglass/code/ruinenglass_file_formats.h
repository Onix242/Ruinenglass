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
// - categories | - bitmaps/pngs of voxel faces
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
// COULDDO(chowie): Load ".lhl" writing format and ".klp" conlang format

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
    // NOTE(chowie): Strictly for gameplay (world-space text)
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

    //
    // Content
    //

    u32 TagCount;
    u32 AssetCount;
    u32 AssetTypeCount;

    u64 TagsOffset; // rui_tag[TagCount]
    u64 AssetTypesOffset; // rui_asset_type[AssetTypeCount]
    u64 AssetsOffset; // rui_asset[AssetCount]
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

// IMPORTANT(chowie): Every Repligram (not fink hash) has a textured
// "block theme" (overlapping colours palettes/textures that masks
// over the base textures of block_id). Doesn't necessarily corresponds
// 1-to-1 of how biomes are defined.
enum block_palette : u16
{
    // NOTE(chowie): Block themes for environments
    BlockPalette_Sterile,
    BlockPalette_Snow,
    BlockPalette_Ash,
    BlockPalette_Ruins,

    // NOTE(chowie): Block themes for characters
    BlockPalette_EurolangCreatures,
    BlockPalette_AsialangCreatures,
    BlockPalette_CalclangCreatures,
};

/********************************
         WHAT IS REPLIGRAM?
   ******************************

   A player-facing 'toy' used for the art production pipeline.
   In the process of creating a Repligram as the artist, you also:
   - Create a procedural/'compressed' 3D voxel model
   - Apply smear animation automatically
   - Make valid nonogram/Picross puzzles (contribute directly to gameplay)

   Used to create environment assets or character assets.

   The player works their way backwards from the final animation loop

   For example, assume you as the artist produce the 2D house below:
   1) Make the model and assign materials, # = Brick, W = Window, D = Door

       ###
      #####
     #######
     W     #
     #     D
     #######

   2) Add up to 2 'priorities' (numbers) to animate (smear) the model
   for each row on the x,y axis. x = don't smear, [number] = smear nth
   frame e.g. [1] = smear every frame, [4] smear every 4th, [1, 5] =
   smear every frame and every 5th.

      5
      2xxx1xx

  7,3   ###
    2  #####
    x #######
    1 W     #
    4 #     D
    x #######

   3) Assign a direction to smear on an animation timeline

      5
      2xxx1xx

  7,3   ###
    2  #####
    x #######
    1 W     #
    4 #     D
    x #######

   |>----------|
   First smear is to the right

      5
      2xxx1 xx

  7,3   ####
    2  ######
    x ########
    1 W      #
    4 #      D
    x ########

   The house was smeared one place to the right, it affected the column:
   roof and floor

   |>^---------|
   Next smear is up

      5
      2xxx1 xx

  7,3   ####
    2  ######
    x ########
      W      #
    1 W      #
    4 #      D
    x ########

   The window and opposite wall is now smeared.

   |>^^--------|
   Next smear is up again

      5
      2xxx1 xx

  7,3   ####
       ######
    2  ######
    x ########
      W      #
      W      #
    1 W      #
    4 #      D
    x ########

   The window and opposite wall is now smeared. And now part of the
   roof (because it's now the second smear on the y-axis)

   You can use this to make tiny little objects and blow up to be
   gigantic! I've used this to make the smallest house, but when an
   entity stops by, it blows up to the size of their body to fit
   through the door. Or a house that turns into an high-rise apartment?

   3) Serialise the animation timeline with the final model. This
   becomes the player's reference model and attempt to replicate.
   - They also make their own compressed model and assign priorities.
   - There's possibly multiple answers! As long as they reach the same
   animation state.

   4) Successfully replicating the reference model plays some fanfare,
   and presents a conlang word that describes the model. The player
   attempts to guess the word by typing.

   The technical:

   Extends idea of "shape grammar" and "L-systems" using
   nonograms/Picross. Comparatively less flexible, but easier to
   understand (for both player and developers) and more performant!

   It's different to "shape grammar" and "L-systems" because:
   - It's for real-time animation (smear animation = extrudes). You
     work with a timeline
   - Fairly straight-forward to implement
   - Doesn't work with strings (patterns) as the interface
   - Not state-based or node-based

   Has these exact (or similar) ops to "shape grammar" and "L-systems":
   - Extrude/Scale
   - Repeat
   - Split = inverted "join" (implicit)
   - Translate (implicit = extrude air blocks)
   - Indexed palette
   - Inc++/Dec--
   - RandomChoice() of blocks

   Has these unique ops:
   - "Priority" = frames until extrude (must be in x,y,z whole rows/columns)

   Doesn't have these ops:
   - Rotate
   - Meta layer to connect select repligrams

   RESOURCE(): https://github.com/stefalie/shapeml
   RESOURCE(): https://www.youtube.com/watch?v=f6ra024-ASY

*/

struct rui_repligram
{
    v3u Dim;
    v2u PriorityDim;
    block_palette Palette;
};

// NOTE(chowie): References continguous range in a separate table
// (array). Allows multiple asset files to be merged contiguously.
struct rui_asset_type
{
    u32 TypeID;
    u32 FirstAssetIndex;
    u32 OnePastLastAssetIndex;
};

enum asset_header_type
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
    enum32(asset_header_type) Type;
    union
    {
        rui_bitmap Bitmap;
        rui_font Font;
        rui_repligram Repligram;
    };
};

#pragma pack(pop)

//
//
//

// TODO(chowie): Find a more elegant solution
// NOTE(chowie): Rep = Repeat (like in compression context i.e. LZ4)
enum repligram_op_type : u8
{
    RepligramOp_None   = BitSet(0),

    RepligramOp_Inc    = BitSet(1), // NOTE(chowie): Like math repeating symbol
    RepligramOp_Dec    = BitSet(2), // NOTE(chowie): Like math repeating symbol
    RepligramOp_Random = BitSet(3), // NOTE(chowie): For debugging
//    RepligramOp_Rep    = BitSet(4), // NOTE(chowie): Requires ops to be adjacent to each other, includes formbanks' bars and blanks
//    RepligramOp_Modulo = BitSet(5),
};
struct repligram_priority_clue
{
    u8 Priority; // NOTE(chowie): 0 maps to 'x'
    enum8(repligram_op_type) Op;
};
struct repligram_priority
{
    repligram_priority_clue Clue[2];
    u8 RLERep; // NOTE(chowie): Runs or offset from base (current row/col index)
};
struct repligram_priority_group
{
    repligram_priority *Priorities;
    u32 Count;
    u8 NonIntervalOverlapRasteriseBitmask; // NOTE(chowie): Only for editor
};
// NOTE(chowie): RLERep, gets converted to a range/v2u at runtime
// |-------------------------|
// | Index, 0                |
// |-------------------------|
// | Index, RLERep (3, 2)    | For closed interval [3, 5], in other words [3, 3+2]
// |-------------------------|
// | Index, 0                |
// |-------------------------|
// 1) Read row/col index as usual (in order)
// 2) Testing point to closed interval e.g. cursor on row/col to move finds the interval
//    __Intervals cannot overlap__
//    "for(; (TestRowColIndex < RepligramDim); ++Index)
//    {
//        if(RLERep != 0)
//        {
//            if(IsRLERepPointInClosedInterval(TestRowColIndex,
//                                             V2U(TestRowColIndex, TestRowColIndex + RLERep)))
//            {...}
//        }
//    }"
// 3) Test with cursor buffer at start and end of interval to know how
//    to move/cut surrounding intervals and cut repligram dim

// NOTE(chowie): How rasterise bitmask works (make sure to check for op_rep)
// Adding group on left-most side needs to always start with 0
// IMPORTANT(chowie): Always need check index before and after to determine
// if it's a range (interval) or not (a point). Needs parity checks!
// IMPORTANT(chowie): Always init new rows/cols to "0 1 0 1 0 1 0 1"
// 1) 0 1 0 0 0 1 0 1 (default)
// 2) 0 1 0 0 0 1 0 0 (add new group = NOT ~)

// 1) 0 1 0 0 0 1 1 1 (default)
// 2) 0 1 0 0 1 0 1 0 (trim = NOT ~)

// 1) 0 1 0 0 0 1 0 0 (default)
// 2) 0 1 0 0 0 0 0 0 (union/merge = copy bit)

// 1) 0 1 1 1 1 0 1 0 (default)
// 2) 0 1 1 0 1 0 1 1 (split = for loop NOT ~ at split)
// 2) 0 1 1 0 1 0 0 0 (split and don't union = for loop NOT ~ at split)

// 1) 0 1 1 1 0 1 1 0 (default)
// 2) 0 1 1 1 1 0 0 1 (extrude and don't union = for loop NOT ~ end point)

// 1) 0 1 1 1 0 1 1 0 (default)
// 2) 0 1 0 0 0 1 1 0 (move and don't union = for loop NOT ~ end point)

// TODO(chowie): Create Repligram's hash by using conlang word name (CRC32)!

// NOTE(chowie): Repligram edit ops (like video editors)
// - Merge/Join
// - Extrude
// - Split
// - Duplicate
// - Delete
// - Move/Trim

// COULDDO(chowie): I could instead use FinkHash for subranges? Or
// RemaleyHash for combinatorics? Or rasterise into bitmasks if < 32
// Or what if there's something that exists that "when you add a number
// the other side decreases"?

// COULDDO(chowie): Could hold next - 1 to get the current and to prevent
// wrapping. e.g. See debug system (DebugState->NextFreeFrame - 1) %
// DEBUG_FRAME_COUNT.
// COULDDO(chowie): Refer to HmH Day 354 29:50 for how to look back to a prev value!
// inline void
// IncrementOrdinal(u32 *Ordinal)
// {
//     *Ordinal = (*Ordinal + 1) % Block_Count;
// }

// NOTE(chowie): "Point - Interval" will intentionally wrap!
// [a, b]
inline b32x
IsRLERepPointInClosedInterval(u32 Point, v2u Interval)
{
    b32x Result = (Point - Interval.Start) <= (Interval.End - Interval.Start);
    return(Result);
}

// NOTE(chowie): [a, b] [c, d]
inline b32x
IsRLERepClosedIntervalsOverlap(v2u IntervalA, v2u IntervalB)
{
    b32x Result = ((IntervalB.Start - IntervalA.Start) <= (IntervalA.End - IntervalA.Start)) ||
                  ((IntervalA.Start - IntervalB.Start) <= (IntervalB.End - IntervalB.Start));
    return(Result);
}

// NOTE(chowie): Static = Non-moving animation, easily toggleable to have animation
// TODO(chowie): Decal = Assert (Dim == (.x == 1) || (.y == 1) || (.z == 1))
enum repligram_anim_type : u8
{
    AnimType_Animated,
    AnimType_Static,
    AnimType_AnimatedDecal,
    AnimType_StaticDecal,
};
enum repligram_anim_stretch_dir : u16
{
    AnimDir_X = BitSet(0),
    AnimDir_nX = BitSet(1),
    AnimDir_Y = BitSet(2),
    AnimDir_nY = BitSet(3),
    AnimDir_Z = BitSet(4),
    AnimDir_nZ = BitSet(5),

    AnimDir_FlipX = BitSet(6),
    AnimDir_FlipnX = BitSet(7),
    AnimDir_FlipY = BitSet(8),
    AnimDir_FlipnY = BitSet(9),
    AnimDir_FlipZ = BitSet(10),
    AnimDir_FlipnZ = BitSet(11),

    AnimDir_SyncX = AnimDir_X | AnimDir_nX,
    AnimDir_SyncY = AnimDir_Y | AnimDir_nY,
    AnimDir_SyncZ = AnimDir_Z | AnimDir_nZ,
    AnimDir_SyncUniform = AnimDir_SyncX | AnimDir_SyncY | AnimDir_SyncZ,
};
struct repligram_timeline_entry
{
    u32 P;
    enum16(repligram_anim_stretch_dir) Dir;
};

// NOTE(chowie): Constant = bones doesn't change value from prev keyframe
enum xform_constant_flags
{
    Translation_Constant = BitSet(1),
    Orientation_Constant = BitSet(2),
    Scale_Constant       = BitSet(3), // NOTE(chowie): Scale may not exist
};

#define RUINENGLASS_FILE_FORMATS_H
#endif
