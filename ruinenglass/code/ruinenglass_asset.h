#if !defined(RUINENGLASS_ASSET_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Michael Chow $
   $Notice: $
   ======================================================================== */

// TODO(chowie): Don't think I need asset locking, assets are small
// and can just use simple asset streaming?

// enum asset_header_type
// {
//     HeaderType_None,
//     HeaderType_Bitmap,
//     HeaderType_Font,
// };

// TODO(chowie): Streamline by using header pointer as an indicator of unloaded status?
enum asset_state
{
    AssetState_Unloaded,
    AssetState_Queued,
    AssetState_Loaded,
};
struct asset_memory_header
{
    asset_memory_header *Prev;
    asset_memory_header *Next;
    union
    {
        rui_bitmap *Bitmap;
        loaded_font *Font;
    };
};
struct asset
{
    u32 AtomicState;

    rui_asset RUI;

    rui_bitmap Bitmap; // TODO(chowie) Remove?
    loaded_font Font;

    u32 FileIndex;
};

// NOTE(chowie): Acts as a group
// TODO(chowie): Eventually use a hash table?
struct asset_type
{
    u32 FirstAssetIndex;
    u32 OnePastLastAssetIndex; // COULDDO(chowie): Could be replaced with a count instead?
};

struct asset_file
{
    platform_file_handle FileHandle;

    rui_header Header;
    rui_asset_type *AssetTypeArray; // COULDDO(chowie): Proper thread stacks would render AssetTypeArray as useless!

    u32 AssetBase;
    u32 TagBase;
    s32 FontBitmapIDOffset;
};

// NOTE(chowie): Loaded from disc
// NOTE(chowie): Difference between asset vs game_asset is raw
// structured-ness from file loading
struct game_assets
{
    memory_arena Arena;
    struct transient_state *TranState; // STUDY(chowie): Not thrilled about back pointer to transtate

    //
    //
    //

    u32 FileCount;
    asset_file *Files;

    u32 AssetCount;
    asset *Assets;

    u32 TagCount;
    rui_tag *Tags;
    f32 AssetsByTag[Tag_Count];

    //
    //
    //

    rui_bitmap  NullBitmap;
    loaded_font NullFont;

    //
    //
    //

    asset_type AssetTypes[Tag_Count];
};

struct task_memory;

struct load_asset_work
{
    task_memory *TaskMemory;
    asset *Asset;
    platform_file_handle *FileHandle;

    u64 Offset;
    u64 Size;
    void *Destination;

    u32 FinalState;
};

//
// NOTE(chowie): Asset API
//

internal rui_bitmap *GetBitmap(game_assets *Assets, bitmap_id ID);
internal void LoadBitmap(game_assets *GameAssets, bitmap_id ID);

#define RUINENGLASS_ASSET_H
#endif
