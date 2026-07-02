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
// GENERAL (N -> N)
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

// RESOURCE(): https://www.jcgt.org/published/0009/03/02/paper.pdf
// RESOURCE(): https://web.archive.org/web/20240610235256/https://rene.ruhr/gfx/gpuhash/
// NOTE(chowie): For multi-threading, SIMD
// NOTE(chowie): These are also good for just generating random numbers
inline v2u
PCGHash2D(v2u Value)
{
    Value = Value*1664525u + V2U(1013904223u);

    Value.x += Value.y*1664525u;
    Value.y += Value.x*1664525u;

    Value.x ^= Value.x >> 16u;
    Value.y ^= Value.y >> 16u;

    Value.x += Value.y*1664525u;
    Value.y += Value.x*1664525u;

    Value.x ^= Value.x >> 16u;
    Value.y ^= Value.y >> 16u;

    return Value;
}

// NOTE(chowie): For multi-threading, SIMD
// NOTE(chowie): These are also good for just generating random numbers
inline v3u
PCGHash3D(v3u Value)
{
    Value = Value*1664525u + V3U(1013904223u);

    Value.x += Value.y*Value.z;
    Value.y += Value.z*Value.x;
    Value.z += Value.x*Value.y;

    Value.x ^= Value.x >> 16u;
    Value.y ^= Value.y >> 16u;
    Value.z ^= Value.z >> 16u;

    Value.x += Value.y*Value.z;
    Value.y += Value.z*Value.x;
    Value.z += Value.x*Value.y;

    return(Value);
}

// NOTE(chowie): For multi-threading, SIMD
// NOTE(chowie): These are also good for just generating random numbers
inline v4u
PCGHash4D(v4u Value)
{
    Value = Value*1664525u + V4U(1013904223u);

    Value.x += Value.y*Value.w;
    Value.y += Value.z*Value.x;
    Value.z += Value.x*Value.y;
    Value.w += Value.y*Value.z;

    Value.x ^= Value.x >> 16u;
    Value.y ^= Value.y >> 16u;
    Value.z ^= Value.z >> 16u;
    Value.w ^= Value.w >> 16u;

    Value.x += Value.y*Value.w;
    Value.y += Value.z*Value.x;
    Value.z += Value.x*Value.y;
    Value.w += Value.y*Value.z;

    return(Value);
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
// SPATIAL (N -> 1)
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

// COULDDO(chowie): Could I not delay "-1)/2" later until after
// FloorF64toU64?, then don't need CurrentTriangleNumber? Yes you can,
// but I don't understand how to simplify everything.
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

inline u32
GetPairwiseRow32(u32 Ordinal)
{
    f32 Row = IsTriangleNumber(Ordinal);
    u32 Result = FloorF32ToU32(Row); // STUDY(chowie): New technique I observed to quickly get the row!
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
// Remaley Hash (2 -> 1 perfect) for pairwise
//

/********************************
        WHAT IS REMALEY HASH?
   ******************************

   2->1 perfect hash
   It's like triangular matrices but functions more like mapping values.
   Doesn't store data within hash, just possible combinations. Used for IDs
   Input order agnostic (pairwise means swaps min to ensure ID is consistent).

   For pairwise interaction, remaley hash stores only the necessary
   data of a matrix. While still indexing linearly with an array:
   - 0 = 0x0
   - 1 = 0x1
   - 2 = 1x0
   - 3 = 0x2
   - 4 = 1x2
   - 5 = 2x2

   You can still process these with "+= 1" when looping through as usual

   #
   # #
   # # #
   # # # #
   # # # # #
   # # # # # #

   - Stores undirected graph edges and update simultaneously on both ends (pathfinding)
     = As standard, serialise a node list and edge list separately.
     = Use Remaley hash to avoid double dispatch problem i.e. updating
     edges between nodes usually means updating on both ends. Remaley
     hash stores a single edge as a number and updates both ends
     simultaneously by altering the number, O(n). It can be streamed
     in too! (for ease, nodes should store a separate number_id enum)
     = Supports self-loop edges (diagonal of matrix), e.g. RemaleyHash(1, 1)
     = Supports null edges, e.g. RemaleyHash(1, 0)
     = Use search O(n) pairwise by row/col to get a list of all
     connections (if available)
   - OR can be used to store combinatorics (matrix) in a linear array

   Please use this for:
   - Narrative Simulation:
     = Map social NPC relationship web(s) e.g. Who knows who
     = Emotional matrix for narrative/story games
     = You may combine/nest all of the above!
       e.g. struct NarrativeRemaleyHashEdge { u32 SocialRelations; u32 EmotionMatrix; u32 FAVVE; }
   - Spatial connections between towns/cities (most 4x games/MMOs/Metroidvanias), 
   (for level design) if the player can access it from their locations.
   It could mean if a road should be procedurally generated or not.
   OR just as documentation like Breath of the Wild.
     = On a micro-scale, random generation of buildings/houses? E.g. should
     the kitchen be connected to the living room or not?
   - Correlation between different parameters (for procedural animation systems),
   maybe you want to cut down on the total number of parameters (to simplify).
   - Expand out settings e.g. [software render  render by Win GDI]
                              [opengl render    render by Opengl]
   - Collision response system handled differently for each type
   - Type knockout chart like in a tournament, to check if everyone beat each other
   - Pairwise Comparison Surveys, "what out of two options do you like more?"
     = Winrate
     = Probabilistic: Bayesian/Glicko/Trueskill ELO score in competitive games
     = Binary: If only one person votes

   COULDDO(chowie): Partial pairwise comparison 3n(n-1)/2?
*/

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

// NOTE(chowie): Pairwise diagonal
internal u32
RemaleyHash(u16 A)
{
    u32 Result = TriangleNumberMat(A);
    return(Result);
}

// COULDDO(chowie): Diagonal for trigraphs (3-variable diagrams)?
// #
// # #
// # # #
// # # A #
// # A # # #
// A # # # # #

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

// Makes a upside down L-shape of triangle numbers
// #
// A A
// # A #
// # A # #
// NOTE(chowie): To show the range/permutations?
// e.g. Anger has Anger-Null, Anger-Love, Anger-Indifferent, Anger-Hate,
// Anger-Anger, Anger-Stress, Anger-Calm ... etc
// NOTE(chowie): Searching is O(n)
internal u32 *
RemaleyHashTableOppositeRows(u16 Row, u32 PairwiseTableCount)
{
    u32 *Result = 0;
    for(u32 RemaleyHashIndex = 0;
        RemaleyHashIndex < PairwiseTableCount;
        ++RemaleyHashIndex)
    {
        Result[RemaleyHashIndex] = RemaleyHash(Row, (u16)RemaleyHashIndex);
    }

    return(Result);
}

// NOTE(chowie): Random combination
internal u32
RandomRemaleyHashTableRow(pcg32_random_series *Series, u16 Row, u32 PairwiseTableCount)
{
    u32 Result = RemaleyHash(Row, (u16)RandomBounds(Series, PairwiseTableCount));
    return(Result);
}

internal u32
SampleRemaleyHashTable(pcg32_random_series *Series, u32 PairwiseTableCount)
{
    u32 Result = RemaleyHash((u16)RandomBounds(Series, PairwiseTableCount), (u16)RandomBounds(Series, PairwiseTableCount));
    return(Result);
}

internal u32
SampleRemaleyHashTableDiagonal(pcg32_random_series *Series, u32 PairwiseTableCount)
{
    u32 Result = RemaleyHash((u16)RandomBounds(Series, PairwiseTableCount));
    return(Result);
}

internal b32x
IsRowInRemaleyHash(u32 RemaleyHash, u16 SourceRow)
{
    b32x Result = false;
    pairwise_index_result Pairwise = InvertRemaleyHash(RemaleyHash);
    if((Pairwise.Row == SourceRow) || (Pairwise.Col == SourceRow))
    {
        Result = true;
    }

    return(Result);
}

#define PAIRWISE_TABLE_MAX(TableDim) (TableDim * (TableDim + 1) / 2)

// TODO(chowie): Change this to a u16 percentage with a 2D invlerp!
enum whittaker_biome : u8
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
#define WHITTAKER_BIOME_COUNT_MAX PAIRWISE_TABLE_MAX(Whittaker_Biome_Count)

// TODO(chowie): Specialised text to represent the mood.
// e.g. 99% has love hearts, 1% has an arrow through the heart
// NOTE(chowie): Inspired by Event[0] game
enum mood_matrix : u8
{
    Mood_Matrix_Null,

    Mood_Matrix_Love,
    Mood_Matrix_Indifferent,
    Mood_Matrix_Hate,

    Mood_Matrix_Calm,
    Mood_Matrix_Stress,
    Mood_Matrix_Anger,

    Mood_Matrix_Confidence,
    Mood_Matrix_Apprehension,
    Mood_Matrix_Fear,

    Mood_Matrix_Homely, // Hygge
    Mood_Matrix_Fleeting,
    Mood_Matrix_Uncomfortable,

    Mood_Matrix_Playful,
    Mood_Matrix_Distracted,
    Mood_Matrix_Bored,

    Mood_Matrix_Bonding,
    Mood_Matrix_Stranger,
    Mood_Matrix_Suspicious,

//    Mood_Matrix_Surprised,
//    Mood_Matrix_Curious,
//    Mood_Matrix_Confused,

    Mood_Matrix_Count,
};
#define MOOD_MATRIX_COUNT_MAX PAIRWISE_TABLE_MAX(Mood_Matrix_Count)

// RESOURCE(): The conceptual structure of human relationships across modern and historical culture (2025)
// NOTE(chowie): Always symbiotic relationship, never one-sided! Very child-like
enum relationship_favee : u8
{
    Relationship_FAVEE_Null,

    Relationship_FAVEE_Formality, // "Cultural Animal" - Public (Large-scale cooperation)
    Relationship_FAVEE_ActiveAffiliative, // "Social Animal" - Private (Small-scale cooperation)
    Relationship_FAVEE_ActiveRomantic, // "Social Animal" - Private (Small-scale cooperation)
    Relationship_FAVEE_ActiveFamilial, // "Social Animal" - Private (Small-scale cooperation)
    Relationship_FAVEE_Valence, // "Animal" - Hostile (no cooperation)
    Relationship_FAVEE_Exchange, // "Cultural Animal" - Public (Large-scale cooperation) i.e. Transactional
    Relationship_FAVEE_Equality, // "Cultural Animal" - Public (Large-scale cooperation) i.e. Power

    Relationship_FAVEE_Count,
};
#define RELATIONSHIP_FAVEE_COUNT_MAX PAIRWISE_TABLE_MAX(Relationship_FAVEE_Count)

// NOTE(chowie): Includes both +tive and -tive risks
enum relationship_risk : u8
{
    Relationship_Risk_Null,

    Relationship_Risk_Financial, // Includes switching occupation prospects
    Relationship_Risk_MentalHealth,
    Relationship_Risk_PhysicalHealth,
    Relationship_Risk_SocialOutcast,
    Relationship_Risk_SocialPublicIntegration, // Is relationship outwardly presentable in the public eye?
    Relationship_Risk_Trust,
    Relationship_Risk_Autonomy, // Alone time, balance other relationships
    Relationship_Risk_QualityTime,
    Relationship_Risk_Commitment,

    Relationship_Risk_Count,
};

// NOTE(chowie): Uses animal boldness-shyness model
enum animal_personality : u8
{
    Animal_Personality_Null,

    Animal_Personality_Social,
    Animal_Personality_Antisocial,

    Animal_Personality_Active,
    Animal_Personality_Inactive,

    Animal_Personality_Aggressive,
    Animal_Personality_Peaceful,

    Animal_Personality_Explorative,
    Animal_Personality_Unexplorative,

    Animal_Personality_Bold,
    Animal_Personality_Shy,

    Animal_Personality_Dominant,
    Animal_Personality_Submissive,

    Animal_Personality_Count,
};

enum animal_order : u8
{
    Animal_Order_Eurolang,
    Animal_Order_Asialang,
    Animal_Order_Calclang,
};

// NOTE(chowie): Where are you from?
enum animal_origin : u8
{
    Animal_Origin_Plains,
    Animal_Origin_Taiga,
    Animal_Origin_Swamp,
};

enum animal_lang : u8
{
    Animal_Lang_Euro = BitSet(0),
    Animal_Lang_Asia = BitSet(1),
    Animal_Lang_Calc = BitSet(2),
};

enum animal_occupation : u8
{
    Occupation_Clockmaker,
    Occupation_GraffitiArtist,
    Occupation_Builder,
    Occupation_ToyMaker,
    Occupation_Guide,
    Occupation_WeatherGirl,
    Occupation_Librarian,
    Occupation_Confidant,
    Occupation_FieldLinguist,
    Occupation_Interpreter,
};

enum inhabitant_name : u16
{
    Inhabitant_Name_IXI,
    Inhabitant_Name_IVVI,
    Inhabitant_Name_IOTHI,

    Inhabitant_Name_Count,
};

// TODO(chowie): Make sure this undirected graph for story/narrative is good

/********************************
       WHAT IS SOCIAL WEB?
   ******************************

   A remaley hash symmetric matrix + fairmath pity system (lerp & variants) model.

   Uses pairwise matrices (remaley hash) for both an undirected graph
   and to store two-axis data, and lerp as the resolution structure
   (fairmath). Fairmath is stored as a _percent_ uses base, t, impact
   (at either extremes, 1% and 99% is bounded and any changes are very
   impactful and tries move you to the middle, 50%). 0% and 100% are
   thresholds that can only be set manually.

   COULDDO(chowie): Save last bit of fairmath? 7-bits data, 1-bit b32?

   RESOURCE(jon ingold): https://medium.com/@inklestudios/changable-minds-d5d434772462
   Character -> Belief -> Event

   RESOURCE(emily short): https://emshort.blog/how-to-play/writing-if/my-articles/conversation/
   IMPORTANT(chowie): Trying an emotive conversational model (rather
   than verbal). Seek a series of quips that represents to an emotive
   movement to lead to the desired emotional outcome.
*/

// IMPORTANT(chowie): This includes you (ID 1), NPCs should always
// treat you more than equally (but everyone else equally). So it
// doesn't seem like you're being ignored.
struct socialweb_node_id
{
    u8 Value;
};

// NOTE(chowie): CAF framework for proficiency
struct lang_profiency
{
    u8 FairmathComplexity; // Range of vocab (lexical) and sentence structures (syntactic)
    u8 FairmathAccuracy; // How many mistakes one makes (and corrects themselves) e.g. Pentiment game rewrites text spelling/grammar
    u8 FairmathFluency; // Articulation rate and pauses (for writing, flow and connection of ideas)
    u8 FairmathAcquisitionRate;
};

// TODO(chowie): Hash this with PCGHash(SourceID)
// NOTE(chowie): Separate ID compared to entity_id and entity_ref
struct socialweb_node
{
    // NOTE(chowie): Graph
    string ConlangName; // NOTE(chowie): Grab entity id's name, stable serialisation/search criteria
    socialweb_node_id SourceID; // NOTE(chowie): Converts conlang name to a smaller ID
    u8 FairmathMentalStability; // How stable am I as a person?
    u8 FairmathSociallyImportant; // How important are you to your connections?

    // COULDDO(chowie): Put this metadata in entity struct?
    // NOTE(chowie): Matrix metadata - used to affect edge matrices
    // Mostly static
    u16 AnimalOrigin;
    u8 AnimalOrder;
    u8 RemaleyPersonality;

    // Dynamic
    u16 RemaleyOccupation; // Up to 2 occupations (dual occupations), day-night occupations (auto-assigned)
    u8 RemaleyMoodAlone; // What emotion do I feel when I'm alone currently?
    u8 FairmathFatigue; // Builds up from events, wanting to talk etc..
    f32 Age;
    lang_profiency LangProficency;
};

struct socialweb_edge_id
{
    u16 Value;
};

// NOTE(chowie): u16 RemaleyHashes gets rounded down from u32.
struct socialweb_edge
{
    // NOTE(chowie): Graph
    socialweb_edge_id RemaleyID;
    u8 FairmathRelationshipStability; // How stable is our relationship i.e. cut each other off?
    u8 FairmathSociallyImportant;

    // NOTE(chowie): Matrix metadata - sort criteria
    // COULDDO(chowie): Can use criteria as edge weights e.g. FAVVE relationship = number
    // Mostly static
    u16 RemaleyAnimalOrigin; // How do you address the other where they're from?
    u8 RemaleyAnimalOrder; // Who are you?
    u8 FairmathPadding0_;

    // Dynamic
    u16 RemaleyCommunity; // Who do you align with?
    u8 RemaleyFAVVE; // What type is our relationship?
    u8 FairmathCommFreq; // How regularly do we speak/interact?
    u8 RemaleyMood; // How do we emotionally feel about each other?
    u8 FairmathMood; // NOTE(chowie): Fairmath caps at 1% & 99% = "on the brink" status, manually set 0% & 100% to show completion
    u8 RemaleyRisk; // Belief: What type of risk from an event would it put on our relationship?
    u8 FairmathRisk; // Belief: Would what I'll do risk/strain our relationship?
};

// NOTE(chowie): This doesn't run frequently and doesn't need that much optimisation
// COULDDO(chowie): Use edge's sort criteria e.g. FAVVE
// NOTE(chowie): IMPORTANT(chowie): To reference certain characters in conversation
struct socialweb_degreeofseparation
{
    socialweb_node_id Source;
    socialweb_node_id Dest;

    u32 SearchDist; // Default 3
    b32x IsClose;
};

struct socialweb_node_hash
{
    socialweb_node *Ptr;
};
struct socialweb_edge_hash
{
    socialweb_edge *Ptr;
};
struct socialweb
{
    u32 MaxNodeCount;
    u32 NodeCount;
    socialweb_node *Nodes;

    u32 MaxEdgeCount;
    u32 EdgeCount;
    socialweb_edge *Edges;

    // STUDY(chowie): These hashes do have an upfront cost (the
    // clearing) if done every frame. Prevents the max number from
    // being arbitrary. But can implement incremental clearing.
    socialweb_node_hash NodeInternalChainHash[64]; // Triangle number would be 64 max
    socialweb_edge_hash EdgeInternalChainHash[2048]; // Triangle number edge would be 2016 max

    // NOTE(chowie): Bitfields avoids clearing out the entire hash
    u64 NodeInternalChainHashOccupancy[64/64];
    u64 EdgeInternalChainHashOccupancy[2048/64];
};

internal void
MarkBit(u64 *Array, umm Index)
{
    umm OccIndex = Index / 64;
    umm BitIndex = Index % 64;
    Array[OccIndex] |= ((u64)1 << BitIndex); // STUDY(chowie): By default bitshift always assumes 32-bit, use ULL or cast with (u64)
}

internal b32x
IsEmpty(u64 *Array, umm Index)
{
    umm OccIndex = Index / 64;
    umm BitIndex = Index % 64;
    b32x Result = !(Array[OccIndex] & ((u64)1 << BitIndex)); // STUDY(chowie): By default bitshift always assumes 32-bit, use ULL or cast with (u64)
    // NOTE(chowie): Alternatively write this as "Array[OccIndex] &= ~((u64)1 << BitIndex);"
    return(Result);
}

internal void
MarkOccupied(socialweb *SocialWeb, socialweb_node_hash *Entry)
{
    umm Index = Entry - SocialWeb->NodeInternalChainHash;
    MarkBit(SocialWeb->NodeInternalChainHashOccupancy, Index);
}

internal void
MarkOccupied(socialweb *SocialWeb, socialweb_edge_hash *Entry)
{
    umm Index = Entry - SocialWeb->EdgeInternalChainHash;
    MarkBit(SocialWeb->EdgeInternalChainHashOccupancy, Index);
}

internal socialweb_node_hash *
GetHashFromID(socialweb *SocialWeb, socialweb_node_id SourceID)
{
    socialweb_node_hash *Result = 0;

    u32 HashValue = (u32)SourceID.Value;
    for(u32 Offset = 0;
        Offset < ArrayCount(SocialWeb->NodeInternalChainHash);
        ++Offset)
    {
        u32 HashSlot = ((HashValue + Offset) & ArrayCount(SocialWeb->NodeInternalChainHash) - 1);
        socialweb_node_hash *Entry = SocialWeb->NodeInternalChainHash + HashSlot;
        if(IsEmpty(SocialWeb->NodeInternalChainHashOccupancy, HashSlot))
        {
            Result = Entry;
            Result->Ptr = 0;
            break;
        }
        else if(Entry->Ptr->SourceID.Value == SourceID.Value)
        {
            Result = Entry;
            break;
        }
    }
    Assert(Result);

    return(Result);
}

internal socialweb_edge_hash *
GetHashFromID(socialweb *SocialWeb, socialweb_edge_id RemaleyID)
{
    socialweb_edge_hash *Result = 0;

    // NOTE(chowie): Perfect hash value, never hash collisions, no chaining needed
    u32 HashSlot = RemaleyID.Value; // NOTE(chowie): RemaleyID also acts as hash value
    socialweb_edge_hash *Entry = SocialWeb->EdgeInternalChainHash + HashSlot;
    if(IsEmpty(SocialWeb->EdgeInternalChainHashOccupancy, HashSlot))
    {
        Result = Entry;
        Result->Ptr = 0;
    }
    else if(Entry->Ptr->RemaleyID.Value == RemaleyID.Value)
    {
        Result = Entry;
    }
    Assert(Result);

    return(Result);
}

internal socialweb_node *
GetNodeByID(socialweb *SocialWeb, socialweb_node_id SourceID)
{
    socialweb_node_hash *Entry = GetHashFromID(SocialWeb, SourceID);
    socialweb_node *Result = Entry ? Entry->Ptr : 0;
    return(Result);
}

internal socialweb_edge *
GetEdgeByID(socialweb *SocialWeb, socialweb_edge_id RemaleyID)
{
    socialweb_edge_hash *Entry = GetHashFromID(SocialWeb, RemaleyID);
    socialweb_edge *Result = Entry ? Entry->Ptr : 0;
    return(Result);
}

internal void
AddNodeToHash(socialweb *SocialWeb, socialweb_node *Node)
{
    socialweb_node_hash *Entry = GetHashFromID(SocialWeb, Node->SourceID);
    Assert(IsEmpty(SocialWeb->NodeInternalChainHashOccupancy, Entry - SocialWeb->NodeInternalChainHash));
    // NOTE(chowie): Either nothing in the hash slot before, which we
    // were trying to set, or it's equal to the one we are trying to
    // put in. Thus, can overset hash slots.
    Entry->Ptr = Node;
    MarkOccupied(SocialWeb, Entry);
}

internal void
AddEdgeToHash(socialweb *SocialWeb, socialweb_edge *Edge)
{
    socialweb_edge_hash *Entry = GetHashFromID(SocialWeb, Edge->RemaleyID);
    Assert(IsEmpty(SocialWeb->EdgeInternalChainHashOccupancy, Entry - SocialWeb->EdgeInternalChainHash));
    // NOTE(chowie): Either nothing in the hash slot before, which we
    // were trying to set, or it's equal to the one we are trying to
    // put in. Thus, can overset hash slots.
    Entry->Ptr = Edge;
    MarkOccupied(SocialWeb, Entry);
}

internal socialweb *
BeginSocialWeb(memory_arena *SocialWebArena)
{
    // IMPORTANT(chowie): When you pass no clear into pushstruct or
    // pusharray, you must _initialise everything_ and look
    // side-by-side of the struct. However, we cannot do this as the
    // hashes must be cleared else we cannot use them for lookups!
    socialweb *SocialWeb = PushStruct(SocialWebArena, socialweb, NoClear());

    ZeroStruct(SocialWeb->NodeInternalChainHashOccupancy);
    ZeroStruct(SocialWeb->EdgeInternalChainHashOccupancy);

    SocialWeb->MaxNodeCount = 64;
    SocialWeb->NodeCount = 0;
    SocialWeb->Nodes = PushArray(SocialWebArena, socialweb_node, SocialWeb->MaxNodeCount, NoClear());

    SocialWeb->MaxEdgeCount = 2048;
    SocialWeb->EdgeCount = 0;
    SocialWeb->Edges = PushArray(SocialWebArena, socialweb_edge, SocialWeb->MaxEdgeCount, NoClear());

    //
}

internal f32
SocialWebSparsity(u32 EdgeCount, u32 MaxNodeCount)
{
    f32 Result = 1.0f - (f32)(EdgeCount/TriangleNumber(MaxNodeCount));
    return(Result);
}

internal void
SocialWebAddNode(u16 Node)
{
    // Push to hash table? / array
}

internal void
SocialWebAddEdge(u16 NodeA, u16 NodeB)
{
    RemaleyHash(NodeA, NodeB);

    // Push to hash table? / array
}

internal void
SocialWebAddEdge(u32 Edge)
{
    // Push to hash table? / array
}

// IMPORTANT(chowie): TODO(chowie): +1 because you don't want the null node
// COULDDO(chowie): Does it make sense to add a random node?
// COULDDO(chowie): A function to validate that this is the kind of
// edge you want to add?

// RESOURCE(): https://www.johndcook.com/blog/2025/09/11/random-inside-triangle/
// NOTE(chowie): Accept-flip model (variation on accept-reject).
// Because this method uses RemaleyHash, creates a square, any points
// outside of bound of square will always fall within the triangle.
internal void
SocialWebAddRandomEdge(pcg32_random_series *Series, u32 NodeMax)
{
    for(;;)
    {
        // NOTE(chowie): This 1D sampling works because of how
        // RemaleyHash is mapped to a triangle!
        u32 SampleEdge = RandomBounds(Series, NodeMax);
        if(!SampleEdge) // 
        {
            // Push to hash table? / array

            break;
        }
    }
}

//
// #
// # #
// # # #
// # # # #_________ Sampling rect
// # # # #|#      |
// # # # #|# #    |
// # # # #|# # #  |
// # # # #|# # # #|
// TODO(chowie): Double-check this is all correct
inline u32
SampleRemaleyHash(pcg32_random_series *Series, rect2i Rect)
{
    u32 RowOffset = GetPairwiseRow32((u32)Rect.Min.x);
    u32 Bounds = TriangleNumber((u32)GetWidth(Rect));

    // NOTE(chowie): This 1D sampling works because of how
    // RemaleyHash is mapped to a triangle!
    u32 SampleEdge = RandomBounds(Series, Bounds);
    u32 Result = RowOffset + SampleEdge; // Sampled Triangle

    return(Result);
}

// RectMinDim(v2s Min, v2s Dim);
internal void
SocialWebAddRandomEdge(pcg32_random_series *Series, rect2i Rect)
{
    for(;;)
    {
        u32 SampleEdge = SampleRemaleyHash(Series, Rect);
        if(!SampleEdge) //
        {
            // Push to hash table? / array

            break;
        }
    }
}

internal void
SocialWebAddRandomEdge(pcg32_random_series *Series, v2s Min, v2s Dim)
{
    SocialWebAddRandomEdge(Series, RectMinDim(Min, Dim));
}

internal void
SocialWebAddRandomEdge(pcg32_random_series *Series, u16 NodeA, u32 NodeMax)
{
    for(;;)
    {
        u32 SampleEdge = RemaleyHash(NodeA, (u16)RandomBounds(Series, NodeMax));
        if(!SampleEdge) // 
        {
            // Push to hash table? / array

            break;
        }
    }

    // Push to hash table? / array
}

internal void
SocialWebAddRandomEdge(pcg32_random_series *Series, u32 Edge, u32 NodeMax)
{
    // Push to hash table? / array
}

// NOTE(chowie): Deletes related edges too
internal void
SocialWebRemoveNode(u16 Node)
{
    for(u32 PairIndex = 0;
        PairIndex < Inhabitant_Name_Count;
        PairIndex++)
    {
        RemaleyHash(Node, (u16)PairIndex);
        // Finds all matrix edges
    }
}

internal void
SocialWebRemoveEdge(u16 NodeA, u16 NodeB)
{
    RemaleyHash(NodeA, NodeB);

    // Push to hash table? / array
}

internal b32x
SocialWebHasEdge(u16 NodeA, u16 NodeB)
{
    b32x Result = false;
    for(;;)
    {
        u32 TestEdge = RemaleyHash(NodeA, NodeB);
        if(TestEdge)
        {
            Result = true;
            break;
        }
    }

    return(Result);
}

internal b32x
SocialWebHasEdge(u32 Edge)
{
    b32x Result = false;
    for(;;)
    {
        u32 TestEdge = Edge;
        if(TestEdge)
        {
            Result = true;
            break;
        }
    }

    return(Result);
}

// NOTE(chowie): O(n) currently
internal void
SocialWebGetAllNeighboursAndDegree(u16 Node)
{
}

// NOTE(chowie): O(n) currently
// NOTE(chowie): For a relationship, find all neighbours for
// both. E.g. lovers might want to sever other relationships?
internal void
SocialWebGetAllNeighboursAndDegree(u16 NodeA, u16 NodeB)
{
}

// NOTE(chowie): O(n) currently
internal void
SocialWebGetAllNeighboursAndDegree(u32 Edge)
{
}

enum remaleyhashsampling_type
{
    RemaleyHashSampling_Type_All,
    RemaleyHashSampling_Type_Row,
    RemaleyHashSampling_Type_Edge,
};
// RESOURCE(): https://www.drmaciver.com/2017/05/a-hybrid-voting-system-for-scheduling/
internal void
SocialWebVoting(pcg32_random_series *Series, u32 Index, u32 Length)
{
    // 1. Select sampling type
//    u32 SampleEdge = RandomBounds(Series, NodeMax);

    // 2. Select sampling type
    Permute(Series, Index, Length);
}

//
// Fink Hash (3->1 rolling perfect) for non-pairwise
//

/********************************
         WHAT IS FINK HASH?
   ******************************

   3->1 rolling perfect hash
   Store all tree data within hash, and used for IDs too
   Input order dependent (non-pairwise)

   An implicit binary tree (Merkle tree) packed into a u64. Makes a 2x2x2 voxel block.

   When reading a fink hash tree like a k-d tree, it guarantees:
   - If it's odd = tree (2x2x2)
   - If it's even = leaf (1x1x1)

         z      --- odd
        / \
       y   y    --- odd
      / \ / \
      x x x x   --- even

   IMPORTANT(chowie): Fink hash trees have complete branch
   optimisation. They're automatically pruned into a leaf (even) if
   they all share the same value. This is checked with a sqrt() to
   determine if the value is a perfect square.

   Appropriate places to utilise this:
   - For pathfinding: Entities should prioritise even (1x1x1) (greater
   control for players), odd for stairs/half-slabs
   - For meshing: Even (1x1x1) is a whole voxel cube mesh, odd is for complex meshes
   e.g. stairs, corner-stairs, half, checkered

   When unpacking a fink hash tree
   - Interpret leaves via block_id (enum) and texture atlas probably

   When packing a fink hash tree, for now it only supports changing
   existing values (no add/delete nodes yet). Either by index or first
   instance of said block_id.

   When serialising a fink hash tree, copy the u64.

   When validating the integrity of a fink hash tree, compare u64 source vs dest.
   Use for networking and any VCS.

   // TODO(chowie): IMPORTANT(chowie): LRU (Least Recently Used) to track voxel data probably
*/

// TODO(chowie): Move this out!
// TODO(chowie): Build specific? Does it share block ids?
// IMPORTANT(chowie): Primitives (child nodes) must be even values!
#define EvenSet(EnumValue) ((2*EnumValue))
enum block_id : u16
{
    // ----------- FIRST 38 (including 0) 1/8th BLOCKS (and can be 1m BLOCKS) ---------
    // OR "comb blocks" that can tile/blend different materials together
    // IMPORTANT(chowie): General blocks/colour palette useful in any build,
    // only can face one direction.

    Block_Air        = EvenSet(0),
    Block_Water      = EvenSet(1),
    Block_Lava       = EvenSet(2),
    Block_Ice        = EvenSet(3),
    Block_Smoke      = EvenSet(4),
    Block_InvisWall  = EvenSet(5),

    // ----------- DO NOT REORDER THE ABOVE (SET PRECOMPUTED VALUES) ---------

    Block_Sod        = EvenSet(6), // NOTE(chowie): More accurate than grass block
    Block_Dirt       = EvenSet(7),
    Block_Snow       = EvenSet(8),
    Block_Sand       = EvenSet(9),
    Block_Mud        = EvenSet(10), // NOTE(chowie): Ancient architecture uses it
    Block_Log        = EvenSet(11),
    Block_Planks     = EvenSet(12),
    Block_BundledBamboo = EvenSet(13),
    Block_Paper      = EvenSet(14), // NOTE(chowie): Asian architecture
    Block_Thatch     = EvenSet(15),
    Block_Leaves     = EvenSet(16),
    Block_Cobblestone = EvenSet(17),
    Block_Limestone  = EvenSet(18),
    Block_Chukum     = EvenSet(19), // NOTE(chowie): Like Stucco
    Block_Marble     = EvenSet(20),
    Block_Basalt     = EvenSet(21),
    Block_ResinBrick = EvenSet(22),
    Block_Purpur     = EvenSet(23),
    Block_Glass      = EvenSet(24), // COULDDO(chowie): Add acrylic? Alternative to glass, bendable
    Block_Polycarbonate = EvenSet(25), // NOTE(chowie): Alternative to glass, Greenhouse/office/roofs
    Block_Brass      = EvenSet(26),
    Block_Bronze     = EvenSet(27),
    Block_Aluminum   = EvenSet(28),
    Block_CorrugatedMetal = EvenSet(29),
    Block_Steel      = EvenSet(30),
    Block_Cage       = EvenSet(31), // NOTE(chowie): Doubles as a chain
    Block_Cork       = EvenSet(32),
    Block_Bone       = EvenSet(33),
    Block_Ceramic    = EvenSet(34),
    // NOTE(chowie): Small decorative blocks
    Block_Vines      = EvenSet(35),
    Block_Grass      = EvenSet(36),
    Block_Flower     = EvenSet(37), // NOTE(chowie): Random colours based on block location

    // ----------- ONLY FULL 1m BLOCKS BELOW (decorative, simulation, interactive) ---------
    // IMPORTANT(chowie): Pathfinding prioritises 1m blocks

    Block_Fire       = EvenSet(38),
    Block_FloodLight = EvenSet(39),
    Block_Reed       = EvenSet(40),
    Block_Lilypad    = EvenSet(41),
    Block_BundledCarrots = EvenSet(42),
    Block_BundledPotatoes = EvenSet(43),
    Block_BundledMushrooms = EvenSet(44),
    Block_Crystal    = EvenSet(45),
    Block_Geyser     = EvenSet(46),
    Block_Cobweb     = EvenSet(47),

    Block_Marker,
    Block_Count = (Block_Marker + 1)/2,
};

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
// NOTE(chowie): This will never cause an integer overflow because the
// tree must be odd (so at least 1)!
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
    u16 Leaves[FINKHASHTREE_MAXLEAVES]; // TODO(chowie): Make this a u8!
//    u64 StackCount;
//    stack_entry *Stack; // COULDDO(chowie): Stack[StackCount] OR Stack[(MaxDepth*2 + 1)]
    // NOTE(onix242): Though I may change my mind later, I'm explicitly storing the leaves because
    // I think I'm specifically looking for a list of the nodes touched, but you could probably
    // remove it when optimising.
};

struct finkhashtree_cardinal_group
{
    finkhashtree Trees[6];
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

// NOTE(chowie): Tries to get a non-reserved/non-special block when
// possible, easiest is to get the top/bot.
// COULDDO(chowie): If top/bot isn't enough, to get the sides try
// sampling leaf index.
internal u8
GetFinkHashTreeLODLeafApprox(u64 Hash, b32x TopFaceBlock)
{
    u64 Acc = TopFaceBlock ? InvertFinkHash(Hash).a : InvertFinkHash(Hash).b;
    for(;
        IsParent(Acc);
        )
    {
        v2u64 Pair = InvertFinkHash(Acc);
        Acc = Max(Pair.a, Pair.b);
    }

    return(SafeTruncateToU8(Acc));
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

// TODO(chowie): Do the same for block_air?
// COULDDO(chowie): Could take use .destleaf to save on a for loop?
internal u64
FinkHashTreeBubbleupCommonLeafInPerfectTreeIfPossibleInternal(u64 Hash)
{
    u64 Result = Hash;
    // NOTE(chowie): The most-likely block ID to be filled in the tree
#define PRECOMPUTED_BLOCKAIRHASH 361 // NOTE(chowie): f(0) or 0
#define PRECOMPUTED_BLOCKWATERHASH 27071209 // NOTE(chowie): f(1) or 2
#define PRECOMPUTED_BLOCKLAVAHASH 2823753321
#define PRECOMPUTED_BLOCKICEHASH 52827804649
#define PRECOMPUTED_BLOCKSMOKEHASH 449547453289
#define PRECOMPUTED_BLOCKINVISWALLHASH 2431662865641
    if(Result == PRECOMPUTED_BLOCKAIRHASH)
    {
        Result = (u64)Block_Air;
    }
    else if(Result == PRECOMPUTED_BLOCKWATERHASH)
    {
        Result = (u64)Block_Water;
    }
    else if(Result == PRECOMPUTED_BLOCKLAVAHASH)
    {
        Result = (u64)Block_Lava;
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

// NOTE(chowie): For water/sand/mud with a "moving = still/calm" state while keeping
// the eighth blocks intact (no bubble up)
// if(CommonLeaf == (u64)Block_Water) // NOTE(chowie): Do not recombine for particle-related/fluid sim stuff
// NOTE(chowie): Complete branch optimisation, e.g. Breaking the last block, recombines all air into an air block
internal u64
FinkHashTreeBubbleupCommonLeafInPerfectTreeIfPossible(u64 Hash)
{
    u64 Result = Hash;
    if(IsPerfectSqr(Result))
    {
        Result = FinkHashTreeBubbleupCommonLeafInPerfectTreeIfPossibleInternal(Result);
    }

    return(Result);
}

// e.g.  x  ->   z
//              / \
//             y   y
//            / \ / \
//            x x x x
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
// Fink hash cardinal adjacent/neighbour voxels
//

// RESOURCE(): https://geidav.wordpress.com/2017/12/02/advanced-octrees-4-finding-neighbor-nodes/
// NOTE(chowie): Because fink hash's max depth is 2x2x2, the 3rd case (different level,
// different parent; subtree of neighbour deeper) doesn't need to be handled
// Case 1: Same level, same parent & Same level, different parent
//   ____________________
//   |        |         |
//   |        |         |
//   |        |         |
//   |        |         |
//   |________|_________|
//   | / / / /|         |
//   |/ / / / |         |
//   | / / / /|         |
//   |/ / / / |         |
//   |_/_/_/_/|_________|
//   ____________________
//   | / / / /|         |
//   |/ / / / |         |
//   | / / / /|         |
//   |/ / / / |         |
//   |_/_/_/_/|_________|
//   |        |         |
//   |        |         |
//   |        |         |
//   |        |         |
//   |________|_________|

// Case 2: Different level, different parent
//   ____________________
//   | / / / /| / / / / |
//   |/ / / / |/ / / / /|
//   | / / / /| / / / / |
//   |/ / / / |/ / / / /|
//   |_/_/_/_/|_/_/_/_/_|
//   | / / / /| / / / / |
//   |/ / / / |/ / / / /|
//   | / / / /| / / / / |
//   |/ / / / |/ / / / /|
//   |_/_/_/_/|_/_/_/_/_|
//   ____________________
//   | / / / /|         |
//   |/ / / / |         |
//   | / / / /|         |
//   |/ / / / |         |
//   |_/_/_/_/|_________|
//   |        |         |
//   |        |         |
//   |        |         |
//   |        |         |
//   |________|_________|

// NOTE(chowie): Neighbour nodes should be the same on neighbour parents i.e.
// if 0 was the source leaf node, a neighbour node is 2. The north parent
// (fink hash tree) should also be 2!
//        ____________________
//       /         /         /|
//      /    0    /   1     / |
//     /_________/_________/  |
//    /         /         /|  |
//   /    2    /    3    / |  |
//  /_________/_________/  | /|
//  |         |         |  |/ |
//  |         |         |  / 5|
//  |         |         | /|  /
//  |_________|_________|/ | /
//  |         |         |  |/
//  |    6    |    7    |  /
//  |         |         | /
//  |_________|_________|/
//
// | Z-order | Z-order [.x, .y, .z] | World space [.x, .y, .z] |
// | 0       | 1, 2, 4              | -1,  1,  1               |
// | 1       | 0, 3, 5              |  1,  1,  1               |
// | 2       | 0, 3, 6              | -1, -1,  1               |
// | 3       | 1, 2, 7              |  1, -1,  1               |
// | 4       | 5, 6, 0              | -1,  1, -1               |
// | 5       | 4, 7, 1              |  1,  1, -1               |
// | 6       | 4, 7, 2              | -1, -1, -1               |
// | 7       | 5, 6, 3              | -1, -1, -1               |
//
// STUDY(chowie): Replaces typical mortons encoding stored in voxel octree
// RESOURCE(): https://fgiesen.wordpress.com/2015/02/22/triangular-numbers-mod-2n/
// NOTE(chowie): Tk = TriangleNumber(n). I've used modulo triangle numbers to
// "biject"/fit the sequence by the source leaf node .x, .y and .z
// k      | 0   1   2   3   4   5   6   7   8   9   10  11
// Tk     | 0   1   3   6   10  15  21  28  36  45  55  66
// Tk % 2 | 0   1   1   0   0   1   1   0   0   1   1   0
//
// Reconstructing Patterns:
// Z-order .x = (Tk + 2) % 2 | unless >= 4, Result + 4
// Z-order .y = 2 + (Tk) % 2 | unless >= 4, Result + 4
// Z-order .z = (Result + 4) % 8
// World space .x = Odd()
// World space .y = (Tk + 1) % 2
// World space .z = >= 4

// TODO(chowie): Rename to not use internal to not conflict with grep!
// IMPORTANT(chowie): Parents = world space directions -> select children based
// on respective .x, .y, .z. Unless neighbour only has a root node, iseven(finkhash)
// IMPORTANT(chowie): To convert values from NeighbourChildren to world space
// by inverting sign bit of NeighbourParents, applies vice versa
struct leaf_neighbour_dir_result 
{
    v3u InternalNeighbours; // NOTE(chowie): In z-order curve
    v3s ExternalNeighbours;  // NOTE(chowie): Parents, in world space
};
#define FINKHASHTREE_MAXLEAVESPITCH 4
internal leaf_neighbour_dir_result
FinkHashTreeGetLeafNodeNeighbourDir(u32 SourceLeafIndex)
{
    leaf_neighbour_dir_result Result = {};

    Result.InternalNeighbours =
    {
        (TriangleNumber(SourceLeafIndex + 2) % 2),
        (2 + (TriangleNumber(SourceLeafIndex) % 2)),
        ((SourceLeafIndex + FINKHASHTREE_MAXLEAVESPITCH) % FINKHASHTREE_MAXLEAVES),
    };
    Result.ExternalNeighbours = V3S(1);

    if(!Odd(SourceLeafIndex))
    {
        Result.ExternalNeighbours.x = -1;
    }

    s32 Offset = (TriangleNumber(SourceLeafIndex + 1) % 2);
    if(Offset == 0)
    {
        Result.ExternalNeighbours.y = -1;
    }

    if(SourceLeafIndex >= FINKHASHTREE_MAXLEAVESPITCH)
    {
        Result.InternalNeighbours.xy += V2U(FINKHASHTREE_MAXLEAVESPITCH, FINKHASHTREE_MAXLEAVESPITCH);
        Result.ExternalNeighbours.z = -1;
    }

    return(Result);
}

// NOTE(chowie): Best case, O(n^3)
internal void
FinkHashTreeGetRootNodeNeighbourDir(u32 SourceRootNode)
{
    // TODO(chowie): Add normal loop here!
}

struct cardinal_neighbours
{
    union
    {
        struct
        {
            // TODO(chowie): Change v4u to a v4u8 or another compressed form like for colours?
            v4u Leaf; // NOTE(chowie): .x, -.x, .y, -.y, .z, -.z,
            v4u ZOrder; // NOTE(chowie): .x, -.x, .y, -.y, .z, -.z,
        };
        struct
        {
            // TODO(chowie): Change u32 to a u8
            u32 InternalLeaf; // NOTE(chowie): .x, -.x, .y, -.y, .z, -.z,
            u32 InternalZOrder; // NOTE(chowie): .x, -.x, .y, -.y, .z, -.z,
        };
        struct
        {
            u64 Tree; // COULDDO(chowie): Remove union and save it out?
        };
    };

    b32 IsFullBlock;
};

// TODO(chowie): This also wants FinkHashTree + v3s MullerHash position to reference
// NOTE(chowie): Mirrors dir functions above
struct neighbour_result
{
    cardinal_neighbours Neighbours[6]; // NOTE(chowie): .x, -.x, .y, -.y, .z, -.z,
    b32 IsSourceNodeLeaf;
};

internal neighbour_result
FinkHashTreeGetLeafNodeNeighbour(finkhashtree_cardinal_group FinkHashTree, leaf_neighbour_dir_result NeighboursDir)
{
    neighbour_result Result = {};
    Result.IsSourceNodeLeaf = true;

    // NOTE(chowie): Internal = z-order
    for(u32 LeafCount = 0;
        LeafCount < ArrayCount(NeighboursDir.InternalNeighbours.E);
        ++LeafCount)
    {
        u32 NeighbourIndex = 0;
        u32 InvDir = -NeighboursDir.ExternalNeighbours.E[LeafCount];
        u32 ZOrderIndex = NeighboursDir.InternalNeighbours.E[LeafCount];

        if(InvDir == 1)
        {
            NeighbourIndex += 0;
        }
        else
        {
            NeighbourIndex += 1;
        }

        Result.Neighbours[NeighbourIndex].InternalZOrder = ZOrderIndex;
        Result.Neighbours[NeighbourIndex].InternalLeaf = FinkHashTree.Trees[NeighbourIndex].Leaves[ZOrderIndex];

        NeighbourIndex += 2;
    }

    // NOTE(chowie): External = world coords
    for(u32 TreeCount = 0;
        TreeCount < ArrayCount(NeighboursDir.ExternalNeighbours.E);
        ++TreeCount)
    {
        if(IsParent(FinkHashTree.Trees[TreeCount].FinkHash))
        {
            u32 NeighbourIndex = 0;
            u32 Dir = NeighboursDir.ExternalNeighbours.E[TreeCount];
            u32 ZOrderIndex = NeighboursDir.InternalNeighbours.E[TreeCount];

            if(Dir == 1)
            {
                NeighbourIndex += 0;
            }
            else
            {
                NeighbourIndex += 1;
            }

            Result.Neighbours[NeighbourIndex].InternalZOrder = ZOrderIndex;
            Result.Neighbours[NeighbourIndex].InternalLeaf = FinkHashTree.Trees[NeighbourIndex].Leaves[ZOrderIndex];

            NeighbourIndex += 2;
        }
        else
        {
            Result.Neighbours[TreeCount].IsFullBlock = true;
            Result.Neighbours[TreeCount].Tree = FinkHashTree.Trees[TreeCount].FinkHash;
        }
    }

    return(Result);
}

// COULDDO(chowie): Clean up the difference between leafcount vs leafindex?
// TODO(chowie): Test this rigorously!
internal neighbour_result
FinkHashTreeGetRootNodeNeighbour(finkhashtree_cardinal_group FinkHashTree)
{
    neighbour_result Result = {};
    Result.IsSourceNodeLeaf = false;

    for(u32 TreeCount = 0;
        TreeCount < ArrayCount(FinkHashTree.Trees);
        ++TreeCount)
    {
        // NOTE(chowie): FinkHashTrees that are odd = has
        // children. Otherwise, it only has a root.
        if(IsParent(FinkHashTree.Trees[TreeCount].FinkHash))
        {
            v2u LeafIndexX = {};
            v2u LeafIndexY = {};
            v2u LeafIndexZ = {};

            for(u32 LeafCount = 0;
                LeafCount < ArrayCount(FinkHashTree.Trees->Leaves);
                ++LeafCount)
            {
                u32 NeighbourIndex = 0;
                u32 FillLeaves = 0;

                if(!Odd(LeafCount)) // (Dir.x == 1)
                {
                    NeighbourIndex = 0;
                    FillLeaves = LeafIndexX.a++;
                }
                else
                {
                    NeighbourIndex = 1;
                    FillLeaves = LeafIndexX.b++;
                }
                Result.Neighbours[NeighbourIndex].Leaf.E[FillLeaves] = FinkHashTree.Trees[TreeCount].Leaves[LeafCount];
                Result.Neighbours[NeighbourIndex].ZOrder.E[FillLeaves] = LeafCount;

                s32 Offset = (TriangleNumber(LeafCount + 1) % 2);
                if(Offset == 0) // (Dir.y == 1)
                {
                    NeighbourIndex = 2;
                    FillLeaves = LeafIndexY.a++;
                }
                else
                {
                    NeighbourIndex = 3;
                    FillLeaves = LeafIndexY.b++;
                }
                Result.Neighbours[NeighbourIndex].Leaf.E[FillLeaves] = FinkHashTree.Trees[TreeCount].Leaves[LeafCount];
                Result.Neighbours[NeighbourIndex].ZOrder.E[FillLeaves] = LeafCount;

                if(LeafCount >= FINKHASHTREE_MAXLEAVESPITCH) // (Dir.z == 1)
                {
                    NeighbourIndex = 4;
                    FillLeaves = LeafIndexZ.a++;
                }
                else
                {
                    NeighbourIndex = 5;
                    FillLeaves = LeafIndexZ.b++;
                }
                Result.Neighbours[NeighbourIndex].Leaf.E[FillLeaves] = FinkHashTree.Trees[TreeCount].Leaves[LeafCount];
                Result.Neighbours[NeighbourIndex].ZOrder.E[FillLeaves] = LeafCount;
            }
        }
        else
        {
            Result.Neighbours[TreeCount].IsFullBlock = true;
            Result.Neighbours[TreeCount].Tree = FinkHashTree.Trees[TreeCount].FinkHash;
        }
    }

    return(Result);
}

//
// Fink hash rotate/reflect adjacent/neighbour voxels
//

// IMPORTANT(chowie): If using for voxels, doesn't swap the texture face!
// TODO(chowie): Support all rot axis, not just .z!
enum finkhashtree_rot
{
    FINKHASHTREE_ROT_Z_CLOCKWISE,
    FINKHASHTREE_ROT_Z_ANTICLOCKWISE,
    FINKHASHTREE_ROT_Z_REFLECT,
};

internal u32
FinkHashTreeGetLeafNodeNeighbourRot(u32 SourceLeafIndex, finkhashtree_rot Rot)
{
    u32 Result = 0;
    v3u InternalNeighbours =
    {
        (TriangleNumber(SourceLeafIndex + 2) % 2),
        (2 + (TriangleNumber(SourceLeafIndex) % 2)),
        ((SourceLeafIndex + FINKHASHTREE_MAXLEAVESPITCH) % FINKHASHTREE_MAXLEAVES),
    };

    switch(Rot)
    {
        case FINKHASHTREE_ROT_Z_CLOCKWISE:
        case FINKHASHTREE_ROT_Z_ANTICLOCKWISE:
        {
            if(Odd(SourceLeafIndex))
            {
                Result = InternalNeighbours.y;
            }
            else
            {
                Result = InternalNeighbours.x;
            }
        } break;

        case FINKHASHTREE_ROT_Z_REFLECT:
        {
            Result = InternalNeighbours.z;
        } break;

        InvalidDefaultCase;
    }

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

// RESOURCE(): http://www.0x80.pl/notesen/2013-11-23-integer-sequence-encoding.html
// TODO(chowie): Compression for variable-sized integers?
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
    finkhashtree_params TreeParams = FinkHashTreeReplaceParams(FinkHashTree_Mode_FirstOfValue, 16, 14, true);
    
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
