#if !defined(RUINENGLASS_HASH_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Michael Chow $
   $Notice: $
   ======================================================================== */

// TODO(chowie): Better Hash Function

//
// General
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
// String
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
// Spatial
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
// Triangle Numbers
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
    f64 Result = (Sqrt64(8*(f64)Value + 1) - 1)/2;
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
// Pairwise (reversible)
//

// COULDDO(chowie): Where to take these next:
// - Hash this?
// - Convert numbers to enum
// - Figure out how this integrates with (debug) UI/chart?

// RESOURCE(mason remaley): https://gamesbymason.com/2020/03/30/symmetric-matrices/
internal u32
MapPairToPairwise(u16 A, u16 B)
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
GetPairFromPairwise(u32 Ordinal)
{
    u16 Row = GetPairwiseRow16(Ordinal);
    u16 Pitch = (u16)TriangleNumber(Row); // STUDY(chowie): Neat way I found to handle 0th case without a clamp

    pairwise_index_result Result = {};
    Result.Row = Row;
    Result.Col = (u16)Ordinal - Pitch;
    Result.IsTriangleNumber = (Ordinal == TriangleNumberMat((u32)Result.Row));
    return(Result);
}

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
// Non-Pairwise (invertible)
//

//
// SAMPLE PASSED FUNCTIONS IN FINK HASH TO PLAY WITH
//

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

// NOTE(chowie): "Adding two adjacent triangle numbers = square number"
// NOTE(chowie): Step = 1 advances, and Step = -1 backsteps
inline u64
FinkHashStep(u64 CurrentTriangleNumber, s64 Step)
{
    u64 Row = GetPairwiseRow64(CurrentTriangleNumber);
    u64 Pitch = CurrentTriangleNumber - TriangleNumber64(Row);
    u64 Result = Pitch + TriangleNumber64(Row + Step);
    return(Result);
}

// NOTE(chowie): Initially worked out eq manually

// Step 1. Convert the fink hash (full) into triangle number form (raw).
// To do the reverse transformation by -1 then /2
// Full form: f(x,y) = 1 + 2(x + (x+y)(x+y+1)/2) -- becomes --> Raw Form: = (x + (x+y)(x+y+1)/2)
// e.g. f(2, 8) = 115 (full) -> 57 (raw). Raw form (57) is the CurrentTriangleNumber

// Step 2. Take an example, calculate knowing f(2, 8) -> 57 (CurrentTriangleNumber),
// Remember Raw Form: f(x,y) = (x + (x+y)(x+y+1)/2)
// a) We're looking for x+y, call it ResultXY = ??. This is the result of the reverse hash
// b) Consequently, we already know the CurrentTriangleNumber = 57,
//    also we need a reference point where we are, PrevTwoTriangleNumber = 38
//    (IMPORTANT(chowie): Not the prev triangle number but prev two because the
//     shortest difference between two prims is an even distance see above "SAMPLE PASSED FUNCTIONS" section)
// c) Get the difference between CurrentTriangleNumber and PrevTwoTriangleNumber
// d) Simulate GetPairwiseRowFull(u32 Ordinal), remember it calls IsTriangleNumber(u32 Value).
//    Note the "- 1)/2" of the complete check "(sqrt(8*(f32)Value + 1) - 1)/2".
//    Now use "- 1)/2" and undo the odd parity (floor -- becomes --> ceil), because
//    we know prims when added up can't be odd (prims must be even), so use "+1"
// Whole eq works out to be:
// ResultXY = (CurrentTriangleNumber - PrevTwoTriangleNumber - 1)/2 + 1
// x+y = (57 - 38 - 1)/2 + 1
// x+y = 10

// Step 3. Substitute all the values in to find how ResultXY (10) breaks down!
// Remember what everything represents:
// 1. CurrentTriangleNumber: f(x,y) = 57
// 2. ResultXY: x+y = 10
// Substitute into Raw Form: f(x,y) = (x + (x+y)(x+y+1)/2)
// Solve for x:
// 57 = (x + (x+y)(x+y+1)/2)
// 57 = x + (110/2)
// 57 = x + 55
// x = 2,
// Solve for y (Substitute x in x+y = 10):
// y = 8 QED

// Check if this works for other eq:
// f(2, 6)
// 8 = x+y
// 38 = x + (72/2)
// x = 2, substitute
// y = 6 QED

// f(0, 8) =
// 8 = x+y
// 36 = x + (72/2)
// x = 0, substitute
// y = 8, substitute

// TODO(chowie): Simplify?
// STUDY(chowie): Many invertible hashes fundamentally uses odd numbers, won't work without it.
internal v2u64
InvertFinkHash(u64 Hash)
{
    // Step 1. Remove parity from equation and leaves triangle, get reference point with PrevTwoTriangleNumbers
    u64 CurrentTriangleNumber = RemoveFinkHashParity(Hash);
    u64 PrevTwoTriangleNumbers = FinkHashStep(CurrentTriangleNumber, -2);

    // RESOURCE(): https://opendatastructures.org/ods-cpp/10_1_Implicit_Binary_Tree.html
    // I only found this info after, to return to the parent is "(i-1)/2", mirrors what I have below.
    // Step 2. Solve for x+y, "x+y = (CurrentTriangleNumber - PrevTwoTriangleNumber - 1)/2 + 1"
    u64 ResultXY = (CurrentTriangleNumber - PrevTwoTriangleNumbers - 1)/2 + 1;

    // Step 3. Substitute x+y (ResultXY), "CurrentTriangleNumber = (x + (x+y)(x+y+1)/2)"
    // Solve for x, "CurrentTriangleNumber - (x+y)(x+y+1)/2)"
    // Solve for y, "x+y - x"
    v2u64 Result =
    {
        CurrentTriangleNumber - TriangleNumber64(ResultXY),
        ResultXY - Result.x,
    };
    return(Result);
}

//
// OTHER TREE CONSTRUCTIONS
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

// NOTE(chowie): Function allows to transform into a linked list/array representation
// ltr and rtl are the directions the fink hash gets appended
//  root (ltr)    root (rtl)
//   /\            /\
//  /\ 2          2 /\
// 7  47           7 47 -- base --
// NOTE(chowie): Can change the order whenever (ltr or rtl per branch), even interleaved.
// COULDDO(chowie): Basically the same as MapPairToPairwise(u16 A, u16 B), combine them?
internal u64
FinkHashBranchDir(v2u64 Hash, b32x ltr = false)
{
    // NOTE(chowie): Default rtl like linguistic syntax trees
    v2u64 Pair = {Hash.b, Hash.a}; // FinkHash.a = Base | FinkHash.b = NewHash
    if(ltr)
    {
        Swap(u64, Pair.a, Pair.b);
    }

    u64 Result = FinkHash(Pair);
    return(Result);
}

//
// FINK HASH TREE TRAVERSAL CHECKS
//

// NOTE(onix242): Because how f(x, y) is mapped, a neat optimisation is
// checking for odd in tree hierarchy finds if it's a parent! 'Even natural
// numbers' (includes 0) = leaf, 'odd' = parent
inline b32x
HasChildrenFromFinkHash(u64 Hash)
{
    b32x Result = Odd(Hash);
    return(Result);
}

// NOTE(chowie): Traverse DFS (Depth-First) to the bottom of the tree
// is very efficient (each branch/node is weighted based on hash,
// larger hash/weight = deeper). Once at the bottom, you know the max
// iterations for each branch (MaxTreeDepth - CurrentTreeDepth).
inline v2u64
GetDeepestAndShallowestFinkHashBranch(v2u64 Pair)
{
    v2u64 Result = MinMax64(Pair);
    return(Result);
}

inline u64
GetDeepestFinkHashBranch(v2u64 Pair)
{
    u64 Result = Max(Pair.a, Pair.b);
    return(Result);
}

// NOTE(chowie): Shallowest determines order if branch is ltr or rtl (unless a tie)
inline u64
GetShallowestFinkHashBranch(v2u64 Pair)
{
    u64 Result = Min(Pair.a, Pair.b);
    return(Result);
}

// COULDDO(chowie): I could do an optimisation to save out the first
// unpack from DFS to then use it for traversal later, but I think
// it's faster to not have to worry about storing/managing them (if it
// can all be done in-place)
// NOTE(chowie): Weighted tree allows O(n), where n = tree depth
internal u32
GetMaxTreeDepthDFS(u64 Hash)
{
    u32 Depth = 0;
    for(;
        HasChildrenFromFinkHash(Hash); // NOTE(chowie): Until it finds deepest leaf
        ++Depth)
    {
        v2u64 Pair = InvertFinkHash(Hash);
        Hash = GetDeepestFinkHashBranch(Pair);
    }
    return(Depth);
}

// TODO(chowie): Merkle trees are good for validating a node without
// the whole tree! Check sum with the root. According to John D Cook
inline b32x
VerifyFinkHashTreeIntegrity(u64 Source, u64 Copy)
{
    b32x Result = (Source == Copy);
    return(Result);
}

//
//
//

// Step 1. Set conword_prim_Count = 4
// Step 2. EvenPrims = (conword_prim_Count*2): Even prims (Largest possible lowest level prim possible) -> not useful
// Step 3. LargestEvenPrimsPairs = EvenPrims*2: Largest possible lowest level prim combinations/pairs (between each other)
// Step 4. LargestEvenPrimsPairs*EvenPrims = Largest possible lowest level combinations/pairs with a lowest level prefab with a prim

inline u64
GetPrimPairHashLevelBand(u64 PrimCount)
{
    u64 Result = FinkHash(V2U64(2*PrimCount));
    return(Result);
}

inline u64
GetPrimPrefabPairHashLowestLevelBandUpperBounds(u64 PrimCount)
{
    // NOTE(chowie): Order matters! To get the largest fink hash result,
    // f(x, y): x = Largest, y = Smallest (see SAMPLE PASSED FUNCTIONS above).
    // This is because x shows up more than y "f(x,y) = 1 + 2(x + (x+y)(x+y+1)/2)"
    u64 Result = FinkHash(V2U64(GetPrimPairHashLevelBand(PrimCount), 2*PrimCount));
    return(Result);
}

inline u64
GetPrimPrefabPairHashLowestLevelBandLowerBounds(u64 PrimCount)
{
    u64 Result = FinkHash(V2U64(2*PrimCount, GetPrimPairHashLevelBand(PrimCount)));
    return(Result);
}

inline u64
GetPrefabPairHashLowestLevelBand(u64 PrimCount)
{
    u64 Result = FinkHash(V2U64(GetPrimPairHashLevelBand(PrimCount)));
    return(Result);
}

enum finkhash_band_type
{
    Band_PrimPair,

    Band_LowestPrimPrefabPairAnyOrder,

    Band_LowestPrimPrefabPairUpperBounds,
    Below_Band_LowestPrimPrefabPairUpperBounds,

    Band_LowestPrimPrefabPairLowerBounds,
    Below_Band_LowestPrimPrefabPairLowerBounds,

    Band_LowestPrefabPair,
    Below_Band_LowestPrefabPair,
};
// NOTE(chowie): Basic search filtering for tree traversal. Finds the lowest depth
// of the tree with building 'blocks'
// COULDDO(chowie): My initial hypothesis is checks for the higher level branches
// greatly diminishes in value since the most pertinent info is at the leaves
inline b32x
IsPairWithinLowestLevelBandingFromFinkHash64(u64 FinkHash, u64 PrimCount, finkhash_band_type Type)
{
    b32x Result;
    switch(Type)
    {
        case Band_PrimPair:
        {
            // NOTE(chowie): To use IsPerfectSquare(FinkHash) would be slower
            Result = (FinkHash <= GetPrimPairHashLevelBand(PrimCount));
        } break;

        case Band_LowestPrimPrefabPairAnyOrder:
        {
            Result = ((GetPrimPairHashLevelBand(PrimCount) > FinkHash) &&
                      (FinkHash <= GetPrimPrefabPairHashLowestLevelBandUpperBounds(PrimCount)));
        } break;

        case Band_LowestPrimPrefabPairUpperBounds:
        {
            Result = ((GetPrimPrefabPairHashLowestLevelBandLowerBounds(PrimCount) > FinkHash) &&
                      (FinkHash <= GetPrimPrefabPairHashLowestLevelBandUpperBounds(PrimCount)));
        } break;

        case Band_LowestPrimPrefabPairLowerBounds:
        {
            Result = ((GetPrimPairHashLevelBand(PrimCount) > FinkHash) &&
                      (FinkHash <= GetPrimPrefabPairHashLowestLevelBandLowerBounds(PrimCount)));
        } break;

        case Below_Band_LowestPrimPrefabPairUpperBounds:
        {
            Result = (FinkHash <= GetPrimPrefabPairHashLowestLevelBandUpperBounds(PrimCount));
        } break;

        case Below_Band_LowestPrimPrefabPairLowerBounds:
        {
            Result = (FinkHash <= GetPrimPrefabPairHashLowestLevelBandLowerBounds(PrimCount));
        } break;

        case Band_LowestPrefabPair:
        {
            Result = ((GetPrimPrefabPairHashLowestLevelBandUpperBounds(PrimCount) > FinkHash) &&
                      (FinkHash <= GetPrefabPairHashLowestLevelBand(PrimCount)));
        } break;

        case Below_Band_LowestPrefabPair:
        {
            Result = (FinkHash <= GetPrefabPairHashLowestLevelBand(PrimCount));
        } break;

        InvalidDefaultCase;
    }

    return(Result);
}

//
//
//

// COULDDO(chowie): RESOURCE(): https://andrew-helmer.github.io/tree-shuffling/

// IMPORTANT(chowie): Primitives (child nodes) must be even values!
// TODO(chowie): Assert this!

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

#define RUINENGLASS_HASH_H
#endif
