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
    u32 Max = Maximum3(Value.x, Value.y, Value.z);
    u32 Result = CUBIC(Max) + (2*Max*Value.z) + Value.z;
    if(Max == Value.z)
    {
        Result += QUADRATIC(Maximum(Value.x, Value.y));
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

    s32 Max = Maximum3(NegX, NegY, NegZ);
    s32 Result = CUBIC(Max) + (2*Max*NegZ) + NegZ;
    if(Max == NegZ)
    {
        Result += QUADRATIC(Maximum(NegX, NegY));
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
    Result = AbsoluteValue(Result);
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
EncodeMortonV2U(v3u Value)
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

//
// Pairwise
//

// COULDDO(chowie): Where to take these next:
// - Hash this?
// - Convert numbers to enum
// - Figure out how this integrates with (debug) UI/chart?

// RESOURCE(): https://en.wikipedia.org/wiki/Triangular_number
// Technically more correct to process the largest int with binary OR (, branchless), too niche for me

// RESOURCE(mason remaley): https://gamesbymason.com/2020/03/30/symmetric-matrices/
// f(x,y) = (x + (y)(y+1)/2), x = low/column, y = high/triangle
// NOTE(chowie): For symmetric tables
inline u32
TriangleNumber(u32 Value)
{
    u32 Result = Value + (Value*(Value + 1))/2;
    return(Result);
}

// NOTE(chowie): For asymmetric tables
inline u32
TriangleNumber(v2u Value)
{
    u32 Result = Value.x + ((Value.y)*(Value.y + 1))/2;
    return(Result);
}

// NOTE(chowie): Non-pairwise, must be used with hierarchical trees
// RESOURCE(alex fink & jorg rhiemeier): https://listserv.brown.edu/archives/cgi-bin/wa?A2=ind0907B&L=CONLANG&P=R12478
// P.S. Original concept from 1999 amazingly! It's not even your main point??
// f(x,y) = 1 + 2(x + (x+y)(x+y+1)/2)
inline u32
NonPairwiseHierarchicalTriangleNumber(v2u Value)
{
    v2u Offset = V2U(Value.x, Value.x + Value.y);
    u32 Result = 2*TriangleNumber(Offset) + 1;
    return(Result);
}

// RESOURCE(): https://stackoverflow.com/questions/2913215/fastest-method-to-define-whether-a-number-is-a-triangular-number
inline u32
IsTriangleNumber(u32 Value)
{
    u32 Result = (u32)((SquareRoot(8*(f32)Value + 1) - 1)/2);
    return(Result);
}

// RESOURCE(mason remaley): https://gamesbymason.com/2020/03/30/symmetric-matrices/
internal u32
MapPairToPairwise(u16 A, u16 B)
{
    v2u Pair = V2U((u32)B, (u32)A);
    if(A < B)
    {
        Swap(u32, Pair.x, Pair.y);
    }

    u32 Result = TriangleNumber(Pair);
    return(Result);
}

inline u16
GetPairwiseRow(u32 Ordinal)
{
    f32 Row = (f32)IsTriangleNumber(Ordinal);
    u16 Result = FloorF32ToU16(Row); // STUDY(chowie): New technique I observed to quickly get the row!
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
    u16 Row = GetPairwiseRow(Ordinal);
    u16 Pitch = (u16)(TriangleNumber(Row - 1) + 1); // STUDY(chowie): Neat way I found to handle 0th case without a clamp

    pairwise_index_result Result = {};
    Result.Row = Row;
    Result.Col = (u16)Ordinal - Pitch;
    Result.IsTriangleNumber = (Ordinal == TriangleNumber((u32)Result.Row)) ? true : false;

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
//    TestHash_EntityType_Familiar,
//    TestHash_EntityType_Monstar,
//    TestHash_EntityType_Sword,
//    TestHash_EntityType_Stairwell,

    TestHash_EntityType_Count,
};

// TODO(chowie): Replace with actual TriangleNumber(TableDim)!!
#define PAIRWISE_TABLE_MAX(TableDim) (TableDim * (TableDim + 1) / 2)
struct pairwise
{
    u32 TypeTable[PAIRWISE_TABLE_MAX(TestHash_EntityType_Count)];
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

#define RUINENGLASS_HASH_H
#endif
