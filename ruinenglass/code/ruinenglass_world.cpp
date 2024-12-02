/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Michael Chow $
   $Notice: $
   ======================================================================== */

// TODO(chowie): Better safe margin?
// NOTE(chowie): World isn't spherical/toroidal; doesn't wrap
#define CHUNK_VALID_APRON (S32Max / 64)
#define CHUNK_UNINITIALISED S32Max

// TODO(chowie): Replace with rectcenterdim?
inline b32x
IsCannonical(r32 ChunkDim, r32 TileRel)
{
    b32x Result = ((TileRel >= -(0.5f*ChunkDim + Epsilon32)) &&
                   (TileRel <= (0.5f*ChunkDim + Epsilon32)));
    return(Result);
}

inline b32x
IsCannonical(world *World, v3 Offset)
{
    b32x Result = (IsCannonical(World->ChunkDimInMeters.x, Offset.x) &&
                   IsCannonical(World->ChunkDimInMeters.y, Offset.y) &&
                   IsCannonical(World->ChunkDimInMeters.z, Offset.z));
    return(Result);
}

internal void
InitialiseWorld(world *World, v3 ChunkDimInMeters)
{
    World->ChunkDimInMeters = ChunkDimInMeters;

    for(u32 ChunkIndex = 0;
        ChunkIndex < ArrayCount(World->ChunkHash);
        ++ChunkIndex)
    {
        World->ChunkHash[ChunkIndex].Chunk.x = CHUNK_UNINITIALISED;
    }
}

// STUDY(chowie): Create a new chunk if passed an arena
internal world_chunk *
GetWorldChunk(world *World, v3s ChunkValue, memory_arena *Arena = 0)
{
    Assert(ChunkValue.x > -CHUNK_VALID_APRON);
    Assert(ChunkValue.y > -CHUNK_VALID_APRON);
    Assert(ChunkValue.z > -CHUNK_VALID_APRON);
    Assert(ChunkValue.x < CHUNK_VALID_APRON);
    Assert(ChunkValue.y < CHUNK_VALID_APRON);
    Assert(ChunkValue.z < CHUNK_VALID_APRON);

    // TODO(chowie): Better hash function
    u32 HashValue = MullerHash(ChunkValue);
    u32 HashSlot = HashValue & (ArrayCount(World->ChunkHash) - 1); // NOTE(chowie): Wrapping number that fits in array size
    Assert(HashSlot < ArrayCount(World->ChunkHash));

    world_chunk *Chunk = World->ChunkHash + HashSlot;
    do
    {
        // NOTE(chowie): Look through chain via coordinate; found matched chunk
        if((ChunkValue.x == Chunk->Chunk.x) &&
           (ChunkValue.y == Chunk->Chunk.y) &&
           (ChunkValue.z == Chunk->Chunk.z))
        {
            break;
        }

        // NOTE(chowie): Chunk in apron without one in the chain
        // (about to run off linked list), and need a new chunk
        if(Arena && (Chunk->Chunk.x != CHUNK_UNINITIALISED) && (!Chunk->NextInHash))
        {
            // NOTE(chowie): Make a new chunk
            Chunk->NextInHash = PushStruct(Arena, world_chunk);
            Chunk = Chunk->NextInHash;
            Chunk->Chunk.x = CHUNK_UNINITIALISED; // STUDY(chowie): Know to move to end of list
        }

        // NOTE(chowie): On an empty slot, or just made an empty slot for ourselves
        if(Arena && (Chunk->Chunk.x == CHUNK_UNINITIALISED))
        {
            Chunk->Chunk = ChunkValue;
            Chunk->NextInHash = 0; // NOTE(chowie): Nothing after list
            break; // TODO(chowie): Multiple chunks with same X,Y,Z; iterator to run through all as a chunk?
        }

        // NOTE(chowie): Traverse hash if no chunk is found
        Chunk = Chunk->NextInHash;
    } while(Chunk);
    // STUDY(chowie): Remember do/while checks at least once!

    return(Chunk);
}

// STUDY(chowie): Rounds to nearest tile from 0.5f. To then add
// however many tiles that was to wherever the tile is. Add the
// relative amount and subtract how much it went.
// _____________________
// |   0.5   ||         |
// |    *---->--------->
// |         ||         |
// ---------------------
//           >0.5      >1.0
// We would like to add .5, then divide. If it was .5, it would round
// to 1. Then do a standard truncation towards 0.
inline void
RecanonicaliseCoord(r32 ChunkDim, s32 *Tile, r32 *TileRel)
{
    s32 Offset = RoundR32ToS32(*TileRel / ChunkDim); // NOTE: Negative offset after round, ChunkDim +tive, TileRel -tive;
    *Tile += Offset; // NOTE: Delta is treated, here Bottom-left corner of a tile, tile-relative
    *TileRel -= (r32)Offset*ChunkDim;
    Assert(IsCannonical(ChunkDim, *TileRel));
}

// TODO(chowie): Remove offset and keep the 3 indices (only store in chunk)
// NOTE(chowie): Offset is a way of moving things around
inline world_position
MapIntoChunkSpace(world *World, world_position BasePos, v3 Offset)
{
    world_position Result = BasePos;

    // NOTE(chowie): Treat position as if it was updated
    Result.Offset_ += Offset;
    RecanonicaliseCoord(World->ChunkDimInMeters.x, &Result.Chunk.x, &Result.Offset_.x);
    RecanonicaliseCoord(World->ChunkDimInMeters.y, &Result.Chunk.y, &Result.Offset_.y);
    RecanonicaliseCoord(World->ChunkDimInMeters.z, &Result.Chunk.z, &Result.Offset_.z);

    return(Result);
}

