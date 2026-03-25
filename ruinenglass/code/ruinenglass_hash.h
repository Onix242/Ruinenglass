#if !defined(RUINENGLASS_HASH_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Michael Chow $
   $Notice: $
   ======================================================================== */

// TODO(chowie): Better hash functions

//
// GENERAL
//

// RESOURCE(reed): https://www.reedbeta.com/blog/hash-functions-for-gpu-rendering/
// NOTE(chowie): Compared to Wang Hash, slightly better performance and much better statistical quality
inline u32
PCGHash(u32 Value)
{
    u32 State = Value*747796405u + 2891336453u;
    u32 Word = ((State >> ((State >> 28u) + 4u)) ^ State)*277803737u;
    u32 Result = (Word >> 22u) ^ Word;
    return(Result);
}

//
// STRING
//

// RESOURCE(): https://theartincode.stanis.me/008-djb2/
inline u32
DJB2Hash(char Scan)
{
    u32 MagicNumber = 5381;
    u32 Result = ((MagicNumber << 5) + MagicNumber) + Scan;
    return(Result);
}

// NOTE(chowie): Compact implementation that balances processor cache usage against speed.
// RESOURCE(schelling): https://github.com/schellingb/ZIPValidateCRC/blob/main/ZIPValidateCRC.cpp
// RESOURCE(karl malbrain): http://www.geocities.ws/malbrain/
global u32 CRC32Table[16] = { 0, 0x1db71064, 0x3b6e20c8, 0x26d930ac, 0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c, 0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c, 0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c };
inline u32
CRC32(void *Data, umm Size)
{
    u32 Result = (u32)~(u32)0;
    for(u8 CRCIndex = 0, *Base = (u8 *)Data; // TODO(chowie): Does "*Base = u8 *Base"?
        Size--;
        )
    {
        CRCIndex = *Base++;
        Result = (Result >> 4) ^ CRC32Table[(Result & 0xF) ^ (CRCIndex & 0xF)];
        Result = (Result >> 4) ^ CRC32Table[(Result & 0xF) ^ (CRCIndex >> 4)];
    }
    return(~Result);
}

//
// SPATIAL
//

// NOTE(by mauro): Using Szudnik Pairing.
// * Seed would always be the same based on location, and collisions would only occur as you got very far away from the origin
// * You could fit two 16-bit integers into a single 32-bit integer with no collisions.
// RESOURCE(mauro): https://dmauro.com/post/77011214305/a-hashing-function-for-x-y-z-coordinates
inline u32
MauroHash(v3u Value)
{
    u32 Max = Max3(Value.x, Value.y, Value.z);
    u32 Result = CUBIC(Max) + (2*Max*Value.z) + Value.z;
    if(Max == Value.z)
    {
        Result += QUADRATIC(Max(Value.x, Value.y));
    }

    if(Value.y >= Value.x)
    {
        Result += (Value.x + Value.y);
    }
    else
    {
        Result += Value.y;
    }

    return(Result);
}

inline s32
MauroHash(v3s Value)
{
    s32 NegX = (Value.x >= 0) ? (2 * Value.x) : (-2*Value.x - 1);
    s32 NegY = (Value.y >= 0) ? (2 * Value.y) : (-2*Value.y - 1);
    s32 NegZ = (Value.z >= 0) ? (2 * Value.z) : (-2*Value.z - 1);

    s32 Max = Max3(NegX, NegY, NegZ);
    s32 Result = CUBIC(Max) + (2*Max*NegZ) + NegZ;
    if(Max == NegZ)
    {
        Result += QUADRATIC(Max(NegX, NegY));
    }

    if(NegY >= NegX)
    {
        Result += (NegX + NegY);
    }
    else
    {
        Result += NegY;
    }

    return(Result);
}

// TODO(chowie): Spatial Hash Table? For particles?
// RESOURCE(): https://matthias-research.github.io/pages/tenMinutePhysics/index.html
// RESOURCE(cincotti): https://carmencincotti.com/2022-10-31/spatial-hash-maps-part-one/#how-to-populate-a-dense-hash-table
// NOTE(chowie): Originally intended for s32, with an Abs. Why not
// just type cast to u32?
inline u32
MullerHash(v3u Value)
{
    u32 Result = (Value.x*92837111) ^ (Value.y*689287499) ^ (Value.z*283923481);
    return(Result);
}

inline u32
MullerHash(v3s Value)
{
    s32 Result = (Value.x*92837111) ^ (Value.y*689287499) ^ (Value.z*283923481);
    Result = Abs(Result);
    return(Result);
}

// RESOURCE(codejar): https://youtu.be/OjZ7vV62lJk?t=271
/*
inline u32
Hash(v4u Value)
{
    u32 Mult = 1103515245u;
    Value *= Mult;
    u32 Result = ((Value >> 2u) ^ (Value.yzwx >> 1u) ^ (Value.zwxy >> 3u) ^ (Value.wxyz >> 4) * Mult);
    return(Result);
}
*/

// TODO(chowie): Try to use this for tile/world storage? (Must be in u32)
// RESOURCE(fabian): https://fgiesen.wordpress.com/2009/12/13/decoding-morton-codes/

// NOTE(by fabian): "Insert" a 0 bit after each of the 16 low bits of x
inline u32
Part1By1(u32 Value)
{
    Value &= 0x0000ffff;                          // Value = ---- ---- ---- ---- fedc ba98 7654 3210
    Value = (Value ^ (Value <<  8)) & 0x00ff00ff; // Value = ---- ---- fedc ba98 ---- ---- 7654 3210
    Value = (Value ^ (Value <<  4)) & 0x0f0f0f0f; // Value = ---- fedc ---- ba98 ---- 7654 ---- 3210
    Value = (Value ^ (Value <<  2)) & 0x33333333; // Value = --fe --dc --ba --98 --76 --54 --32 --10
    Value = (Value ^ (Value <<  1)) & 0x55555555; // Value = -f-e -d-c -b-a -9-8 -7-6 -5-4 -3-2 -1-0
    return(Value);
}

// NOTE(by fabian): "Insert" two 0 bits after each of the 10 low bits of Value
inline u32
Part1By2(u32 Value)
{
    Value &= 0x000003ff;                          // Value = ---- ---- ---- ---- ---- --98 7654 3210
    Value = (Value ^ (Value << 16)) & 0xff0000ff; // Value = ---- --98 ---- ---- ---- ---- 7654 3210
    Value = (Value ^ (Value <<  8)) & 0x0300f00f; // Value = ---- --98 ---- ---- 7654 ---- ---- 3210
    Value = (Value ^ (Value <<  4)) & 0x030c30c3; // Value = ---- --98 ---- 76-- --54 ---- 32-- --10
    Value = (Value ^ (Value <<  2)) & 0x09249249; // Value = ---- 9--8 --7- -6-- 5--4 --3- -2-- 1--0
    return(Value);
}

inline u32
EncodeMortonV2U(v2u Value)
{
    u32 Result = (Part1By1(Value.y) << 1) + Part1By1(Value.x);
    return(Result);
}

inline u32
EncodeMortonV3U(v3u Value)
{
    u32 Result = (Part1By2(Value.z) << 2) + (Part1By2(Value.y) << 1) + Part1By2(Value.x);
    return(Result);
}

// NOTE(by fabian): Inverse of Part1By1 - "delete" all odd-indexed bits
inline u32
Compact1By1(u32 Value)
{
    Value &= 0x55555555;                          // Value = -f-e -d-c -b-a -9-8 -7-6 -5-4 -3-2 -1-0
    Value = (Value ^ (Value >>  1)) & 0x33333333; // Value = --fe --dc --ba --98 --76 --54 --32 --10
    Value = (Value ^ (Value >>  2)) & 0x0f0f0f0f; // Value = ---- fedc ---- ba98 ---- 7654 ---- 3210
    Value = (Value ^ (Value >>  4)) & 0x00ff00ff; // Value = ---- ---- fedc ba98 ---- ---- 7654 3210
    Value = (Value ^ (Value >>  8)) & 0x0000ffff; // Value = ---- ---- ---- ---- fedc ba98 7654 3210
    return(Value);
}

// NOTE(by fabian): Inverse of Part1By2 - "delete" all bits not at positions divisible by 3
inline u32
Compact1By2(u32 Value)
{
    Value &= 0x09249249;                          // Value = ---- 9--8 --7- -6-- 5--4 --3- -2-- 1--0
    Value = (Value ^ (Value >>  2)) & 0x030c30c3; // Value = ---- --98 ---- 76-- --54 ---- 32-- --10
    Value = (Value ^ (Value >>  4)) & 0x0300f00f; // Value = ---- --98 ---- ---- 7654 ---- ---- 3210
    Value = (Value ^ (Value >>  8)) & 0xff0000ff; // Value = ---- --98 ---- ---- ---- ---- 7654 3210
    Value = (Value ^ (Value >> 16)) & 0x000003ff; // Value = ---- ---- ---- ---- ---- --98 7654 3210
    return(Value);
}

inline v2u
DecodeMorton2(u32 Value)
{
    v2u Result =
    {
        Compact1By1(Value >> 0),
        Compact1By1(Value >> 1),
    };

    return(Result);
}

inline v3u
DecodeMorton3(u32 Value)
{
    v3u Result =
    {
        Compact1By2(Value >> 0),
        Compact1By2(Value >> 1),
        Compact1By2(Value >> 2),
    };

    return(Result);
}

// RESOURCE(): http://www.volumesoffun.com/implementing-morton-ordering-for-chunked-voxel-data/
// NOTE(chowie): Fabian mentions to step in an axis doesn't require decoding!
inline u32
EncodeMortonAxis(u32 MortonAxis, s32 Step)
{
    u32 IncrementAxis = ((MortonAxis & Step) - Step) & Step;
    u32 Result = (MortonAxis & ~Step) | IncrementAxis;
    return(Result);
}

// TODO(chowie): Double check this is correcet!
inline u32
EncodeMorton3D(u32 MortonAxis, v3s Step, v3s dAxis)
{
    v3u Axis =
    {
        ((MortonAxis & Step.x) - EncodeMortonAxis(MortonAxis, -dAxis.x)) & Step.x,
        ((MortonAxis & Step.y) - EncodeMortonAxis(MortonAxis, -dAxis.y)) & Step.y,
        ((MortonAxis & Step.z) - EncodeMortonAxis(MortonAxis, -dAxis.z)) & Step.z,
    };
    u32 Result = Axis.x | Axis.y | Axis.z;
    return(Result);
}

//
// TRIANGLE NUMBERS
//

inline u32
TriangleNumber(u32 Value)
{
    u32 Result = (Value*(Value + 1) / 2);
    return(Result);
}

inline u64
TriangleNumber64(u64 Value)
{
    u64 Result = (Value*(Value + 1) / 2);
    return(Result);
}

// RESOURCE(mason remaley): https://gamesbymason.com/2020/03/30/symmetric-matrices/
// NOTE(chowie): For symmetric tables/matrix
inline u32
TriangleNumberMat(u32 Value)
{
    u32 Result = Value + TriangleNumber(Value);
    return(Result);
}

inline u64
TriangleNumberMat64(u64 Value)
{
    u64 Result = Value + TriangleNumber64(Value);
    return(Result);
}

// RESOURCE(mason remaley): https://gamesbymason.com/2020/03/30/symmetric-matrices/
// f(x,y) = (x + (y)(y+1)/2), x = low/column, y = high/triangle
// NOTE(chowie): For asymmetric tables/matrix
inline u32
TriangleNumberMat(v2u Value)
{
    u32 Result = Value.x + TriangleNumber(Value.y);
    return(Result);
}

inline u64
TriangleNumberMat64(v2u64 Value)
{
    u64 Result = Value.x + TriangleNumber64(Value.y);
    return(Result);
}

// RESOURCE(): https://stackoverflow.com/questions/2913215/fastest-method-to-define-whether-a-number-is-a-triangular-number
inline f32
IsTriangleNumber(u32 Value)
{
    f32 Result = (Sqrt(8*(f32)Value + 1) - 1)/2;
    return(Result);
}

inline f64
IsTriangleNumber64(u64 Value)
{
    f64 Result = (Sqrt(8*(f64)Value + 1) - 1)/2;
    return(Result);
}

inline u16
GetPairwiseRow16(u32 Ordinal)
{
    f32 Row = IsTriangleNumber(Ordinal);
    u16 Result = FloorF32ToU16(Row); // STUDY(chowie): New technique I observed to quickly get the row!
    return(Result);
}

inline u64
GetPairwiseRow64(u64 Ordinal)
{
    f64 Row = IsTriangleNumber64(Ordinal);
    u64 Result = FloorF64ToU64(Row); // STUDY(chowie): New technique I observed to quickly get the row!
    return(Result);
}

//
// Remaley Hash (2D, pairwise)
//

// COULDDO(chowie): Where to take these next:
// - Hash this?
// - Convert numbers to enum
// - Figure out how this integrates with (debug) UI/chart?

// RESOURCE(mason remaley): https://gamesbymason.com/2020/03/30/symmetric-matrices/
internal u32
RemaleyHash(u16 A, u16 B)
{
    v2u Pair = {(u32)B, (u32)A}; // NOTE(chowie): In general, most will automatically write A > B
    if(A < B)
    {
        Swap(u32, Pair.a, Pair.b);
    }

    u32 Result = TriangleNumberMat(Pair);
    return(Result);
}

// NOTE(chowie): Alternative trick to get row if I need this later!
// STUDY(chowie): Trick I made to avoid floating point precision with
// IsInteger() Signof check ensures it's always past the first index!
// (SignOf(Ordinal - TriangleNumber((u32)Result.Row)) == -1) ? false : true;
struct pairwise_index_result
{
    b32x IsTriangleNumber;
    u16 Row;
    u16 Col;
};
internal pairwise_index_result
InvertRemaleyHash(u32 Ordinal)
{
    u16 Row = GetPairwiseRow16(Ordinal);
    u16 Pitch = (u16)TriangleNumber(Row); // STUDY(chowie): Neat way I found to handle 0th case without a clamp

    pairwise_index_result Result = {};
    Result.Row = Row;
    Result.Col = (u16)Ordinal - Pitch;
    Result.IsTriangleNumber = (Ordinal == TriangleNumberMat((u32)Result.Row));
    return(Result);
}

// TODO(chowie): Change this to a u16 percentage with a 2D invlerp!
enum whittaker_biome : u16
{
    Whittaker_Biome_Null,

    Whittaker_Biome_Rainforest,
    Whittaker_Biome_Seasonalforest,
    Whittaker_Biome_Forest,
    Whittaker_Biome_Swampland,
    Whittaker_Biome_Plains,
    Whittaker_Biome_Shrubland,
    Whittaker_Biome_Taiga,
    Whittaker_Biome_Desert,
    Whittaker_Biome_Savanna,
    Whittaker_Biome_Tundra,

    Whittaker_Biome_Count,
};

// NOTE(chowie): Triangle Number Usage: Pack -> Unpack
// u32 EntityPairwise = MapPairToPairwise(TestHash_EntityType_Wall, TestHash_EntityType_Wall);
// pairwise_index_result TestPair = GetPairFromPairwise(EntityPairwise);
enum testhash_entity_type : u16
{
    TestHash_EntityType_Null,

    TestHash_EntityType_Space,
    TestHash_EntityType_Hero,
    TestHash_EntityType_Wall,

    TestHash_EntityType_Count,
};

#define PAIRWISE_TABLE_MAX(TableDim) (TableDim * (TableDim + 1) / 2)
struct pairwise
{
    enum16(testhash_entity_type) TypeTable[PAIRWISE_TABLE_MAX(TestHash_EntityType_Count)];
};

// COULDDO(chowie): Use for hash tables??
// TODO(chowie): Could hold next - 1 to get the current and to prevent
// wrapping. e.g. See debug system (DebugState->NextFreeFrame - 1) %
// DEBUG_FRAME_COUNT.
// TODO(chowie): Refer to HmH Day 354 29:50 for how to look back to a prev value!
inline void
IncrementOrdinal(u32 *Ordinal)
{
    *Ordinal = (*Ordinal + 1) % TestHash_EntityType_Count;
}
//    u32 *TriangleNumberOrdinal;
//    Pairwise->TriangleNumberOrdinal = &Ordinal;
//    printf("Not at a triangle number: I'm here at ordinal %d!\n", *Pairwise->TriangleNumberOrdinal);

//
// Fink Hash (3D, non-pairwise)
//

// COULDDO(chowie): Can I do better than linear?
// RESOURCE(alex fink & jorg rhiemeier): https://listserv.brown.edu/archives/cgi-bin/wa?A2=ind0907B&L=CONLANG&P=R12478
// P.S. Original concept from 2009 amazingly! It's not even your main point??
// f(x,y) = 1 + 2(x + (x+y)(x+y+1)/2)
// STUDY(chowie): Many invertible hashes fundamentally uses odd numbers, won't work without it.
inline u64
FinkHash(v2u64 Value)
{
    v2u64 Offset = {Value.x, Value.x + Value.y};
    u64 Result = 2*TriangleNumberMat64(Offset) + 1; // NOTE(chowie): Parents must be odd, leaves even for children
    return(Result);
}

// f(x,y) = 1 + 2(x + (x+y)(x+y+1)/2) -- becomes --> = (x + (x+y)(x+y+1)/2)
// In other words, "2*TriangleNumberMat(Offset) + 1" -- becomes --> "TriangleNumberMat(Offset)"
inline u64
RemoveFinkHashParity(u64 Hash)
{
    u64 Result = (Hash - 1)/2;
    return(Result);
}

// STUDY(chowie): Many invertible hashes must use odd numbers to work.
internal v2u64
InvertFinkHash(u64 Hash)
{
    // Step 1. Remove parity from equation and leaves triangle
    u64 CurrentTriangleNumber = RemoveFinkHashParity(Hash);
    // Step 2. Solve for x+y (is row = parent node). Inverts tree, mirrors implicit tree see IsTriangleNumber64()
    u64 Row = GetPairwiseRow64(CurrentTriangleNumber);

    // Step 3. Substitute x+y (ResultXY), "CurrentTriangleNumber = (x + (x+y)(x+y+1)/2)"
    // Solve for x, "CurrentTriangleNumber - (x+y)(x+y+1)/2)"
    // Solve for y, "x+y - x"
    v2u64 Result =
    {
        CurrentTriangleNumber - TriangleNumber64(Row), // NOTE(chowie): Pitch/Width
        Row - Result.x,
    };

    return(Result);
}

#define FINKHASHTREE_MAXLEAVES 8
struct finkhashtree
{
    u64 FinkHash;
    u32 MaxDepth;

    //
    // NOTE(chowie): Below are reconstructed on-the-fly that may want to be viewed
    //

    u32 LeafCount;
    u16 Leaves[FINKHASHTREE_MAXLEAVES];
//    u64 StackCount;
//    stack_entry *Stack; // COULDDO(chowie): Stack[StackCount] OR Stack[(MaxDepth*2 + 1)]
    // NOTE(onix242): Though I may change my mind later, I'm explicitly storing the leaves because
    // I think I'm specifically looking for a list of the nodes touched, but you could probably
    // remove it when optimising.
};

enum finkhashtree_type
{
    FinkHashTree_Type_Read,
    FinkHashTree_Type_ReadWriteReplace,
    FinkHashTree_Type_ReadWriteAdd,
    FinkHashTree_Type_ReadWriteDelete,
};

// RESOURCE(): https://stackoverflow.com/questions/480811/semantic-difference-between-find-and-search
// STUDY(chowie): Difference between "find()" vs "search()" is find typically returns a
// single record while search is for multiple.
// TODO(chowie): Collapse down?
enum finkhashtree_mode
{
    FinkHashTree_Mode_None, // NOTE(chowie): Only for FinkHashTreeReadAll()
    FinkHashTree_Mode_Index,
    FinkHashTree_Mode_FirstOfValue, // NOTE(chowie): Finds first leaf/materials if multiple are the same
};

struct finkhashtree_data
{
    union
    {
        u16 SourceLeafValue;
        u16 SourceLeafIndex;
    };
    u16 DestLeafValue;
};

struct finkhashtree_params
{
    finkhashtree_type Type;
    finkhashtree_mode Mode;
    finkhashtree_data Data;
    // COULDDO(chowie): Optional flag to see the old change vs new change? Especially if using index based.
    // NOTE(chowie): If you wanted a list of materials + can accumulate across multiple blocks
    b32x CompareTreeLeafChange;

    //
    // NOTE(chowie): Below only is reconstructed on-the-fly
    // Applies to FinkHashTree_Type_Read + find leaf value/index
    //

    u16 FoundLeaf;
    b32x FoundLeafStatus;
};

struct finkhashtree_group
{
    finkhashtree Tree;
    finkhashtree_params Params;
};

internal finkhashtree_group
InitFinkHashTreeGroup(u64 Hash, finkhashtree_params Params, u32 MaxDepth = 3)
{
    finkhashtree_group Result = {};
    Result.Tree.FinkHash = Hash;
    Result.Tree.MaxDepth = MaxDepth;
    Result.Params = Params;
    return(Result);
}

inline finkhashtree_params
FinkHashTreeDefaultParams(finkhashtree_type Type, finkhashtree_mode Mode, u16 SourceLeafIndex, u16 DestLeafValue, b32x CompareTreeLeafChange = false)
{
    finkhashtree_params Params = {};
    Params.Type = Type;
    Params.Mode = Mode;
    Params.Data.SourceLeafIndex = SourceLeafIndex;
    Params.Data.DestLeafValue = DestLeafValue;
    Params.CompareTreeLeafChange = CompareTreeLeafChange;
    return(Params);
}

// NOTE(chowie): Params.Mode ZII defaults to FinkHashTree_OpMode_None
inline finkhashtree_params
FinkHashTreeReadAllParams(b32x CompareTreeLeafChange = false)
{
    finkhashtree_params Params = {};
    Params.Type = FinkHashTree_Type_Read;
    Params.CompareTreeLeafChange = CompareTreeLeafChange;
    return(Params);
}

// COULDDO(chowie): Add "contains how many" by inspecting the leaf array?
inline finkhashtree_params
FinkHashTreeReadParams(finkhashtree_mode Mode, u16 SourceLeafIndex, b32x CompareTreeLeafChange = false)
{
    finkhashtree_params Params = FinkHashTreeDefaultParams(FinkHashTree_Type_Read,
                                                           Mode, SourceLeafIndex, 0, CompareTreeLeafChange);
    return(Params);
}

inline finkhashtree_params
FinkHashTreeReplaceParams(finkhashtree_mode Mode, u16 SourceLeafIndex, u16 DestLeafValue, b32x CompareTreeLeafChange = false)
{
    finkhashtree_params Params = FinkHashTreeDefaultParams(FinkHashTree_Type_ReadWriteReplace,
                                                           Mode, SourceLeafIndex, DestLeafValue, CompareTreeLeafChange);
    return(Params);
}

inline finkhashtree_params
FinkHashTreeAddParams(finkhashtree_mode Mode, u16 SourceLeafIndex, u16 DestLeafValue, b32x CompareTreeLeafChange = false)
{
    finkhashtree_params Params = FinkHashTreeDefaultParams(FinkHashTree_Type_ReadWriteAdd,
                                                           Mode, SourceLeafIndex, DestLeafValue, CompareTreeLeafChange);
    return(Params);
}

inline finkhashtree_params
FinkHashTreeDeleteParams(finkhashtree_mode Mode, u16 SourceLeafIndex, b32x CompareTreeLeafChange = false)
{
    finkhashtree_params Params = FinkHashTreeDefaultParams(FinkHashTree_Type_ReadWriteDelete,
                                                           Mode, SourceLeafIndex, 0, CompareTreeLeafChange);
    return(Params);
}

//
// Fink hash tree traversal
//

// NOTE(chowie): Because how f(x, y) is mapped, a neat optimisation is checking for odd in
// tree hierarchy finds leaves! 'Even natural numbers' (includes 0) = parent, 'odd' = leaves
inline b32x
IsParent(u64 Hash)
{
    b32x Result = Odd(Hash);
    return(Result);
}

// COULDDO(chowie): Swap the order around of loops?
// STUDY(chowie): N + 1 for leaves. Thus, it's the most common case!
inline b32x
IsLeaf(u64 Hash)
{
    b32x Result = !IsParent(Hash);
    return(Result);
}

// NOTE(chowie): By default you read all leaves
internal void
FinkHashTreeRead(finkhashtree_group *TreeGroup)
{
    finkhashtree *Tree = &TreeGroup->Tree;
    finkhashtree_params *Params = &TreeGroup->Params;

    stack *Frontier = {};
    Frontier = Push(Frontier, {Tree->FinkHash});
    while(!StackIsEmpty(Frontier))
    {
        if(IsParent(StackTop(Frontier)->Entry))
        {
            v2u64 Pair = InvertFinkHash(StackTop(Frontier)->Entry);
            Frontier = Pop(Frontier);

            Frontier = Push(Frontier, {Pair.y}); // 115
            Frontier = Push(Frontier, {Pair.x}); // 119

            // TODO(chowie): Do something knowing they break down to leaves! Predict in leaves path?
            // For the row, Pow2n(CurrentTreeDepth)
        }
        else
        {
            // IMPORTANT(chowie): COULDDO(chowie): This would no longer be true with LUT prefabs over prims
            Tree->Leaves[Tree->LeafCount++] = (u16)StackTop(Frontier)->Entry;

            // NOTE(chowie): Toggle to find a specific leaf
            if(Params->Mode != FinkHashTree_Mode_None)
            {
                b32x SearchToggle = (Params->Mode == FinkHashTree_Mode_Index) ?
                                    (Tree->LeafCount == (u32)Params->Data.SourceLeafIndex) :
                                    ((StackTop(Frontier)->Entry == (u64)Params->Data.SourceLeafValue) && !Params->FoundLeafStatus);
                if(SearchToggle)
                {
                    // NOTE(chowie): Searching by index = leaf value, otherwise by firstofvalue = index (opposite to search params)
                    Params->FoundLeaf = (Params->Mode == FinkHashTree_Mode_Index) ? (u16)StackTop(Frontier)->Entry : (u16)Tree->LeafCount;

                    Params->FoundLeafStatus = true;
                    if(!Params->CompareTreeLeafChange)
                    {
                        break;
                    }
                }
            }

            Frontier = Pop(Frontier);
        }
    }
}

// NOTE(chowie): Fabric LIFO stack data format for accumulator (acc)
// FinkHashCCSwap is inserted in the header when a leaf swap happens for FinkHash(a, b)
//
//         In Stack
// |-------------------------|
// | FinkHashCCSwap          |
// | Hash                    |
// |-------------------------|
// | Hash                    |
// |-------------------------|
// | Hash                    |
// |-------------------------|
// NOTE(chowie): By default you replace a leaf. Add/delete is just a replace that can be toggled.
internal void
FinkHashTreeReadWriteFullTree(finkhashtree_group *TreeGroup)
{
    finkhashtree *Tree = &TreeGroup->Tree;
    finkhashtree_params *Params = &TreeGroup->Params;

    // NOTE(chowie): Don't need to explicitly save the depth as I can implicitly recontruct
    // it smartly using LeavesInARow
    u32 Depth = 0;
    u32 PrevLeafDepth = 0; // NOTE(chowie): Handles LeafFollowedByParent case
    // TODO(chowie): Fabric->Depth? Use to see if there's room to add new nodes?

    // RESOURCE(): https://legendsoflocalization.com/articles/video-game-control-codes/
    // NOTE(chowie): CC (Control Code) or Prim/Parent = 30000, is an unused hash number
    // to swap two leaf IDs. I'm using this technique to push/string multiple swaps
    // into the fabric. The fabric when processed should 'eat' all 30000s to swap!
#define FinkHashCCSwap 30000

    // NOTE(chowie): Pairs starts as 1
    u32 LeavesInARow = 0;

    // NOTE(chowie): Only thing that needs to be stored (when in left or right subtree)
    stack *Fabric = {};
    v2u64 TopSubTreePair = InvertFinkHash(Tree->FinkHash); // TODO(chowie): Simplify?

    stack *Frontier = {};
    Frontier = Push(Frontier, {Tree->FinkHash});
    while(!StackIsEmpty(Frontier))
    {
        // NOTE(chowie): Must be checked for all nodes since the top-most
        // subtree could be a parent or _leaf_
        // NOTE(chowie): Guards the possibility that TopSubTreePair.y
        // could be a duplicate leaf, but not the right one
        if(StackTop(Frontier)->Entry == TopSubTreePair.y)
        {
            if(StackLength(Frontier) == 1) // NOTE(chowie): Swaps to right subtree once, technically this also triggers at the end
            {
                if(!Params->FoundLeafStatus)
                {
                    ZeroStruct(Fabric); // NOTE(chowie): Flush left subtree fabric
                    Fabric = Push(Fabric, {TopSubTreePair.x});
                    Fabric = Push(Fabric, {FinkHashCCSwap});
                }
                Depth = 1;
            }
        }

        if(IsParent(StackTop(Frontier)->Entry))
        {
            v2u64 Pair = InvertFinkHash(StackTop(Frontier)->Entry);

            Frontier = Pop(Frontier);
            Frontier = Push(Frontier, {Pair.y}); // 115
            Frontier = Push(Frontier, {Pair.x}); // 119

            if(!Params->FoundLeafStatus)
            {
                Fabric = Push(Fabric, {Pair.y});
            }

            ++Depth;
            LeavesInARow = 0;
        }
        else
        {
            b32x SearchToggle = (Params->Mode == FinkHashTree_Mode_Index) ?
                                (Tree->LeafCount == Params->Data.SourceLeafIndex) :
                                ((StackTop(Frontier)->Entry == Params->Data.SourceLeafValue) && !Params->FoundLeafStatus);
            if(SearchToggle)
            {
                // NOTE(chowie): Early out asserts
                switch(TreeGroup->Params.Type)
                {
                    case FinkHashTree_Type_ReadWriteAdd:
                    {
                        b32x Check = (StackTop(Frontier)->Entry == 0);
                        if(!Check)
                        {
                            // TODO(chowie): Message log here!
                            // printf("Invalid Add\n");
                            break;
                        }
                    } break;

                    case FinkHashTree_Type_ReadWriteDelete:
                    {
                        b32x Check = (StackTop(Frontier)->Entry != 0);
                        if(!Check)
                        {
                            // TODO(chowie): Message log here!
                            // printf("Invalid Delete\n");
                            break;
                        }
                    } break;

                    default:
                    {
                        break;
                    }
                }

                // NOTE(chowie): Because DFS leans left, double leaf should be .y pair!
                // NOTE(chowie): Hack to clean up/recombines left tree, specific to 2-3 depth trees.
                if(LeavesInARow > 0)
                {
                    if(StackTop(Fabric)->Entry == FinkHashCCSwap)
                    {
                        Fabric = Pop(Fabric);
                    }
                    Fabric = Pop(Fabric);

                    // COULDDO(chowie): For loop?
                    if(LeavesInARow == 1)
                    {
                        Fabric = Push(Fabric, {(u64)Tree->Leaves[Tree->LeafCount - 1]});
                    }
                    else if(LeavesInARow == 2)
                    {
                        // NOTE(chowie): The next leaf/node must be above (Depth - 1)
                        u64 CombineParentAgain = FinkHash({(u64)Tree->Leaves[Tree->LeafCount - 2],
                                                           (u64)Tree->Leaves[Tree->LeafCount - 1]});
                        Fabric = Push(Fabric, {CombineParentAgain});
                    }
                    else if(LeavesInARow == 3)
                    {
                        u64 CombineParentAgainInternal = FinkHash({(u64)Tree->Leaves[Tree->LeafCount - 3],
                                                                   (u64)Tree->Leaves[Tree->LeafCount - 2]});
                        u64 CombineParentAgain = FinkHash({CombineParentAgainInternal,
                                                           (u64)Tree->Leaves[Tree->LeafCount - 1]});
                        Fabric = Push(Fabric, {CombineParentAgain});
                    }

                    Fabric = Push(Fabric, {FinkHashCCSwap});
                }
                else if((PrevLeafDepth + 1) == Depth) // NOTE(chowie): Handles LeafFollowedByParent edge case
                {
                    Assert(StackTop(Fabric)->Entry != FinkHashCCSwap);
                    u64 A = StackTop(Fabric)->Entry;
                    Fabric = Pop(Fabric);
                    Fabric = Pop(Fabric);

                    Fabric = Push(Fabric, {(u64)Tree->Leaves[Tree->LeafCount - 1]});
                    Fabric = Push(Fabric, {FinkHashCCSwap});
                    Fabric = Push(Fabric, {A});
                }

                // RESOURCE(): https://hero.handmade.network/forums/code-discussion/t/1750-function_for_drawing_polygons
                // NOTE(chowie): Sean Barret trick to start with exception case before normal
                u64 Acc = Params->Data.DestLeafValue;
                while(!StackIsEmpty(Fabric))
                {
                    u64 A = Acc;
                    u64 B = StackTop(Fabric)->Entry;

                    if(StackTop(Fabric)->Entry == FinkHashCCSwap)
                    {
                        Fabric = Pop(Fabric);
                        B = StackTop(Fabric)->Entry;

                        Swap(u64, A, B); // NOTE(chowie): Normal becomes Exception
                    }

                    u64 NewTreeFinkHash = FinkHash({A, B});
                    Fabric = Pop(Fabric);
                    Acc = NewTreeFinkHash;
                }

                Tree->FinkHash = Acc;
                Params->FoundLeaf = (Params->Mode == FinkHashTree_Mode_Index) ? (u16)StackTop(Frontier)->Entry : (u16)Tree->LeafCount;
                Params->FoundLeafStatus = true;
                if(!Params->CompareTreeLeafChange)
                {
                    break;
                }
            }

            // TODO(chowie): What do I do about add/delete since leaves don't actually save properly?
            // Search again or what do I do?
            Tree->Leaves[Tree->LeafCount++] = (u16)StackTop(Frontier)->Entry;
            Frontier = Pop(Frontier);

            PrevLeafDepth = Depth;
            // NOTE(chowie): Cleans up/recombines leaf pairs.
            if(LeavesInARow == 1)
            {
                --Depth;
                // NOTE(chowie): Never Assert(StackTop(Fabric)->Entry != FinkHashCCSwap);
                // I want to keep the loop running to find all of the materials! It shows a 3
                // because at the top "if(StackLength(Frontier) == 1)" adds a CC swap, but
                // would never actually run!
                if(!Params->FoundLeafStatus)
                {
                    Fabric = Pop(Fabric);
                    Fabric = Pop(Fabric);

                    u64 CombineParentAgain = FinkHash({(u64)Tree->Leaves[Tree->LeafCount - 2],
                                                       (u64)Tree->Leaves[Tree->LeafCount - 1]});
                    Fabric = Push(Fabric, {CombineParentAgain});
                    Fabric = Push(Fabric, {FinkHashCCSwap});
                }
            }

            ++LeavesInARow;
        }
    }

    if(Params->FoundLeafStatus)
    {
        if(Params->CompareTreeLeafChange)
        {
            Tree->Leaves[Params->FoundLeaf] = Params->Data.DestLeafValue;
        }
    }
    else
    {
        // TODO(chowie): Message log here!
        // printf("Couldn't find the searched leaf!\n");
    }
}

// NOTE(chowie): Optimisation of the above
// NOTE(chowie): By default you replace a leaf. Add/delete is just a replace that can be toggled.
internal void
FinkHashTreeReadWritePerfectTree(finkhashtree_group *TreeGroup)
{
    finkhashtree *Tree = &TreeGroup->Tree;
    finkhashtree_params *Params = &TreeGroup->Params;

    // RESOURCE(): https://legendsoflocalization.com/articles/video-game-control-codes/
    // NOTE(chowie): CC (Control Code) or Leaf/Parent = 30000, is an unused hash number
    // to swap two leaf IDs. I'm using this technique to push/string multiple swaps
    // into the fabric. The fabric when processed should 'eat' all 30000s to swap!
#define FinkHashCCSwap 30000

    // NOTE(chowie): Pairs starts as 1
    u32 LeavesInARow = 0;

    // NOTE(chowie): Only thing that needs to be stored (when in left or right subtree)
    stack *Fabric = {};
    v2u64 TopSubTreePair = InvertFinkHash(Tree->FinkHash); // TODO(chowie): Simplify?

    stack *Frontier = {};
    Frontier = Push(Frontier, {Tree->FinkHash});
    while(!StackIsEmpty(Frontier))
    {
        // NOTE(chowie): Must be checked for all nodes since the top-most
        // subtree could be a parent or _leaf_
        // NOTE(chowie): Guards the possibility that TopSubTreePair.y
        // could be a duplicate leaf, but not the right one
        if(StackTop(Frontier)->Entry == TopSubTreePair.y)
        {
            if(StackLength(Frontier) == 1) // NOTE(chowie): Swaps to right subtree once, technically this also triggers at the end
            {
                if(!Params->FoundLeafStatus)
                {
                    ZeroStruct(Fabric); // NOTE(chowie): Flush left subtree fabric
                    Fabric = Push(Fabric, {TopSubTreePair.x});
                    Fabric = Push(Fabric, {FinkHashCCSwap});
                }
            }
        }

        if(IsParent(StackTop(Frontier)->Entry))
        {
            v2u64 Pair = InvertFinkHash(StackTop(Frontier)->Entry);

            Frontier = Pop(Frontier);
            Frontier = Push(Frontier, {Pair.y}); // 115
            Frontier = Push(Frontier, {Pair.x}); // 119

            if(!Params->FoundLeafStatus)
            {
                Fabric = Push(Fabric, {Pair.y});
            }

            LeavesInARow = 0;
        }
        else
        {
            b32x SearchToggle = (Params->Mode == FinkHashTree_Mode_Index) ?
                                (Tree->LeafCount == Params->Data.SourceLeafIndex) :
                                ((StackTop(Frontier)->Entry == Params->Data.SourceLeafValue) && !Params->FoundLeafStatus);
            if(SearchToggle)
            {
                // NOTE(chowie): Early out asserts
                switch(TreeGroup->Params.Type)
                {
                    case FinkHashTree_Type_ReadWriteAdd:
                    {
                        b32x Check = (StackTop(Frontier)->Entry == 0);
                        if(!Check)
                        {
                            // TODO(chowie): Message log here!
                            // printf("Invalid Add\n");
                            break;
                        }
                    } break;

                    case FinkHashTree_Type_ReadWriteDelete:
                    {
                        b32x Check = (StackTop(Frontier)->Entry != 0);
                        if(!Check)
                        {
                            // TODO(chowie): Message log here!
                            // printf("Invalid Delete\n");
                            break;
                        }
                    } break;

                    default:
                    {
                        break;
                    }
                }

                // NOTE(chowie): Because DFS leans left, double leaf should be .y pair!
                // NOTE(chowie): Hack to clean up/recombines left tree, specific to 2-3 depth trees.
                if(LeavesInARow == 1)
                {
                    // TODO(chowie): Could this check be removed?
                    if(StackTop(Fabric)->Entry == FinkHashCCSwap)
                    {
                        Fabric = Pop(Fabric);
                    }
                    Fabric = Pop(Fabric);

                    Fabric = Push(Fabric, {(u64)Tree->Leaves[Tree->LeafCount - 1]});
                    Fabric = Push(Fabric, {FinkHashCCSwap});
                }

                // RESOURCE(): https://hero.handmade.network/forums/code-discussion/t/1750-function_for_drawing_polygons
                // NOTE(chowie): Sean Barret trick to start with exception case before normal
                u64 Acc = Params->Data.DestLeafValue;
                while(!StackIsEmpty(Fabric))
                {
                    u64 A = Acc;
                    u64 B = StackTop(Fabric)->Entry;

                    if(StackTop(Fabric)->Entry == FinkHashCCSwap)
                    {
                        Fabric = Pop(Fabric);
                        B = StackTop(Fabric)->Entry;

                        Swap(u64, A, B); // NOTE(chowie): Normal becomes Exception
                    }

                    u64 NewTreeFinkHash = FinkHash({A, B});
                    Fabric = Pop(Fabric);
                    Acc = NewTreeFinkHash;
                }

                Tree->FinkHash = Acc;
                Params->FoundLeaf = (Params->Mode == FinkHashTree_Mode_Index) ? (u16)StackTop(Frontier)->Entry : (u16)Tree->LeafCount;
                Params->FoundLeafStatus = true;
                if(!Params->CompareTreeLeafChange)
                {
                    break;
                }
            }

            Tree->Leaves[Tree->LeafCount++] = (u16)StackTop(Frontier)->Entry;
            Frontier = Pop(Frontier);

            // NOTE(chowie): Cleans up/recombines leaf pairs.
            if(LeavesInARow == 1)
            {
                // NOTE(chowie): Never Assert(StackTop(Fabric)->Entry != FinkHashCCSwap);
                // I want to keep the loop running to find all of the materials! It shows a 3
                // because at the top "if(StackLength(Frontier) == 1)" adds a CC swap, but
                // would never actually run!
                if(!Params->FoundLeafStatus)
                {
                    Fabric = Pop(Fabric);
                    Fabric = Pop(Fabric);

                    u64 CombineParentAgain = FinkHash({(u64)Tree->Leaves[Tree->LeafCount - 2],
                                                       (u64)Tree->Leaves[Tree->LeafCount - 1]});
                    Fabric = Push(Fabric, {CombineParentAgain});
                    Fabric = Push(Fabric, {FinkHashCCSwap});
                }
            }

            ++LeavesInARow;
        }
    }

    if(Params->FoundLeafStatus)
    {
        if(Params->CompareTreeLeafChange)
        {
            Tree->Leaves[Params->FoundLeaf] = Params->Data.DestLeafValue;
        }
    }
    else
    {
        // TODO(chowie): Message log here!
        // printf("Couldn't find the searched leaf!\n");
    }
}

internal u32
GetDepthFinkHashPerfectTree(u64 Hash)
{
    u32 Iter = 0;
    u64 Acc = Hash;
    for(;
        IsParent(Acc);
        )
    {
        ++Iter;
        v2u64 Pair = InvertFinkHash(Acc);
        Acc = Pair.x;
    }
    return(Iter);
}

//
// NOTE(chowie): First 38 (including 0) can be used for eighth blocks
//
// IMPORTANT(chowie): Primitives (child nodes) must be even values!
#define EvenSet(EnumValue) ((2*EnumValue))
enum block_id : u16
{
    Block_Air    = EvenSet(0),
    Block_Water  = EvenSet(1),

    // ----------- DO NOT REORDER THE ABOVE (USED TO PRECOMPUTATE VALUES) ---------

    Block_Stone  = EvenSet(2),
    Block_Leaves = EvenSet(3),

    //
    //
    //
};

// COULDDO(chowie): Could take use .destleaf to save on a for loop?
internal u64
FinkHashTreeBubbleupCommonLeafInPerfectTreeIfPossibleInternal(u64 Hash)
{
    u64 Result = Hash;
    // NOTE(chowie): The most-likely block ID to be filled in the tree
#define PRECOMPUTED_BLOCKWATERHASH 27071209 // NOTE(chowie): f(1) or 2
    if(Result == PRECOMPUTED_BLOCKWATERHASH)
    {
        Result = (u64)Block_Water;
    }
    else
    {
        for(;
            IsParent(Result);
            )
        {
            v2u64 Pair = InvertFinkHash(Result);
            Result = Pair.x;
        }
    }

    return(Result);
}

internal u64
FinkHashTreeBubbleupCommonLeafInPerfectTreeIfPossible(u64 Hash)
{
    u64 Result = Hash;
    if(IsPerfectSqr(Result))
    {
        u64 CommonLeaf = FinkHashTreeBubbleupCommonLeafInPerfectTreeIfPossibleInternal(Result);
        // NOTE(chowie): For water/sand/mud with a "moving = still/calm" state while keeping
        // the eighth blocks intact (no bubble up)
        if(CommonLeaf == (u64)Block_Water)
        {
            // NOTE(chowie): Do not recombine for particle-related/fluid sim stuff
        }
        else
        {
            // NOTE(chowie): E.g. Breaking the last block, recombines all air into an air block
            Result = CommonLeaf;
        }
    }

    return(Result);
}

// e.g.  x  ->   x
//              / \
//             y   y
//            / \ / \
//            2 2 2 2
enum finkhash_expandmode
{
    FinkHash_ExpandMode_2Depth,
    FinkHash_ExpandMode_3Depth,
};

internal u64
FinkHashExpandLeafToPerfectTree(u64 Hash, finkhash_expandmode ExpandMode = FinkHash_ExpandMode_3Depth)
{
    u64 Result = 0;
    switch(ExpandMode)
    {
        // NOTE(chowie): 3 depth is the most common case, so it's here (above 2 depth)
        case FinkHash_ExpandMode_3Depth:
        {
#define PERFECTTREEPRIM_MAX3DEPTH 74
            Assert(Hash <= PERFECTTREEPRIM_MAX3DEPTH);

            u64 FinkHashMaxBot = FinkHash({Hash, Hash});
            u64 FinkHashMaxMid = FinkHash({FinkHashMaxBot, FinkHashMaxBot});
            Result = FinkHash({FinkHashMaxMid, FinkHashMaxMid});
        } break;

        case FinkHash_ExpandMode_2Depth:
        {
#define PERFECTTREEPRIM_MAX2DEPTH 16382
            Assert(Hash <= PERFECTTREEPRIM_MAX2DEPTH);

            u64 FinkHashMaxBot = FinkHash({Hash, Hash});
            Result = FinkHash({FinkHashMaxBot, FinkHashMaxBot});
        } break;

        InvalidDefaultCase;
    }

    return(Result);
}

//
// Fink hash integrity
//

// TODO(chowie): Merkle trees are good for validating a node without
// the whole tree! Check sum with the root. According to John D Cook
inline b32x
VerifyFinkHashTreeIntegrity(u64 Source, u64 Copy)
{
    b32x Result = (Source == Copy);
    return(Result);
}

// NOTE(chowie): Don't need to actually repair hash tree since everything is baked into hash
inline u64
RepairSourceFinkHashTree(u64 Source, u64 Copy)
{
    if(!VerifyFinkHashTreeIntegrity(Source, Copy))
    {
        Source = Copy;
    }
    else
    {
        // TODO(chowie): Message to user to say all is fine!
    }

    u64 Result = Source;
    return(Result);
}

//
// Fink hash unit tests
//

enum testcase_finkhash_max
{
    BalancedDepth1Max      = BitSet(0),
    BalancedDepth2Max      = BitSet(1),
    BalancedDepth3Max      = BitSet(2),
    BalancedDepth4Max      = BitSet(3),

    LeftSkewedDepth3Max    = BitSet(4),
    RightSkewedDepth3Max   = BitSet(5),
    BotLeftHeavyDepth3Max  = BitSet(6),
    BotRightHeavyDepth3Max = BitSet(7),
    MidLeftHeavyDepth3Max  = BitSet(8),
    MidRightHeavyDepth3Max = BitSet(9),

    BalancedDepth3Stairs   = BitSet(10),
};

internal u64
TestFinkHashMax(u32 Flag)
{
    u64 MaxPrim = 0;
    u64 TestFinkHashMaxInternal = 0;
    u64 AddTestFinkHashMax_ = 0;
    u64 AddTestFinkHashMax = 0;
    u64 TestFinkHashMax = 0;
    u64 TestFinkHashMaxAirInternal = 0;

    // NOTE(chowie): Copied from above "Order matters! To get the largest fink hash result,
    // f(x, y): x = Largest, y = Smallest (see SAMPLE PASSED FUNCTIONS above).
    // This is because x shows up more than y "f(x,y) = 1 + 2(x + (x+y)(x+y+1)/2)" "
    // Thus, right-leaning trees has more max prims + I don't want to balance trees

    //
    // For depth 1 (height 2)
    //

    // NOTE(chowie): Definitely can go a little higher, I'm just stopping here...
    // Tree max = 1152921506754330625
    // u64 max  = 18446744073709551615
    if(FlagSet(Flag, BalancedDepth1Max))
    {
#define MAX_BALANCEDTREE1_PRIMS 536870912 // Total Prims: 536870912/2 = 26843546
        MaxPrim = 536870912;
        TestFinkHashMax = FinkHash({MaxPrim, MaxPrim}); // 2^29 (can go a little higher)
        // printf("BalancedDepth1Max: %llu\n", TestFinkHashMax);
    }
    // NOTE(chowie): Precomputed values
    // - Max all 2's (tree filled with empty air) = 25
    //   if (Value > 25), we know the tree must contain a block
    // - Max all 4's (tree filled with solid)     = 81

    //
    // For depth 2 (height 3)
    //

    // TODO(chowie): Assert with max 2x2^2x2 prims e.g. Assert(Pair.x <= 16382); Assert(Pair.y <= 16382);
    // NOTE(chowie): Tree max (for primitives) 16382x16382^16382x16382 = 4609997404775383401.
    // Tree max = 4609997404775383401
    //            18224914292213428809
    // u64 max  = 18446744073709551615
    //           root
    //       /         \
    //    ?????       ?????
    //    /   \       /   \
    // 16382 16382 16382 16382 -- depth 2 (height 3) --
    if(FlagSet(Flag, BalancedDepth2Max))
    {
#define MAX_BALANCEDTREE2_PRIMS 23168 // Total Prims: 23168/2 + 1 (to include 0) = 11585 or 2^13
        MaxPrim = MAX_BALANCEDTREE2_PRIMS;
        TestFinkHashMaxInternal = FinkHash({MaxPrim, MaxPrim});
        TestFinkHashMax = FinkHash({TestFinkHashMaxInternal, TestFinkHashMaxInternal});
        // printf("BalancedDepth2Max: %llu\n", TestFinkHashMax);
    }
    // NOTE(chowie): Precomputed values
    // - Max all 2's (tree filled with empty air) = 2601
    //   if (Value > 2601), we know the tree must contain a block
    // - Max all 4's (tree filled with solid)     = 26569

    //
    // For depth 3 (height 4)
    //

    // NOTE(chowie): Tree max (for primitives) 88x88 ^88 ^88 = 974409950480300793.
    // Tree max = 974409950480300793
    // u64 max  = 18446744073709551615
    //         root
    //         /  \
    //      ????  88
    //      /  \
    //   ????  88
    //   /  \
    //  88  88 -- depth 3 (height 4) --
    if(FlagSet(Flag, LeftSkewedDepth3Max))
    {
#define MAX_UNBALANCEDTREE3_PRIMS 88 // Total Prims: 88/2 = 44
        MaxPrim = 88;
        TestFinkHashMaxInternal = FinkHash({MaxPrim, MaxPrim});
        AddTestFinkHashMax = FinkHash({TestFinkHashMaxInternal, MaxPrim});
        TestFinkHashMax = FinkHash({AddTestFinkHashMax, MaxPrim});
        // printf("LeftSkewedDepth3Max: %llu\n", TestFinkHashMax);
    }

    // NOTE(chowie): Tree max (for primitives) ^106 ^106 106x106 = 2690819653717842813.
    // Tree max = 4276712591510795193
    // u64 max  = 18446744073709551615
    //    root
    //    /  \
    //  106  ????
    //       /  \
    //     106  ????
    //          /  \
    //        106  106 -- depth 3 (height 4) --
    if(FlagSet(Flag, RightSkewedDepth3Max))
    {
#define INVMAX_UNBALANCEDTREE3_PRIMS 106 // Total Prims: 106/2 = 53
        MaxPrim = 106;
        TestFinkHashMaxInternal = FinkHash({MaxPrim, MaxPrim});
        AddTestFinkHashMax = FinkHash({MaxPrim, TestFinkHashMaxInternal});
        TestFinkHashMax = FinkHash({MaxPrim, AddTestFinkHashMax});
        // printf("RightSkewedDepth3Max: %llu\n", TestFinkHashMax);
    }

    // NOTE(chowie): Tree max (for primitives) 60x60^60x60 ^60 = 735296218141716529.
    // Tree max = 1985778663213204553
    // u64 max  = 18446744073709551615
    //         root
    //         /  \
    //      ????  60
    //      /   \
    //   ????   ????
    //   /  \   /  \
    //  60  60 60  60 -- depth 3 (height 4) --
    if(FlagSet(Flag, BotLeftHeavyDepth3Max))
    {
#define MAX_TOPUNBALANCEDTREE3_PRIMS 68 // Total Prims: 68/2 + 1 (to include 0) = 35
        MaxPrim = 68;
        TestFinkHashMaxInternal = FinkHash({MaxPrim, MaxPrim});
        AddTestFinkHashMax = FinkHash({TestFinkHashMaxInternal, TestFinkHashMaxInternal});
        TestFinkHashMax = FinkHash({AddTestFinkHashMax, MaxPrim});
        // printf("BotLeftHeavyDepth3Max: %llu\n", TestFinkHashMax);
    }

    // NOTE(chowie): Tree max (for primitives) ^74 74x74^74x74 = 3887310990438575921.
    // Tree max = 3887310990438575921
    // u64 max  = 18446744073709551615
    //    root
    //    /  \
    //   74  ????
    //       /  \
    //   ????   ????
    //   /  \   /  \
    //  74  74 74  74 -- depth 3 (height 4) --
    if(FlagSet(Flag, BotRightHeavyDepth3Max))
    {
#define INVMAX_TOPUNBALANCEDTREE3_PRIMS 74 // Total Prims: 74/2 = 37
        MaxPrim = 74;
        TestFinkHashMaxInternal = FinkHash({MaxPrim, MaxPrim});
        AddTestFinkHashMax = FinkHash({TestFinkHashMaxInternal, TestFinkHashMaxInternal});
        TestFinkHashMax = FinkHash({MaxPrim, AddTestFinkHashMax});
        // printf("BotRightHeavyDepth3Max: %llu\n", TestFinkHashMax);
    }

    // NOTE(chowie): Tree balanced (for primitives) 70x70^70 ^70x70 = 3917225850163947171.
    // Tree max = 3917225850163947171
    // u64 max  = 18446744073709551615
    //           root
    //         /      \
    //      ????      ????
    //      /  \      /  \
    //   ????  70  ????   ????
    //   /  \      /  \   /  \
    //  70  70    70  70 70  70 -- depth 3 (height 4) --
    if(FlagSet(Flag, MidLeftHeavyDepth3Max))
    {
#define MAX_MIDUNBALANCEDTREE3_PRIMS 70 // Total Prims: 70/2 = 35
        MaxPrim = 70;
        TestFinkHashMaxInternal = FinkHash({MaxPrim, MaxPrim});
        AddTestFinkHashMax_ = FinkHash({TestFinkHashMaxInternal, MaxPrim}); // NOTE(chowie): Doesn't matter if you swap xy order, max is still the same!
        AddTestFinkHashMax  = FinkHash({TestFinkHashMaxInternal, TestFinkHashMaxInternal});
        TestFinkHashMax = FinkHash({AddTestFinkHashMax_, AddTestFinkHashMax}); // NOTE(chowie): Doesn't matter if you swap xy order, max is still the same!
        // printf("MidLeftHeavyDepth3Max: %llu\n", TestFinkHashMax);
    }

    // NOTE(chowie): Tree balanced (for primitives) ^70x70 ^70 70x70 = 3917069012144913893.
    // Tree max = 16675805043663149251
    // u64 max  = 18446744073709551615
    //            root
    //         /       \
    //      ????       ????
    //      /  \       /  \
    //   ????   ????  70  ????
    //   /  \   /  \      /  \
    //  70  70 70  70   70   70 -- depth 3 (height 4) --
    if(FlagSet(Flag, MidRightHeavyDepth3Max))
    {
#define INVMAX_MIDUNBALANCEDTREE3_PRIMS 84 // Total Prims: 84/2 + 1 (to include 0) = 43
        MaxPrim = INVMAX_MIDUNBALANCEDTREE3_PRIMS;
        TestFinkHashMaxInternal = FinkHash({MaxPrim, MaxPrim});
        AddTestFinkHashMax_ = FinkHash({MaxPrim, TestFinkHashMaxInternal});  // NOTE(chowie): Doesn't matter if you swap xy order, max is still the same!
        AddTestFinkHashMax  = FinkHash({TestFinkHashMaxInternal, TestFinkHashMaxInternal});
        TestFinkHashMax = FinkHash({AddTestFinkHashMax, AddTestFinkHashMax_}); // NOTE(chowie): Doesn't matter if you swap xy order, max is still the same!
        // printf("MidRightHeavyDepth3Max: %llu\n", TestFinkHashMax);
    }

    // NOTE(chowie): Tree balanced (for primitives) ^62x62 ^62x62 ^62x62 ^62x62 = 3917069012144913893.
    // Tree max = 15549242794551446761
    // u64 max  = 18446744073709551615
    //              root
    //         /            \
    //      ????            ????
    //      /  \            /  \
    //   ????   ????     ????   ????
    //   /  \   /  \     /  \   /  \
    //  74  74 74  74   74  74 74  74 -- depth 3 (height 4) --
    if(FlagSet(Flag, BalancedDepth3Max))
    {
#define MAX_BALANCEDTREE3_PRIMS 74 // Total Prims: 74/2 + 1 (to include 0) = 38
        MaxPrim = MAX_BALANCEDTREE3_PRIMS;
        TestFinkHashMaxInternal = FinkHash({MaxPrim, MaxPrim});
        AddTestFinkHashMax_ = FinkHash({TestFinkHashMaxInternal, TestFinkHashMaxInternal});
        AddTestFinkHashMax  = FinkHash({TestFinkHashMaxInternal, TestFinkHashMaxInternal});
        TestFinkHashMax = FinkHash({AddTestFinkHashMax, AddTestFinkHashMax_}); // IMPORTANT(chowie): Doesn't matter if you swap xy order, max is still the same!
        // printf("BalancedDepth3Max: %llu\n", TestFinkHashMax);
    }
    // NOTE(chowie): Precomputed values
    // - Max all 2's (tree filled with empty air) = 27071209
    //   if (Value > 27071209), we know the tree must contain a block
    // - Max all 4's (tree filled with solid)     = 2823753321

    // Tree max = 16172331688244510289
    // u64 max  = 18446744073709551615
    if(FlagSet(Flag, BalancedDepth3Stairs))
    {
#define MAX_BALANCEDTREE3STAIRS_PRIMS 94 // Total Prims: 94/2 + 1 (to include 0) = 48
        MaxPrim = MAX_BALANCEDTREE3STAIRS_PRIMS;
        TestFinkHashMaxInternal = FinkHash({MaxPrim, MaxPrim});
        TestFinkHashMaxAirInternal = FinkHash({MaxPrim, Block_Air});
        AddTestFinkHashMax  = FinkHash({TestFinkHashMaxInternal, TestFinkHashMaxAirInternal});
        TestFinkHashMax = FinkHash({AddTestFinkHashMax, AddTestFinkHashMax}); // IMPORTANT(chowie): Doesn't matter if you swap xy order, max is still the same!
        // printf("BalancedDepth3Max: %llu\n", TestFinkHashMax);
    }

    // NOTE(chowie): However, 4 is too large to unpack, so it must be 2
    // Tree max = 13447587209019577833
    // u64 max  = 18446744073709551615
    //                  root
    //          /               \
    //       ?????             ?????
    //       /   \             /   \
    //    ????    ????      ????    ????
    //    /  \    /  \      /  \    /  \
    //   ??  ??  ??  ??    ??  ??  ??  ??
    //   /\  /\  /\  /\    /\  /\  /\  /\
    //  4 4 4 4 4 4 4 4   4 4 4 4 4 4 4 4 -- depth 4 (height 5) --
    if(FlagSet(Flag, BalancedDepth4Max))
    {
#define MAX_BALANCEDTREE4_PRIMS 2 // Total Prims: 2/2 + 1 (to include 0) = 2
        MaxPrim = MAX_BALANCEDTREE4_PRIMS;
        TestFinkHashMaxInternal = FinkHash({MaxPrim, MaxPrim});
        AddTestFinkHashMax_     = FinkHash({TestFinkHashMaxInternal, TestFinkHashMaxInternal});
        AddTestFinkHashMax      = FinkHash({AddTestFinkHashMax_, AddTestFinkHashMax_});
        TestFinkHashMax = FinkHash({AddTestFinkHashMax, AddTestFinkHashMax});
        // printf("BalancedDepth4Max: %llu\n", TestFinkHashMax);
    }

    return(TestFinkHashMax);
}

struct testreplacedepth_info
{
    u16 HashL1a;
    u16 HashL1b;
    u16 HashR1a;
    u16 HashR1b;

    u16 HashL2a;
    u16 HashL2b;
    u16 HashR2a;
    u16 HashR2b;

    u64 CheckFinkHashFinal;
};

internal testreplacedepth_info
InitTestFinkHashReplaceDepth2(u16 HashL1a, u16 HashL1b, u16 HashR1a, u16 HashR1b)
{
    testreplacedepth_info Result = {};
    Result.HashL1a = HashL1a;
    Result.HashL1b = HashL1b;
    Result.HashR1a = HashR1a;
    Result.HashR1b = HashR1b;
    return(Result);
}

internal testreplacedepth_info
InitTestFinkHashReplaceDepth3(u16 HashL1a, u16 HashL1b, u16 HashR1a, u16 HashR1b,
                              u16 HashL2a, u16 HashL2b, u16 HashR2a, u16 HashR2b)
{
    testreplacedepth_info Result = {};
    Result.HashL1a = HashL1a;
    Result.HashL1b = HashL1b;
    Result.HashR1a = HashR1a;
    Result.HashR1b = HashR1b;

    Result.HashL2a = HashL2a;
    Result.HashL2b = HashL2b;
    Result.HashR2a = HashR2a;
    Result.HashR2b = HashR2b;

    return(Result);
}

internal void
DEBUGAddReplaceFinkHashDepth2(testreplacedepth_info *Info)
{
    u64 CheckFinkHashL = FinkHash({Info->HashL1a, Info->HashL1b});
    u64 CheckFinkHashR = FinkHash({Info->HashR1a, Info->HashR1b});
    Info->CheckFinkHashFinal = FinkHash({CheckFinkHashL, CheckFinkHashR});

    // printf("CheckFinkHashR %llu\n", CheckFinkHashR);
    // printf("Validate New Fink Hash Depth 2 L{%llu, %llu} R{%llu, %llu}: %llu\n",
    //       Info->HashL1a, Info->HashL1b, Info->HashR1a, Info->HashR1b, Info->CheckFinkHashFinal);
}

internal void
DEBUGAddReplaceFinkHashLSkewedDepth2(testreplacedepth_info *Info)
{
    u64 CheckFinkHashL = FinkHash({Info->HashL1a, Info->HashL1b});
    Info->CheckFinkHashFinal = FinkHash({CheckFinkHashL, Info->HashR1a});
    // printf("Validate New Fink Hash Skewed Depth 2 L{%llu, %llu} R{%llu}: %llu\n",
    //       Info->HashL1a, Info->HashL1b, Info->HashR1a, Info->CheckFinkHashFinal);
}

internal void
DEBUGAddReplaceFinkHashRSkewedDepth2(testreplacedepth_info *Info)
{
    u64 CheckFinkHashR = FinkHash({Info->HashR1a, Info->HashR1b});
    Info->CheckFinkHashFinal = FinkHash({Info->HashL1a, CheckFinkHashR});
    // printf("Validate New Fink Hash Skewed Depth 2 L{%llu} R{%llu, %llu}: %llu\n",
    //       Info->HashL1a, Info->HashR1a, Info->HashR1b, Info->CheckFinkHashFinal);
}

internal void
DEBUGAddReplaceFinkHashDepth3(testreplacedepth_info *Info)
{
    u64 CheckFinkHashL1 = FinkHash({Info->HashL1a, Info->HashL1b});
    u64 CheckFinkHashR1 = FinkHash({Info->HashR1a, Info->HashR1b});
    u64 CheckFinkHashL2 = FinkHash({Info->HashL2a, Info->HashL2b});
    u64 CheckFinkHashR2 = FinkHash({Info->HashR2a, Info->HashR2b});

    u64 CombineHashL = FinkHash({CheckFinkHashL1, CheckFinkHashR1});
    u64 CombineHashR = FinkHash({CheckFinkHashL2, CheckFinkHashR2});
    Info->CheckFinkHashFinal = FinkHash({CombineHashL, CombineHashR});
    // IMPORTANT(chowie): Doesn't matter if you swap xy order, max is still the same
    // if you replace a node in the middle!

    // printf("CombineHashR %llu\n", CombineHashR);
    // printf("Validate New Fink Hash Depth 3 L1{%llu, %llu} R1{%llu, %llu} L2{%llu, %llu} R2{%llu, %llu}: %llu\n",
    //       Info->HashL1a, Info->HashL1b, Info->HashR1a, Info->HashR1b,
    //       Info->HashL2a, Info->HashL2b, Info->HashR2a, Info->HashR2b, Info->CheckFinkHashFinal);
}

//      ???
//      / \
//    ???  8
//    / \
//  ???  2
//  / \
// 4   6
//
internal void
DEBUGAddReplaceFinkHashLSkewedDepth3(testreplacedepth_info *Info)
{
    u64 CheckFinkHashL_ = FinkHash({Info->HashL1a, Info->HashL1b});
    u64 CheckFinkHashL = FinkHash({CheckFinkHashL_, Info->HashR1a});
    Info->CheckFinkHashFinal = FinkHash({CheckFinkHashL, Info->HashL2a});
    // printf("Validate New Fink Hash L1 Skewed Depth 3 L1{%llu, %llu} R1{%llu} L2{%llu}: %llu\n",
    //       Info->HashL1a, Info->HashL1b, Info->HashR1a, Info->HashL2a, Info->CheckFinkHashFinal);
}

internal void
DEBUGAddReplaceFinkHashR1SkewedDepth3(testreplacedepth_info *Info)
{
    u64 CheckFinkHashL_ = FinkHash({Info->HashR1a, Info->HashR1b});
    u64 CheckFinkHashL = FinkHash({Info->HashL1a, CheckFinkHashL_});
    Info->CheckFinkHashFinal = FinkHash({CheckFinkHashL, Info->HashL2a});
    // printf("Validate New Fink Hash L2 Skewed Depth 3 L1{%llu} R1{%llu, %llu} L2{%llu}: %llu\n",
    //       Info->HashL1a, Info->HashR1a, Info->HashR1b, Info->HashL2a, Info->CheckFinkHashFinal);
}

internal void
DEBUGEndReplaceFinkHashDepth(testreplacedepth_info *Info, testreplacedepth_info *ResetInfo)
{
//    Assert();
    *Info = *ResetInfo;
}

// TODO(chowie): Test case for Depth1?
enum testcase_finkhash
{
    FinkHash_BalancedDepth2,
    FinkHash_LSkewedDepth2,
    FinkHash_RSkewedDepth2,

    FinkHash_BalancedDepth3,
    FinkHash_LSkewedDepth3,
    FinkHash_LSkewedDepth3Duplicate,
    FinkHash_R1SkewedDepth3, // NOTE(chowie): Tests Leaf followed by Parent
};

// NOTE(chowie): Starting test cases
internal u64
TestFinkHash(testcase_finkhash Type)
{
    u64 Result = 0;
    switch(Type)
    {
        case FinkHash_BalancedDepth2:
        {
            testreplacedepth_info TestDepth2_ = InitTestFinkHashReplaceDepth2(4, 6, 2, 8);
            testreplacedepth_info *TestDepth2 = &TestDepth2_;
            DEBUGAddReplaceFinkHashDepth2(TestDepth2);
            Result = TestDepth2->CheckFinkHashFinal;
        } break;

        case FinkHash_LSkewedDepth2:
        {
            testreplacedepth_info TestDepth2_ = InitTestFinkHashReplaceDepth2(4, 6, 2, 0);
            testreplacedepth_info *TestDepth2 = &TestDepth2_;
            DEBUGAddReplaceFinkHashLSkewedDepth2(TestDepth2);
            Result = TestDepth2->CheckFinkHashFinal;
        } break;

        case FinkHash_RSkewedDepth2:
        {
            testreplacedepth_info TestDepth2_ = InitTestFinkHashReplaceDepth2(4, 0, 2, 8);
            testreplacedepth_info *TestDepth2 = &TestDepth2_;
            DEBUGAddReplaceFinkHashRSkewedDepth2(TestDepth2);
            Result = TestDepth2->CheckFinkHashFinal;
        } break;

        case FinkHash_BalancedDepth3:
        {
            testreplacedepth_info TestDepth3_ = InitTestFinkHashReplaceDepth3(4, 6, 2, 8,
                                                                              12, 14, 10, 16);
            testreplacedepth_info *TestDepth3 = &TestDepth3_;
            DEBUGAddReplaceFinkHashDepth3(TestDepth3);
            Result = TestDepth3->CheckFinkHashFinal;
        } break;

        case FinkHash_LSkewedDepth3:
        {
            testreplacedepth_info TestDepth3_ = InitTestFinkHashReplaceDepth3(4, 6, 2, 8,
                                                                              12, 14, 10, 16);
            testreplacedepth_info *TestDepth3 = &TestDepth3_;
            DEBUGAddReplaceFinkHashLSkewedDepth3(TestDepth3);
            Result = TestDepth3->CheckFinkHashFinal;
        } break;

        case FinkHash_LSkewedDepth3Duplicate:
        {
            testreplacedepth_info TestDepth3_ = InitTestFinkHashReplaceDepth3(4, 12, 2, 8,
                                                                              12, 14, 10, 16);
            testreplacedepth_info *TestDepth3 = &TestDepth3_;
            DEBUGAddReplaceFinkHashLSkewedDepth3(TestDepth3);
            Result = TestDepth3->CheckFinkHashFinal;
        } break;

        case FinkHash_R1SkewedDepth3:
        {
            testreplacedepth_info TestDepth3_ = InitTestFinkHashReplaceDepth3(4, 6, 2, 8,
                                                                              12, 14, 10, 16);
            testreplacedepth_info *TestDepth3 = &TestDepth3_;
            DEBUGAddReplaceFinkHashR1SkewedDepth3(TestDepth3);
            Result = TestDepth3->CheckFinkHashFinal;
        } break;

        InvalidDefaultCase;
    }

    // printf("------------------------------------\n");

    return(Result);
}

enum testcase_finkhash_replace
{
    TestBalancedDepth2ReplaceL1a = BitSet(0),
    TestBalancedDepth2ReplaceL1b = BitSet(1),
    TestBalancedDepth2ReplaceR1a = BitSet(2),
    TestBalancedDepth2ReplaceR1b = BitSet(3),

    TestBalancedDepth3ReplaceL1a = BitSet(4),
    TestBalancedDepth3ReplaceL1b = BitSet(5),
    TestBalancedDepth3ReplaceR1a = BitSet(6),
    TestBalancedDepth3ReplaceR1b = BitSet(7),
    TestBalancedDepth3ReplaceL2a = BitSet(8),
    TestBalancedDepth3ReplaceL2b = BitSet(9),
    TestBalancedDepth3ReplaceR2a = BitSet(10),
    TestBalancedDepth3ReplaceR2b = BitSet(11),

    TestLSkewedDepth2ReplaceR1a = BitSet(12),
    TestRSkewedDepth2ReplaceR1a = BitSet(13),

    TestLSkewedDepth3ReplaceL2a = BitSet(14),
    TestLSkewedDepth3ReplaceAllByType = BitSet(15),

    TestR1SkewedDepth3ReplaceR1a = BitSet(16),
};

// NOTE(chowie): These test cases are modified from the start, to validate/assert
// it's also the same when traversing the tree.
internal void
TestDoubleCheckFinkHashReplace(u32 Flag)
{
    testreplacedepth_info TestDepth2_ = InitTestFinkHashReplaceDepth2(4, 6, 2, 8);
    testreplacedepth_info *TestDepth2 = &TestDepth2_;

    testreplacedepth_info ResetDepth2_ = *TestDepth2;
    testreplacedepth_info *ResetDepth2 = &ResetDepth2_;

    DEBUGAddReplaceFinkHashDepth2(TestDepth2);
    DEBUGEndReplaceFinkHashDepth(TestDepth2, ResetDepth2);

    if(FlagSet(Flag, TestBalancedDepth2ReplaceL1a))
    {
        TestDepth2->HashL1a = 6; // Test1 ({4->6, 6})
        DEBUGAddReplaceFinkHashDepth2(TestDepth2);
        DEBUGEndReplaceFinkHashDepth(TestDepth2, ResetDepth2);
    }

    if(FlagSet(Flag, TestBalancedDepth2ReplaceL1b))
    {
        TestDepth2->HashL1b = 8; // Test2 ({4, 6->8})
        DEBUGAddReplaceFinkHashDepth2(TestDepth2);
        DEBUGEndReplaceFinkHashDepth(TestDepth2, ResetDepth2);
    }

    if(FlagSet(Flag, TestBalancedDepth2ReplaceR1a))
    {
        TestDepth2->HashR1a = 8; // Test3 ({2->8, 8})
        DEBUGAddReplaceFinkHashDepth2(TestDepth2);
        DEBUGEndReplaceFinkHashDepth(TestDepth2, ResetDepth2);
    }

    if(FlagSet(Flag, TestBalancedDepth2ReplaceR1b))
    {
        TestDepth2->HashR1b = 6; // Test4 ({2, 8->6})
        DEBUGAddReplaceFinkHashDepth2(TestDepth2);
        DEBUGEndReplaceFinkHashDepth(TestDepth2, ResetDepth2);
    }

    if(FlagSet(Flag, TestLSkewedDepth2ReplaceR1a))
    {
        TestDepth2->HashR1a = 8; // Test4 ({2->8, 8})
        DEBUGAddReplaceFinkHashLSkewedDepth2(TestDepth2);
        DEBUGEndReplaceFinkHashDepth(TestDepth2, ResetDepth2);
    }

    if(FlagSet(Flag, TestRSkewedDepth2ReplaceR1a))
    {
        TestDepth2->HashR1a = 8; // Test4 ({2->8, 8})
        DEBUGAddReplaceFinkHashRSkewedDepth2(TestDepth2);
        DEBUGEndReplaceFinkHashDepth(TestDepth2, ResetDepth2);
    }

    //
    //
    //

    testreplacedepth_info TestDepth3_ = InitTestFinkHashReplaceDepth3(4, 6, 2, 8,
                                                                      12, 14, 10, 16);
    testreplacedepth_info *TestDepth3 = &TestDepth3_;

    testreplacedepth_info ResetDepth3_ = *TestDepth3;
    testreplacedepth_info *ResetDepth3 = &ResetDepth3_;

    DEBUGAddReplaceFinkHashDepth3(TestDepth3);
    DEBUGEndReplaceFinkHashDepth(TestDepth3, ResetDepth3);

    if(FlagSet(Flag, TestBalancedDepth3ReplaceL1a))
    {
        // printf("TestBalancedDepth3ReplaceL1a: 4->6\n");
        TestDepth3->HashL1a = 6; // Test1 ({4->6, 6})
        DEBUGAddReplaceFinkHashDepth3(TestDepth3);
        DEBUGEndReplaceFinkHashDepth(TestDepth3, ResetDepth3);
    }

    if(FlagSet(Flag, TestBalancedDepth3ReplaceL1b))
    {
        // printf("TestBalancedDepth3ReplaceL1b: 6->8\n");
        TestDepth3->HashL1b = 8; // Test1 ({4, 6->8})
        DEBUGAddReplaceFinkHashDepth3(TestDepth3);
        DEBUGEndReplaceFinkHashDepth(TestDepth3, ResetDepth3);
    }

    if(FlagSet(Flag, TestBalancedDepth3ReplaceR1a))
    {
        // printf("TestBalancedDepth3ReplaceR1a: 2->8\n");
        TestDepth3->HashR1a = 8; // Test1 ({2->8, 8})
        DEBUGAddReplaceFinkHashDepth3(TestDepth3);
        DEBUGEndReplaceFinkHashDepth(TestDepth3, ResetDepth3);
    }

    if(FlagSet(Flag, TestBalancedDepth3ReplaceR1b))
    {
        // printf("TestBalancedDepth3ReplaceR1b: 8->6\n");
        TestDepth3->HashR1b = 6; // Test4 ({2, 8->6})
        DEBUGAddReplaceFinkHashDepth3(TestDepth3);
        DEBUGEndReplaceFinkHashDepth(TestDepth3, ResetDepth3);
    }

    if(FlagSet(Flag, TestBalancedDepth3ReplaceL2a))
    {
        // printf("TestBalancedDepth3ReplaceL2a: 12->14\n");
        TestDepth3->HashL2a = 14; // Test1 ({12->14, 14})
        DEBUGAddReplaceFinkHashDepth3(TestDepth3);
        DEBUGEndReplaceFinkHashDepth(TestDepth3, ResetDepth3);
    }

    if(FlagSet(Flag, TestBalancedDepth3ReplaceL2b))
    {
        // printf("TestBalancedDepth3ReplaceL2b: 14->16\n");
        TestDepth3->HashL2b = 16; // Test1 ({12, 14->16})
        DEBUGAddReplaceFinkHashDepth3(TestDepth3);
        DEBUGEndReplaceFinkHashDepth(TestDepth3, ResetDepth3);
    }

    if(FlagSet(Flag, TestBalancedDepth3ReplaceR2a))
    {
        // printf("TestBalancedDepth3ReplaceR2a: 10->16\n");
        TestDepth3->HashR2a = 16; // Test1 ({10->16, 16})
        DEBUGAddReplaceFinkHashDepth3(TestDepth3);
        DEBUGEndReplaceFinkHashDepth(TestDepth3, ResetDepth3);
    }

    if(FlagSet(Flag, TestBalancedDepth3ReplaceR2b))
    {
        // printf("TestBalancedDepth3ReplaceR2b: 16->14\n");
        TestDepth3->HashR2b = 14; // Test1 ({10, 16->14})
        DEBUGAddReplaceFinkHashDepth3(TestDepth3);
        DEBUGEndReplaceFinkHashDepth(TestDepth3, ResetDepth3);
    }

    if(FlagSet(Flag, TestLSkewedDepth3ReplaceL2a))
    {
        // printf("TestLSkewedDepth3ReplaceL2a: 12->10\n");
        TestDepth3->HashL2a = 10; // Test1 ({12->10})
        DEBUGAddReplaceFinkHashLSkewedDepth3(TestDepth3);
        DEBUGEndReplaceFinkHashDepth(TestDepth3, ResetDepth3);
    }

    if(FlagSet(Flag, TestLSkewedDepth3ReplaceAllByType))
    {
        // printf("TestLSkewedDepth3ReplaceAllByType: 12->10\n");
        TestDepth3->HashL1b = 10; // Test1 ({12->10})
        TestDepth3->HashL2a = 10; // Test1 ({12->10})
        DEBUGAddReplaceFinkHashLSkewedDepth3(TestDepth3);
        DEBUGEndReplaceFinkHashDepth(TestDepth3, ResetDepth3);
    }

    if(FlagSet(Flag, TestR1SkewedDepth3ReplaceR1a))
    {
        // printf("TestR1SkewedDepth3ReplaceR1a: 2->8\n");
        TestDepth3->HashR1a = 8; // Test1 ({2->8})
        DEBUGAddReplaceFinkHashR1SkewedDepth3(TestDepth3);
        DEBUGEndReplaceFinkHashDepth(TestDepth3, ResetDepth3);
    }
}

//
// Fink hash main
//

internal void
ProcessFinkHashTree(void)
{
    u64 LUTFinkHash = TestFinkHash(FinkHash_BalancedDepth3); // TestFinkHashMax(BalancedDepth2Max);

    // IMPORTANT(onix242): Add, delete and replace is determined by the params
    // (since they share the same code path = add and delete is just a replace!)

    // NOTE(onix242): If you want to, you can switch to an index-based mode
    // (should produce identical results)
    // 1. FinkHashTreeReadWriteParams(FinkHashTree_Type_ReadWriteReplace, FinkHashTree_Mode_FirstOfValue, 16, 14)
    // 2. FinkHashTreeReadWriteParams(FinkHashTree_Type_ReadWriteReplace, FinkHashTree_Mode_Index, 7, 14)
    // OR try reading in the results!
    // 1. FinkHashTreeReadAllParams();
    // 2. FinkHashTreeReadParams(FinkHashTree_Mode_FirstOfValue, 16);
    // 3. FinkHashTreeReadParams(FinkHashTree_Mode_Index, 7);
    finkhashtree_params TreeParams = FinkHashTreeReplaceParams(FinkHashTree_Mode_FirstOfValue, 16, 14);
    
    // IMPORTANT(chowie): Allowed trees:
    // 1. Full tree with leaves (of at least _one different leaf value_)
    // 2. Just root/leaf. All leaves have the same leaf value
    // COULDDO(chowie): Use GetDepthFinkHashPerfectTree(u64 Hash) to determine if it's a 2-depth tree or 3-depth tree!
    finkhashtree_group TreeGroup_ = InitFinkHashTreeGroup(LUTFinkHash, TreeParams);
    finkhashtree_group *TreeGroup = &TreeGroup_;

    switch(TreeGroup->Params.Type)
    {
        case FinkHashTree_Type_Read:
        {
            if(IsParent(TreeGroup->Tree.FinkHash))
            {
                FinkHashTreeRead(TreeGroup);
            }
        } break;

        case FinkHashTree_Type_ReadWriteReplace:
        case FinkHashTree_Type_ReadWriteAdd:
        case FinkHashTree_Type_ReadWriteDelete:
        {
            if(IsLeaf(TreeGroup->Tree.FinkHash))
            {
                if(TreeGroup->Params.Mode == FinkHashTree_Mode_None)
                {
                    TreeGroup->Tree.FinkHash = TreeGroup->Params.Data.DestLeafValue;
                    break;
                }
                else
                {
                    finkhash_expandmode ExpandMode = {};
                    if(TreeGroup->Tree.FinkHash <= PERFECTTREEPRIM_MAX3DEPTH)
                    {
                        ExpandMode = FinkHash_ExpandMode_3Depth;
                    }
                    else
                    {
                        ExpandMode = FinkHash_ExpandMode_2Depth;
                    }

                    TreeGroup->Tree.FinkHash = FinkHashExpandLeafToPerfectTree(TreeGroup->Tree.FinkHash, ExpandMode);
                }
            }

            // TODO(chowie): Change to FinkHashTreeReadWritePerfectTree(TreeGroup)
            // if you want to utilise FinkHashExpandLeafToPerfectTree()
            FinkHashTreeReadWriteFullTree(TreeGroup);

            // NOTE(chowie): Cleans up replacing a leaf to be full/empty with the same block IDs
            TreeGroup->Tree.FinkHash = FinkHashTreeBubbleupCommonLeafInPerfectTreeIfPossible(TreeGroup->Tree.FinkHash);
        } break;

        InvalidDefaultCase;
    }
}

//
// Fink hash future work
//

// RESOURCE(): https://en.wikipedia.org/wiki/Tree_rotation
// TODO(chowie): IMPORTANT(chowie): Tree roll?
//  root (ltr)    root (rtl)
//   /\            /\
//  /\ 47         2 /\
// 2  7            7 47
// TODO(chowie): Can you swap the node order easily? Or is rebuilding it quicker?

// TODO(chowie): Tree balancing?
//     root
//   /\    /\
//  /\ 47 2 /\
// 2  7    7 47
// NOTE(chowie): Actually, I can combine two different trees easily!!

// COULDDO(chowie): Sample API = {2, 7, 47}, improve API with varargs? Check if the number of varargs is odd/even?
// API something like how I do memory arenas?? "#define {2, 7, ...} MerkleTreeFinkHashRecursive(A, B, #varargs)"
// E.g. "H (UTO)H H(IUV)", H = 2, (UTO) = 0, (IUV) = 4. Doesn't really matter unless a specific pair is a set phrase!

//
// Fink hash manual working out
//

// COULDDO(chowie): RESOURCE(): https://andrew-helmer.github.io/tree-shuffling/

// NOTE(chowie): If you knew info about A and B
// 1)
// 1a Maximum(A, B) = A must the largest value to produce the greatest result
// 1b Diff is always = "2*abs(A - B)". Add this difference to the lowest will be correct!

// 2)
// 2a (Min, Max) -> (must lie at an even triangle number row) = (Value - 1)/2
// 2b (Max, Min) -> (= 2a.Value + abs(A - B))
// 2b+ Check if result isn't a triangle number. If not, "Result - abs(A - B)"
// 2c Next even number proceeding A or B are two triangle numbers
// apart! e.g. f(2, 0) and f(4, 0).

//
// Dual.
//

// f(0, 2) = 7 -> 3
// f(2, 4) = 47 -> 23
// Thus, f(7, 47) = 2985 -> 1492

//
// Sing.
//

// f(0, 2) = 7 -> 3
// f(2, 0) = 11 >> 5

// f(0, 4) = 21 -> 10
// f(4, 0) = 29 >> 14

// f(0, 6) = 43 -> 21
// f(6, 0) = 55 >> 27

// f(0, 8) = 73 -> 36
// f(8, 0) = 89 >> 44

//
// Sing.
//

// f(2, 4) = 47 -> 23
// f(4, 2) = 51 >> 25

// f(2, 6) = 77 -> 38
// f(6, 2) = 85 >> 42

// f(2, 8) = 115 -> 57
// f(8, 2) = 127 >> 63

//
// Sing.
//

// f(4, 6) = 119 -> 59
// f(6, 4) = 123 >> 61

// f(4, 8) = 165 -> 82
// f(8, 4) = 173 >> 86

// f(a, b) = FinkHash (full) -> RemovedHashParity (raw)
// NOTE(chowie): IMPORTANT(chowie): Prims must be even steps

// f(0, 2) = 7 -> 3
// f(2, 0) = 11 -> 5

// f(0, 4) = 21 -> 10
// f(4, 0) = 29 -> 14

// f(0, 6) = 43 -> 21
// f(6, 0) = 55 -> 27

// f(0, 8) = 73 -> 36
// f(8, 0) = 89 -> 44

//

// f(2, 4) = 47 -> 23
// f(4, 2) = 51 -> 25

// f(2, 6) = 77 -> 38
// f(6, 2) = 85 -> 42

// f(2, 8) = 115 -> 57
// f(8, 2) = 127 -> 63

//

// f(4, 6) = 119 -> 59
// f(6, 4) = 123 -> 61

// f(4, 8) = 165 -> 82
// f(8, 4) = 173 -> 86

//

// NOTE(chowie): The full hash are perfect squares! + 2^4 is the difference
// f(0, 0) = 1 -> 0
// f(2, 2) = 25 -> 12
// f(4, 4) = 81 -> 40
// f(6, 6) = 169 -> 84
// f(8, 8) = 289 -> 144

#define RUINENGLASS_HASH_H
#endif
