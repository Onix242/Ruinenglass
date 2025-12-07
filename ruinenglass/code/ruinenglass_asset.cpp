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

internal game_assets *
AllocGameAssets(transient_state *TranState, memory_arena *Arena, umm Size)
{
    game_assets *GameAssets = PushStruct(Arena, game_assets);

    GameAssets->TranState = TranState;

    // NOTE(chowie): First entries are intentionally left null
    GameAssets->TagCount   = 1;
    GameAssets->AssetCount = 1;
    GameAssets->FileCount  = 1;

    {
        platform_file_group FileGroup = Platform.GetAllFilesOfTypeBegin(PlatformFileType_AssetFile);
        GameAssets->FileCount = FileGroup.FileCount;
        GameAssets->Files = PushArray(Arena, asset_file, GameAssets->FileCount);
        for(u32 FileIndex = 0;
            FileIndex < GameAssets->FileCount;
            ++FileIndex)
        {
            asset_file *File = GameAssets->Files + FileIndex;

            File->FontBitmapIDOffset = 0;
            File->AssetBase = GameAssets->AssetCount - 1;
            File->TagBase = GameAssets->TagCount;

            ZeroStruct(File->Header);
            File->FileHandle = Platform.OpenNextFile(&FileGroup);
            Platform.ReadDataFromFile(&File->FileHandle, 0, sizeof(File->Header), &File->Header);

            u32 AssetTypeArraySize = File->Header.AssetTypeCount*sizeof(rui_asset_type);
            File->AssetTypeArray = (rui_asset_type *)PushSize(Arena, AssetTypeArraySize);
            Platform.ReadDataFromFile(&File->FileHandle, File->Header.AssetTypesOffset,
                                      AssetTypeArraySize, File->AssetTypeArray);

            // NOTE(chowie): All we care about is getting back a
            // header, we don't care when that happens; if we cannot
            // open the file, we can't open the file.
            if(File->Header.MagicValue != RUI_MAGIC_VALUE)
            {
                Platform.FileError(&File->FileHandle, "RUI file has an invalid magic value.");
            }

            if(File->Header.Version > RUI_VERSION)
            {
                Platform.FileError(&File->FileHandle, "RUI file is of a later version.");
            }

            if(PlatformNoFileErrors(&File->FileHandle))
            {
                // NOTE(chowie): The first asset and tag slot is a
                // null asset (reserved), it is not counted as
                // something that requires space!
                if(File->Header.TagCount)
                {
                    GameAssets->TagCount += (File->Header.TagCount - 1);
                }

                if(File->Header.AssetCount)
                {
                    GameAssets->AssetCount += (File->Header.AssetCount - 1);
                }
            }
            else
            {
                // TODO(chowie): Some way of notifying users of bogus files?
                InvalidCodePath;
            }
        }
        Platform.GetAllFilesOfTypeEnd(&FileGroup);
    }

    GameAssets->Tags   = PushArray(Arena, rui_tag, GameAssets->TagCount);
    ZeroStruct(GameAssets->Tags[0]); // NOTE(chowie): First entry is intentionally left null
    GameAssets->Assets = PushArray(Arena, asset, GameAssets->AssetCount);

    // NOTE(chowie): First entry is intentionally left null
    u32 GameAssetCount = 0;
    ZeroStruct(*(GameAssets->Assets + GameAssetCount));
    ++GameAssetCount;

    for(u32 FileIndex = 0;
        FileIndex < GameAssets->FileCount;
        ++FileIndex)
    {
        asset_file *File = GameAssets->Files + FileIndex;
        if(PlatformNoFileErrors(&File->FileHandle))
        {
            if(File->Header.TagCount)
            {
                // NOTE(chowie): Skip the first tag, since it is null!
                u32 TagArraySize = sizeof(rui_tag)*(File->Header.TagCount - 1);
                Platform.ReadDataFromFile(&File->FileHandle, File->Header.TagsOffset + sizeof(rui_tag),
                                          TagArraySize, GameAssets->Tags + File->TagBase);
                // NOTE(chowie): Tags don't really reference anything that needs
                // to be moved really! It is flat-loaded in this case.
            }

            if(File->Header.AssetCount)
            {
                u32 FileAssetCount = (File->Header.AssetCount - 1);

                temporary_memory TempMemory = BeginTemporaryMemory(&TranState->TranArena);
                rui_asset *AssetArray = PushArray(&TranState->TranArena,
                                                  rui_asset, FileAssetCount);
                Platform.ReadDataFromFile(&File->FileHandle,
                                          File->Header.AssetsOffset + 1*sizeof(rui_asset),
                                          FileAssetCount*sizeof(rui_asset),
                                          AssetArray);

                for(u32 AssetIndex = 0;
                    AssetIndex < FileAssetCount;
                    ++AssetIndex)
                {
                    rui_asset *RUIAsset = AssetArray + AssetIndex;
                    Assert(GameAssetCount < GameAssets->AssetCount);
                    u32 GlobalGameAssetCount = GameAssetCount++;
                    asset *Asset = GameAssets->Assets + GameAssetCount++;

                    Asset->FileIndex = FileIndex;
                    Asset->RUI = *RUIAsset;
                    if(Asset->RUI.FirstTagIndex == 0)
                    {
                        Asset->RUI.FirstTagIndex = Asset->RUI.OnePastLastTagIndex = 0;
                    }
                    else
                    {
                        Asset->RUI.FirstTagIndex += (File->TagBase - 1);
                        Asset->RUI.OnePastLastTagIndex += (File->TagBase - 1);
                    }
                }

                EndTemporaryMemory(TempMemory);
            }
        }
    }

    return(GameAssets);
}

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
// Multithreading Assets?
//

internal
PLATFORM_WORK_QUEUE_CALLBACK(LoadAssetWork)
{
    load_asset_work *Work = (load_asset_work *)Data;
    asset_state State = AssetState_Unloaded;

    Platform.ReadDataFromFile(Work->FileHandle, Work->Offset, Work->Size, Work->Destination);
    if(PlatformNoFileErrors(Work->FileHandle))
    {
        State = AssetState_Loaded;
    
        // TODO(chowie): Finalise the asset types here
    }
    else
    {
        ZeroSize(Work->Size, Work->Destination);
    }

    Work->Asset->AtomicState = Work->FinalState;

    CompletePrevWritesBeforeFutureWrites;

    EndTaskMemory(Work->TaskMemory);
}

//
//
//

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

internal rui_bitmap *
GetBitmap(game_assets *GameAssets, bitmap_id ID)
{
    asset *Asset = GetAsset(GameAssets, ID.Value);
    rui_bitmap *Result = {};
    if(Asset)
    {
        Assert(ID.Value == 0);
        Result = Asset ? &Asset->Bitmap : 0;
    }

    return(Result);
}

internal rui_bitmap *
GetBitmapInfo(game_assets *GameAssets, bitmap_id ID)
{
    asset *Asset = GetAsset(GameAssets, ID.Value);
    rui_bitmap *Result = {};
    if(Asset)
    {
        Assert(ID.Value == 0); // TODO(chowie): Safeguard for bitmap asset type?
        Result = &Asset->RUI.Bitmap;
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
        Result = Asset ? &Asset->Font : 0;
    }

    return(Result);
}

internal rui_font *
GetFontInfo(game_assets *GameAssets, font_id ID)
{
    asset *Asset = GetAsset(GameAssets, ID.Value);
    rui_font *Result = {};
    if(Asset)
    {
        Assert(ID.Value == 0); // TODO(chowie): Safeguard for font asset type?
        Result = &Asset->RUI.Font;
    }

    return(Result);
}

//
// Asset Loading
//

// TODO(chowie): I'd like to use GetBitmapAsset() but can't because of
// the ordering/atomics
internal void
LoadBitmap(game_assets *GameAssets, bitmap_id ID)
{
    asset *Asset = GetAsset(GameAssets, ID.Value);
    if(ID.Value &&
       (AtomicCompareExchangeU32((u32 *)&Asset->AtomicState,
                                 AssetState_Queued, AssetState_Unloaded) == AssetState_Unloaded))
    {
        rui_bitmap *Info = &Asset->RUI.Bitmap;
        v2u Dim = Info->Dim;
        u32 TextureSize = Dim.Width*Dim.Height*BITMAP_BYTES_PER_PIXEL;

        task_memory *TaskMemory = BeginTaskMemory(GameAssets->TranState);
        if(TaskMemory)
        {
            load_asset_work *Work = PushStruct(&TaskMemory->Arena, load_asset_work, NoClear());
            Work->TaskMemory = TaskMemory;
            Work->Asset = Asset;
            Work->FileHandle = GetFileHandleFor(GameAssets, Asset->FileIndex);
            Work->Offset = Asset->RUI.DataOffset;
            Work->Size = TextureSize;
//            Work->Destination = Bitmap->Memory; // TODO(chowie): Figure out someway TextureOp?
            Work->FinalState = AssetState_Loaded;
//            Work->TextureOpQueue = Assets->TextureOpQueue;

            Platform.AddWorkQueueEntry(GameAssets->TranState->LowPriorityQueue, LoadAssetWork, Work);
        }
        else
        {
            Asset->AtomicState = AssetState_Unloaded;
        }
    }
    else
    {
        Asset->AtomicState = AssetState_Unloaded;
    }
}

internal void
LoadFont(game_assets *GameAssets, font_id ID)
{
    asset *Asset = GetAsset(GameAssets, ID.Value);
    if(ID.Value &&
       (AtomicCompareExchangeU32((u32 *)&Asset->AtomicState,
                                 AssetState_Queued, AssetState_Unloaded) == AssetState_Unloaded))
    {
        rui_font *Info = &Asset->RUI.Font;
        u32 HorizontalAdvanceSize = sizeof(f32)*Info->GlyphCount*Info->GlyphCount;
        u32 GlyphsSize = Info->GlyphCount*sizeof(rui_font_glyph);
        u32 UnicodeMapSize = sizeof(u16)*Info->OnePastMaxCodepoint;
        u32 SizeData = GlyphsSize + HorizontalAdvanceSize;
        u32 SizeTotal = SizeData + UnicodeMapSize;

        loaded_font *Font = &Asset->Font;
        Font->BitmapIDOffset = GetFile(GameAssets, Asset->FileIndex)->FontBitmapIDOffset;
//        Font->Glyphs = (rui_font_glyph *)(Asset->Header + 1); // TODO(chowie): Asset header?
        Font->HorizontalAdvance = (f32 *)((u8 *)Font->Glyphs + GlyphsSize);
        Font->UnicodeMap = (u16 *)((u8 *)Font->HorizontalAdvance + HorizontalAdvanceSize);

        ZeroSize(UnicodeMapSize, Font->UnicodeMap);

        task_memory *TaskMemory = BeginTaskMemory(GameAssets->TranState);
        if(TaskMemory)
        {
            load_asset_work *Work = PushStruct(&TaskMemory->Arena, load_asset_work, NoClear());
            Work->TaskMemory = TaskMemory;
            Work->Asset = Asset;
            Work->FileHandle = GetFileHandleFor(GameAssets, Asset->FileIndex);
            Work->Offset = Asset->RUI.DataOffset;
            Work->Size = SizeData;
//            Work->Destination = Bitmap->Memory; // TODO(chowie): Figure out someway TextureOp?
            Work->FinalState = AssetState_Loaded;
//            Work->TextureOpQueue = Assets->TextureOpQueue;

            Platform.AddWorkQueueEntry(GameAssets->TranState->LowPriorityQueue, LoadAssetWork, Work);
        }
        else
        {
            Asset->AtomicState = AssetState_Unloaded;
        }
    }
    else
    {
        Asset->AtomicState = AssetState_Unloaded;
    }
}

//
//
//

internal u32
GetAssetMatch(game_assets *GameAssets, asset_type_id ID, asset_match_vector MatchVector)
{
    u32 Result = 0;

    f32 BestMatch = 0.0f;
    asset_type *Type = GameAssets->AssetTypes + ID;
    for(u32 AssetIndex = Type->FirstAssetIndex;
        AssetIndex < Type->OnePastLastAssetIndex;
        ++AssetIndex)
    {
        asset *Asset = GetAsset(GameAssets, AssetIndex);

        f32 TotalMatch = 0.0f;
        for(u32 TagIndex = Asset->RUI.FirstTagIndex;
            TagIndex < Asset->RUI.OnePastLastTagIndex;
            ++TagIndex)
        {
            rui_tag *Tag = GetTag(GameAssets, TagIndex);

            for(u32 MatchIndex = 0;
                MatchIndex < ArrayCount(MatchVector.E);
                ++MatchIndex)
            {
                f32 A = MatchVector.E[MatchIndex];
                f32 B = Tag->Value;
                f32 Dist0 = AbsoluteValue(A - B);
                f32 Difference = 1.0f - Dist0;
                TotalMatch += Difference;
            }

            if(BestMatch < TotalMatch)
            {
                BestMatch = TotalMatch;
                Result = AssetIndex;
            }
        }
    }

    return(Result);
}

inline bitmap_id
GetBestBitmapMatch(game_assets *GameAssets, asset_type_id ID, asset_match_vector MatchVector = {})
{
    bitmap_id Result = {GetAssetMatch(GameAssets, ID, MatchVector)};
    return(Result);
}

inline font_id
GetBestFontMatch(game_assets *GameAssets, asset_type_id ID, asset_match_vector MatchVector = {})
{
    font_id Result = {GetAssetMatch(GameAssets, ID, MatchVector)};
    return(Result);
}

//
// Font Ops
//

internal u32
GetGlyphFromCodePoint(rui_font *Info, loaded_font *Font, u32 CodePoint)
{
    u32 Result = 0;
    if(CodePoint < Info->OnePastMaxCodepoint)
    {
        Result = Font->UnicodeMap[CodePoint];
        Assert(Result < Info->GlyphCount);
    }

    return(Result);
}

internal f32
GetHorizontalAdvanceForPair(rui_font *Info, loaded_font *Font,
                            u32 DesiredPrevCodePoint, u32 DesiredCodePoint)
{
    u32 PrevGlyph = GetGlyphFromCodePoint(Info, Font, DesiredPrevCodePoint);
    u32 Glyph = GetGlyphFromCodePoint(Info, Font, DesiredCodePoint);
    f32 Result = Font->HorizontalAdvance[PrevGlyph*Info->GlyphCount + Glyph];

    return(Result);
}

internal bitmap_id
GetBitmapForGlyph(rui_font *Info, loaded_font *Font,
                  u32 DesiredCodePoint)
{
    u32 Glyph = GetGlyphFromCodePoint(Info, Font, DesiredCodePoint);
    bitmap_id Result = {Font->Glyphs[Glyph].ID.Value + Font->BitmapIDOffset};
    return(Result);
}

inline f32
GetLineAdvanceFor(rui_font *Info)
{
    f32 Result = Info->AscenderHeight + Info->DescenderHeight + Info->ExternalLeading;
    return(Result);
}

inline f32
GetStartingBaselineY(rui_font *Info)
{
    f32 Result = Info->AscenderHeight;
    return(Result);
}

