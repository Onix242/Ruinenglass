/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Michael Chow $
   $Notice: $
   ======================================================================== */

#include "test_asset_builder.h"

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

struct entire_file
{
    u32 ContentsSize;
    void *Contents;
};
entire_file
ReadEntireFile(char *FileName)
{
    entire_file Result = {};

    FILE *In = fopen(FileName, "rb");
    if(In)
    {
        // NOTE(chowie): Hacky way to get the entire file!
        fseek(In, 0, SEEK_END);
        Result.ContentsSize = ftell(In);
        fseek(In, 0, SEEK_SET);

        Result.Contents = malloc(Result.ContentsSize);
        fread(Result.Contents, Result.ContentsSize, 1, In);
        fclose(In);
    }
    else
    {
        printf("ERROR: Cannot open file %s.\n", FileName);
    }

    return(Result);
}

internal builder_loaded_bitmap
LoadBMP(char *FileName)
{
    builder_loaded_bitmap Result = {};

    entire_file ReadResult = ReadEntireFile(FileName);
    if(ReadResult.ContentsSize != 0)
    {
        Result.Free = ReadResult.Contents;

        // NOTE: Cold-cast
        bitmap_header *Header = (bitmap_header *)ReadResult.Contents;
        u32 *Pixels = (u32 *)((u8 *)ReadResult.Contents + Header->BitmapOffset);
        Result.Memory = Pixels;
        Result.Dim = Header->Dim;

        Assert(Result.Dim.Height >= 0);
        Assert(Header->Compression == 3);
        // NOTE: If you are using this generically, remember bmp files can go in either
        // directions and the height will be negative for top-down.
        // Also, compression etc... Not complete BMP code.

        // NOTE: Byte order is determined by the header so we have to read out the masks
        // and convert the pixels ourselves.
        u32 RedMask = Header->ColourMasks.r;
        u32 GreenMask = Header->ColourMasks.g;
        u32 BlueMask = Header->ColourMasks.b;
        u32 AlphaMask = ~(RedMask | BlueMask | GreenMask);

        bit_scan_result RedScan   = FindLeastSignificantBit(RedMask);
        bit_scan_result GreenScan = FindLeastSignificantBit(GreenMask);
        bit_scan_result BlueScan  = FindLeastSignificantBit(BlueMask);
        bit_scan_result AlphaScan = FindLeastSignificantBit(AlphaMask);
        
        Assert(RedScan.Found);
        Assert(GreenScan.Found);
        Assert(BlueScan.Found);
        Assert(AlphaScan.Found);

        s32 RedShiftDown   = (s32)RedScan.Index;
        s32 GreenShiftDown = (s32)GreenScan.Index;
        s32 BlueShiftDown  = (s32)BlueScan.Index;
        s32 AlphaShiftDown = (s32)AlphaScan.Index;

        u32 *SourceDest = Pixels;
        for(s32 Y = 0;
            Y < Header->Dim.Height;
            ++Y)
        {
            for(s32 X = 0;
                X < Header->Dim.Width;
                ++X)
            {
                u32 C = *SourceDest;

                // COULDDO(chowie): Can I use BGRAUnpack4x8(C), maybe?
                v4 Texel = {(f32)((C & RedMask)   >> RedShiftDown),
                            (f32)((C & GreenMask) >> GreenShiftDown),
                            (f32)((C & BlueMask)  >> BlueShiftDown),
                            (f32)((C & AlphaMask) >> AlphaShiftDown)};

                Texel = sRGB255ToLinear1(Texel);

                PremultipliedStoreColour(Texel);

                Texel = Linear1TosRGB255(Texel); // NOTE(chowie): Make sure all other routines are gamma aware!

                *SourceDest++ = BGRAPack4x8(Texel);
            }
        }
    }

    Result.Pitch = Result.Dim.Width*BITMAP_BYTES_PER_PIXEL;

    return(Result);
}

//
//
//

struct added_asset
{
    u32 ID;
    rui_asset *RUI;
    builder_asset_source *Source;
};
internal added_asset
AddAsset(builder_game_assets *GameAssets)
{
    Assert(GameAssets->AssetType);
    Assert(GameAssets->AssetType->OnePastLastAssetIndex < ArrayCount(GameAssets->Assets));

    u32 Index = GameAssets->AssetType->OnePastLastAssetIndex++;
    builder_asset_source *Source = GameAssets->AssetSources + Index;
    rui_asset *RUI = GameAssets->Assets + Index;
    RUI->FirstTagIndex = GameAssets->TagCount;
    RUI->OnePastLastTagIndex = RUI->FirstTagIndex;

    GameAssets->AssetTypeIndex = Index;

    added_asset Result;
    Result.ID = Index;
    Result.RUI = RUI;
    Result.Source = Source;

    return(Result);
}

internal void
BeginAssetType(builder_game_assets *GameAssets, asset_tag_id TagID)
{
    Assert(GameAssets->AssetType == 0); // NOTE(chowie): Assumes not multithreaded for now!

    GameAssets->AssetType = GameAssets->AssetType + TagID;
    GameAssets->AssetType->TypeID = TagID;
    GameAssets->AssetType->FirstAssetIndex = GameAssets->AssetCount;
    GameAssets->AssetType->OnePastLastAssetIndex = GameAssets->AssetType->FirstAssetIndex;
}

internal void
EndAssetType(builder_game_assets *GameAssets)
{
    Assert(GameAssets->AssetType);
    GameAssets->AssetCount = GameAssets->AssetType->OnePastLastAssetIndex;
    GameAssets->AssetType = 0;
    GameAssets->AssetTypeIndex = 0;
}

internal bitmap_id
AddBitmapAsset(builder_game_assets *GameAssets, char *FileName, v2 AlignPercentage = {0.5f, 0.5f})
{
    added_asset Asset = AddAsset(GameAssets);
    Asset.RUI->Bitmap.AlignPercentage = AlignPercentage;
    Asset.Source->Type = AssetType_Bitmap;
    Asset.Source->Bitmap.FileName = FileName;

    bitmap_id Result = {Asset.ID};
    return(Result);
}

//
//
//

int
main(int ArgCount, char **Args)
{
}

