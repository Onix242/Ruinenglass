/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Michael Chow $
   $Notice: $
   ======================================================================== */

//
//
//

inline asset_file *
GetFile(game_assets *GameAssets, u32 FileIndex)
{
    Assert(FileIndex < GameAssets->FileCount);
    asset_file *Result = GameAssets->Files + FileIndex;
    return(Result);
}

inline platform_file_handle *
GetFileHandleFor(game_assets *GameAssets, u32 FileIndex)
{
    platform_file_handle *Result = &GetFile(GameAssets, FileIndex)->FileHandle;
    return(Result);
}

//
// NOTE(chowie): Multithreading Assets?
//

struct load_asset_work
{
    task_memory *TaskMemory;
    game_assets *GameAssets;
};

internal void
LoadAssetWorkDirectly(load_asset_work *Work)
{
};

internal
PLATFORM_WORK_QUEUE_CALLBACK(LoadAssetWork)
{
    load_asset_work *Work = (load_asset_work *)Data;

//    LoadAssetWorkDirect(Work);

    EndTaskMemory(Work->TaskMemory);
}

//
//
//

internal void
AllocGameAssets(game_state *GameState, game_assets *GameAssets, umm Size)
{
    game_assets *Assets = PushStruct(&GameAssets->Arena, game_assets);
}

inline asset *
GetAsset(game_assets *GameAssets, u32 ID)
{
    Assert(ID <= GameAssets->AssetCount);
    asset *Result = GameAssets->Assets + ID;
    return(Result);
}

inline rui_tag *
GetTag(game_assets *GameAssets, u32 ID)
{
    Assert(ID <= GameAssets->TagCount);
    rui_tag *Result = GameAssets->Tags + ID;
    return(Result);
}

internal loaded_bitmap *
GetBitmap(game_assets *GameAssets, bitmap_id ID)
{
    asset *Asset = GetAsset(GameAssets, ID.Value);
    loaded_bitmap *Result = {};
    if(Asset)
    {
        Assert(ID.Value == 0);
        Result = Asset ? Asset->Bitmap : 0;
    }

    return(Result);
}

internal loaded_font *
GetFont(game_assets *GameAssets, font_id ID)
{
    asset *Asset = GetAsset(GameAssets, ID.Value);
    loaded_font *Result = {};
    if(Asset)
    {
        Assert(ID.Value == 0);
        Result = Asset ? Asset->Font : 0;
    }

    return(Result);
}

internal void
LoadBitmap(game_assets *GameAssets, bitmap_id ID)
{
}

internal void
LoadFont(game_assets *GameAssets, font_id ID)
{
    asset *Asset = GameAssets->Assets + ID.Value;
    // TODO(chowie): Check if I need the cast?
    if(ID.Value &&
       (AtomicCompareExchangeU32((u32 *)&Asset->AtomicState,
                                 AssetState_Queued, AssetState_Unloaded) == AssetState_Unloaded))
    {
        task_memory *TaskMemory = BeginTaskMemory(GameAssets->GameState);
        if(TaskMemory)
        {
            load_asset_work *Work = PushStruct(&TaskMemory->Arena, load_asset_work, NoClear());
            Work->TaskMemory = TaskMemory;
            Work->GameAssets = 0; // TODO(chowie): Add assets!
            Platform.AddWorkQueueEntry(GameAssets->GameState->LowPriorityQueue, LoadAssetWork, Work);
        }
    }
    else
    {
        GameAssets->Assets[ID.Value].AtomicState = AssetState_Unloaded;
    }
}

