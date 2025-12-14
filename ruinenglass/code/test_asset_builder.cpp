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
        // STUDY(chowie): Hacky way to get location of the entire
        // file! Seek = location.
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

        // STUDY(chowie): Cold-cast
        bitmap_header *Header = (bitmap_header *)ReadResult.Data;
        u32 *Pixels = (u32 *)((u8 *)ReadResult.Data + Header->BitmapOffset);
        Result.Memory = Pixels;
        Result.Dim = Header->Dim;

        Assert(Result.Dim.Height >= 0);
        Assert(Header->Compression == 3);
        // NOTE(chowie): If you are using this generically, remember
        // bmp files can go in either directions and the height will
        // be negative for top-down. Also, compression etc... Not
        // complete BMP code.

        // NOTE(chowie): Byte order is determined by the header, read
        // out the masks and convert the pixels ourselves.
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
BeginAssetType(loaded_rui *RUI, asset_type_id ID)
{
    Assert(RUI->AssetType == 0); // NOTE(chowie): Assumes not multithreaded for now!

    RUI->AssetType = RUI->AssetType + ID;
    RUI->AssetType->TypeID = ID;
    RUI->AssetType->FirstAssetIndex = RUI->AssetCount;
    RUI->AssetType->OnePastLastAssetIndex = RUI->AssetType->FirstAssetIndex;
}

internal void
AddTag(loaded_rui *RUI, asset_type_id ID, f32 Value)
{
    Assert(RUI->AssetTypeIndex != 0);

    rui_asset *Asset = RUI->Assets + RUI->AssetTypeIndex;
    ++Asset->OnePastLastTagIndex;

    rui_tag *Tag = RUI->Tags + RUI->TagCount++;
    Tag->ID = ID;
    Tag->Value = Value;
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

//
//
//

// struct import_grid_tag
// {
//     u32 FirstTagIndex;
//     u32 OnePastLastTagIndex;
// };
// 
// struct tag_builder
// {
//     loaded_rui *RUI;
//     u32 FirstTagIndex;
//     b32x Error;
// };
// 
// internal void
// AddTag(tag_builder *TagBuilder, asset_tag_id ID, f32 Value)
// {
//     if(TagBuilder->RUI->TagCount < BUILDER_MAX_SIZE)
//     {
//         rui_tag *Tag = TagBuilder->RUI->Tags + TagBuilder->RUI->TagCount++;
//         Tag->ID = ID;
//         Tag->Value = Value;
//     }
//     else
//     {
//         TagBuilder->Error = true;
//     }
// }
// 
// internal tag_builder
// BeginTags(loaded_rui *RUI)
// {
//     tag_builder Result = {};
//     Result.RUI = RUI;
//     Result.FirstTagIndex = RUI->TagCount;
//     
//     return(Result);
// }
// 
// internal import_grid_tag
// EndTags(tag_builder *TagBuilder, asset_basic_category Category)
// {
//     import_grid_tag Result = {};
//     if(Category != Category_None)
//     {
//         AddTag(TagBuilder, Tag_BasicCategory, (f32)Category);
//     }
// 
//     Result.FirstTagIndex = TagBuilder->FirstTagIndex;
//     Result.OnePastLastTagIndex = TagBuilder->RUI->TagCount;
//     
//     return(Result);
// }

//
//
//

// STUDY(chowie): Position for a stream in fwrite is a bad idea
// since it's hidden state. Would rather design it to be queued
// and allows out-of-order queue (doesn't need multithreading):
// (HmH 148)
// Write LocationA sizeof(A)
// Write LocationB sizeof(B)
// And not (only works for single threaded, no queue or force FIFO):
// Write sizeof(A)
// Write sizeof(B)
// Also working around this by (is broken by multithreading). Seeks
// must be bundled with write _always_:
// Seek A -> Write A
// Seek B -> Write B
// Basic streaming could look like in a struct
// u64 Location
// FILE *Handle
internal void
WriteRUI(loaded_rui *RUI, char *FileName)
{
    FILE *Out = fopen(FileName, "wb");
    if(Out)
    {
        rui_header Header = {};
        Header.MagicValue = RUI_MAGIC_VALUE;
        Header.Version = RUI_VERSION;
        Header.TagCount = RUI->TagCount;
        Header.AssetCount = RUI->AssetCount;
        Header.AssetTypeCount = Asset_Count; // TODO(chowie): Sparseness?

        //
        //
        //

        u32 TagArraySize = Header.TagCount*sizeof(rui_tag); // NOTE(chowie): Use a lower-level write if you need write u64
        u32 AssetTypeArraySize = Header.AssetTypeCount*sizeof(rui_asset_type);
        u32 AssetArraySize = Header.AssetCount*sizeof(rui_asset);

        Header.TagsOffset = sizeof(Header);
        Header.AssetTypesOffset = Header.TagsOffset + TagArraySize;
        Header.AssetsOffset = Header.AssetTypesOffset + AssetTypeArraySize;

        //
        //
        //

        fwrite(&Header, sizeof(Header), 1, Out);
        fwrite(RUI->Tags, TagArraySize, 1, Out);
        fwrite(RUI->AssetTypes, AssetTypeArraySize, 1, Out);

        //
        //
        //

//        fwrite(RUI->Assets, AssetArraySize, 1, Out);

        fclose(Out);
    }
    else
    {
        printf("ERROR: Couldn't open file :(\n");
    }
}

//
//
//

internal void
InitRUI(loaded_rui *RUI)
{
    RUI->AssetCount = 1;
    RUI->TagCount = 1;
    RUI->AssetTypeIndex = 0;

    RUI->AssetTypeCount = Asset_Count;
    memset(RUI->Tags, 0, sizeof(RUI->Tags));
}

internal void
WriteFonts(void)
{
    loaded_rui RUI_;
    loaded_rui *RUI = &RUI_;
    InitRUI(RUI);

    WriteRUI(RUI, "Fonts.rui");
}

internal void
WriteNonPlayer(void)
{
    loaded_rui RUI_;
    loaded_rui *RUI = &RUI_;
    InitRUI(RUI);

    BeginAssetType(RUI, Asset_DEBUG_Bush);
    AddBitmapAsset(RUI, "TestBMP/test_bush_00.bmp");
    EndAssetType(RUI);

    BeginAssetType(RUI, Asset_DEBUG_Lilypad);
    AddBitmapAsset(RUI, "TestBMP/test_lilypad_00.bmp");
    EndAssetType(RUI);

    BeginAssetType(RUI, Asset_DEBUG_Lotus);
    AddBitmapAsset(RUI, "TestBMP/test_lotus_00.bmp");
    EndAssetType(RUI);

    WriteRUI(RUI, "DEBUG_NonPlayer.rui");
}

int
main(int ArgCount, char **Args)
{
    WriteNonPlayer();
}

