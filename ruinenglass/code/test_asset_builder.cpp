/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Michael Chow $
   $Notice: $
   ======================================================================== */

#include "test_asset_builder.h"

//
// IMPORTANT(chowie): This is an _offline_ asset packer
//

// RESOURCE(): https://handmade.network/p/29/swedish-cubes-for-unity/blog/p/2723-how_media_molecule_does_serialization
// TODO(chowie): Try out this type of serialisation (for game + world)!

internal string
ReadEntireFile(char *FileName)
{
    string Result = {};

    FILE *In = fopen(FileName, "rb"); // STUDY(chowie): "r" = read, "b" = binary
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
LoadBMP(char *FileName)
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

    // NOTE(chowie): Guard against multiple bitmap
    // pitches, fixed or padded to a particular thing.
    Result.Pitch = Result.Dim.Width*BITMAP_BYTES_PER_PIXEL;

    return(Result);
}

internal builder_loaded_repligram
LoadRepligram(char *FileName)
{
    builder_loaded_repligram Result = {};

    string ReadResult = ReadEntireFile(FileName);
    if(ReadResult.Size != 0)
    {
        Result.Free = ReadResult.Data;

        repligram_header *Header = (repligram_header *)ReadResult.Data;
        Assert(Header->MagicValue == REPLIGRAM_MAGIC_VALUE);

        // TODO(chowie): Do more stuff here!
        Result.Dim = Header->Dim;
    }

    return(Result);
}

internal void
WriteRepligram(rui_repligram Repligram, char *FileName)
{
}

//
//
//

internal void
BeginAssetType(loaded_rui *RUI, asset_type_id TypeID)
{
    Assert(RUI->DEBUGAssetType == 0); // NOTE(chowie): Assumes not multithreaded for now!

    RUI->DEBUGAssetType = RUI->AssetTypes + TypeID;
    RUI->DEBUGAssetType->TypeID = TypeID;
    RUI->DEBUGAssetType->FirstAssetIndex = RUI->AssetCount;
    RUI->DEBUGAssetType->OnePastLastAssetIndex = RUI->DEBUGAssetType->FirstAssetIndex;
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
    Assert(RUI->DEBUGAssetType);
    RUI->AssetCount = RUI->DEBUGAssetType->OnePastLastAssetIndex;
    RUI->DEBUGAssetType = 0;
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
    Assert(RUI->DEBUGAssetType);
    Assert(RUI->DEBUGAssetType->OnePastLastAssetIndex < ArrayCount(RUI->Assets));

    u32 Index = RUI->DEBUGAssetType->OnePastLastAssetIndex++;
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

internal repligram_id
AddRepligramAsset(loaded_rui *RUI, char *FileName, v3u Dim)
{
    added_asset Added = AddAsset(RUI);
    Added.Asset->Repligram.Dim = Dim; // TODO(chowie): Not sure if I need this exactly?
    Added.Source->Type = AssetType_Repligram;
    Added.Source->Repligram.FileName = FileName;

    repligram_id Result = {Added.ID};
    return(Result);
}

// RESOURCE(): http://hero.handmade.network/forums/code-discussion/t/1114-storing_strings_data
// TODO(chowie): Add strings assets

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
    FILE *Out = fopen(FileName, "wb"); // STUDY(chowie): "w" = write, "b" = binary
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

        // NOTE(chowie): Byte offset
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
        // NOTE(chowie): Asset file creation, write all assets
        //

        fseek(Out, AssetArraySize, SEEK_CUR);
        for(u32 AssetIndex = 1; // NOTE(chowie): First file intentionally left blank
            AssetIndex < Header.AssetCount;
            ++AssetIndex)
        {
            builder_asset_source *Source = RUI->AssetSources + AssetIndex;
            rui_asset *Dest = RUI->Assets + AssetIndex;

            // NOTE(chowie): Position
            Dest->DataOffset = ftell(Out); // NOTE(chowie): Use ftelli64 if past 4GB/64-bit

            switch(Source->Type)
            {
                // TODO(chowie): Add AssetType_Sounds
                case AssetType_Repligram:
                {
                } break;

                // TODO(chowie): Intentionally fallthrough to next case!
                case AssetType_Font:
                {
                } break;

                case AssetType_Bitmap:
                {
                    builder_loaded_bitmap Bitmap;
                    Bitmap = LoadBMP(Source->Bitmap.FileName);
                    Dest->Bitmap.Dim = V2U(Bitmap.Dim);

                    fwrite(Bitmap.Memory, Bitmap.Dim.Width*Bitmap.Dim.Height*BITMAP_BYTES_PER_PIXEL, 1, Out);

                    free(Bitmap.Free);
                } break;

                InvalidDefaultCase;
            }
        }
        fseek(Out, (u32)Header.AssetsOffset, SEEK_SET); // NOTE(chowie): Use lseek if past 4GB/64-bit
        fwrite(RUI->Assets, AssetArraySize, 1, Out);

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
    RUI->DEBUGAssetType = 0;
    RUI->AssetTypeIndex = 0;

    RUI->AssetTypeCount = Asset_Count;
    memset(RUI->AssetTypes, 0, sizeof(RUI->AssetTypes));
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

// TODO(chowie): Add simple compression!
int
main(int ArgCount, char **Args)
{
    WriteNonPlayer();
}

//
// LBP Serializer (still testing)
//

// TODO(chowie): Move this out into its own file!
// RESOURCE(): https://www.oskarmendel.me/p/brainroll-postmortem-part-5-assets
// RESOURCE(): https://handmade.network/p/29/swedish-cubes-for-unity/blog/p/2723-how_media_molecule_does_serialization

struct debug_state
{
    u32 DebugState;
    u32 Points;
};

// NOTE(chowie): Every time you want to make a change, add to this list (don't reorder)!
// TODO(chowie): Replace with proper data
// STUDY(chowie): This is the equivalent of 'writing' the location
enum serialization_versions
{
    SV_Scores = 1,
    SV_ExtraPlayers,
    SV_Fouls,

    SV_LatestPlusOne, // NOTE(chowie): Keep this as the last element!
};
#define SV_Latest (SV_LatestPlusOne - 1)

struct lbp_serializer
{
    u32 DataVersion;
    b32 IsWriting;

    // FILE *In; // TODO(chowie): Any point to saving file handle here (instead of buffer)?
    u32 Counter; // NOTE(chowie): Checks for integrity

    buffer Buffer; // TODO(chowie): Replace with arenas? Replace with u32, not umm?
};

internal void
Serialize(lbp_serializer *Serializer, u32 *Datum)
{
    if(Serializer->IsWriting)
    {
        // NOTE(chowie): Equivant to alloc (write)
        u32 *Pointer = (u32 *)(Serializer->Buffer.Data + Serializer->Buffer.Size);
        *Pointer = *Datum;
    }
    else
    {
        // NOTE(chowie): Equivant to realloc (read)
        *Datum = *(u32 *)(Serializer->Buffer.Data + Serializer->Buffer.Size);
    }
    Serializer->Buffer.Size += sizeof(u32);
}

#define VerifySerializerIntegrity(CheckAdded)          \
    if(Serializer->DataVersion >= (CheckAdded))        \
    {                                                  \
        u32 Verify = Serializer->Counter;              \
        Serializer(Serializer, &Verify);               \
        Assert(Verify == Serializer->Counter++);       \
    }                                                  \

#define VersionInRange(From, To)                       \
    ((Serializer->DataVersion >= (From)) &&            \
     (Serializer->DataVersion < (To)))                 \

#define SerializeAdd(FieldAdded, FieldName)            \
    if(Serializer->DataVersion >= FieldAdded)          \
    {                                                  \
        Serialize(Serializer, &(Datum->FieldName));    \
    }                                                  \

#define SerializeRemove(FieldAdded, FieldName, type, FieldRemoved, DefaultValue) \
    type FieldName = (DefaultValue);                   \
    if(VersionInRange((FieldAdded), (FieldRemoved))    \
    {                                                  \
        Serialize(Serializer, &(FieldName));           \
    }                                                  \

//
// TODO(chowie): Add whatever state you'd like to serialize!
//

enum serialization_state_type
{
    LBPState_Debug,

    LBPState_Count,
};

// COULDDO(chowie): Do something like this to generaliser for different state serialization?
// #define PushRenderElement(RenderGroup, type) PushRenderElement_(RenderGroup, sizeof(type), RenderGroupEntryType_##type)
// #define SERIALIZESTATE(name, type) void name(lbp_serializer *Serializer, type *State)
// typedef SERIALIZESTATE(serialize_state);

// TODO(chowie): Complete this test case!
internal void
Serialize(lbp_serializer *Serializer, debug_state *Datum)
{
    SerializeAdd(SV_ExtraPlayers, Points);
}

// COULDDO(chowie): Convert this to a "#define"?
internal b32x
SerializeIncludingVersion(lbp_serializer *Serializer, debug_state *Datum)
{
    b32x Result = false;
    if(Serializer->IsWriting)
    {
        Serializer->DataVersion = SV_Latest;
    }

    Serialize(Serializer, &Serializer->DataVersion);

    // NOTE(chowie): Stays false if reading from a file that comes after this one!
    if(Serializer->DataVersion <= SV_Latest)
    {
        Serialize(Serializer, Datum);
        Result = true;
    }

    return(Result);
}

