#if !defined(RUINENGLASS_ASSET_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Michael Chow $
   $Notice: $
   ======================================================================== */

//
// NOTE(chowie): Asset Formats
//

struct loaded_font
{
    rui_font_glyph *Glyphs;
};

//
//
//

// TODO(chowie): Don't think I need asset locking, assets are small
// and can just use simple asset streaming?

// TODO(chowie): Streamline by using header pointer as an indicator of unloaded status?
enum asset_state
{
    AssetState_Unloaded,
    AssetState_Queued,
    AssetState_Loaded,
};
struct asset
{
    u32 AtomicState;

    loaded_bitmap *Bitmap;
    loaded_font *Font;
};

struct asset_file
{
    platform_file_handle FileHandle;

    rui_header Header;
};

// NOTE(chowie): Loaded from disc
// NOTE(chowie): Difference between asset vs game_asset is raw
// structured-ness from file loading
struct game_assets
{
    memory_arena Arena;
    struct game_state *GameState; // STUDY(chowie): Not thrilled about back pointer to transtate

    //
    //
    //

    u32 FileCount;
    asset_file *Files;

    u32 AssetCount;
    asset *Assets;

    u32 TagCount;
    rui_tag *Tags;
};

//
// NOTE(chowie): Asset API
//

internal loaded_bitmap *GetBitmap(game_assets *Assets, bitmap_id ID);

#define RUINENGLASS_ASSET_H
#endif
