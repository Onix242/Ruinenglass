/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Michael Chow $
   $Notice: $
   ======================================================================== */

#include "test_asset_builder.h"

internal string
ReadEntireFile(char *FileName)
{
    string Result = {};

    FILE *In = fopen(FileName, "rb");
    if(In)
    {
        // NOTE: Hacky way to get the entire file!
        fseek(In, 0, SEEK_END);
        Result.Size = ftell(In);
        fseek(In, 0, SEEK_SET);

        Result.Data = (u8 *)malloc(Result.Size);
        fread(Result.Data, Result.Size, 1, In);
        fclose(In);
    }
    else
    {
        printf("ERROR: Cannot open file %s.\n", FileName);
    }

    return(Result);
}

//
//
//

internal builder_loaded_bitmap
LoadBitmap(char *FileName)
{
    builder_loaded_bitmap Result = {};

    string ReadResult = ReadEntireFile(FileName);
    if(ReadResult.Size != 0)
    {
        Result.Free = ReadResult.Data;

        // NOTE: Cold-cast
        bitmap_header *Header = (bitmap_header *)ReadResult.Data;
        u32 *Pixels = (u32 *)((u8 *)ReadResult.Data + Header->BitmapOffset);
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

internal void
BeginAssetType(loaded_rui *RUI, asset_tag_id ID)
{
    Assert(RUI->AssetType == 0); // NOTE(chowie): Assumes not multithreaded for now!

    RUI->AssetType = RUI->AssetType + ID;
    RUI->AssetType->TypeID = ID;
    RUI->AssetType->FirstAssetIndex = RUI->AssetCount;
    RUI->AssetType->OnePastLastAssetIndex = RUI->AssetType->FirstAssetIndex;
}

internal void
EndAssetType(loaded_rui *RUI)
{
    Assert(RUI->AssetType);
    RUI->AssetCount = RUI->AssetType->OnePastLastAssetIndex;
    RUI->AssetType = 0;
    RUI->AssetTypeIndex = 0;
}

struct added_asset
{
    u32 ID;
    rui_asset *Asset;
    builder_asset_source *Source;
};
internal added_asset
AddAsset(loaded_rui *RUI)
{
    Assert(RUI->AssetType);
    Assert(RUI->AssetType->OnePastLastAssetIndex < ArrayCount(RUI->Assets));

    u32 Index = RUI->AssetType->OnePastLastAssetIndex++;
    builder_asset_source *Source = RUI->AssetSources + Index;

    rui_asset *Asset = RUI->Assets + Index;
    Asset->FirstTagIndex = RUI->TagCount;
    Asset->OnePastLastTagIndex = Asset->FirstTagIndex;

    RUI->AssetTypeIndex = Index;

    added_asset Result;
    Result.ID = Index;
    Result.Asset = Asset;
    Result.Source = Source;

    return(Result);
}

internal bitmap_id
AddBitmapAsset(loaded_rui *RUI, char *FileName, v2 AlignPercentage = {0.5f, 0.5f})
{
    added_asset Added = AddAsset(RUI);
    Added.Asset->Bitmap.AlignPercentage = AlignPercentage;
    Added.Source->Type = AssetType_Bitmap;
    Added.Source->Bitmap.FileName = FileName;

    bitmap_id Result = {Added.ID};
    return(Result);
}

internal void
AddTag(loaded_rui *RUI, asset_tag_id ID, f32 Value)
{
    Assert(RUI->AssetTypeIndex != 0);

    rui_asset *Asset = RUI->Assets + RUI->AssetTypeIndex;
    ++Asset->OnePastLastTagIndex;

    rui_tag *Tag = RUI->Tags + RUI->TagCount++;
    Tag->ID = ID;
    Tag->Value = Value;
}

//
//
//

internal void
WriteRUI(loaded_rui *RUI, char *FileName)
{
    FILE *Out = fopen(FileName, "wb");
    if(Out)
    {
    }
}

//
//
//

int
main(int ArgCount, char **Args)
{
}

