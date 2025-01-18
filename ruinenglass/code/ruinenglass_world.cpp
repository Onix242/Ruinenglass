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

inline world_pos
NullPosition(void)
{
    world_pos Result = {};
    Result.Chunk.x = CHUNK_UNINITIALISED;
    return(Result);
}

// TODO(chowie): Replace with rectcenterdim?
inline b32x
IsCanonical(r32 ChunkDim, r32 TileRel)
{
    b32x Result = ((TileRel >= -(0.5f*ChunkDim + Epsilon32)) &&
                   (TileRel <= (0.5f*ChunkDim + Epsilon32)));
    return(Result);
}

inline b32x
IsCanonical(world *World, v3 Offset)
{
    b32x Result = (IsCanonical(World->ChunkDimInMeters.x, Offset.x) &&
                   IsCanonical(World->ChunkDimInMeters.y, Offset.y) &&
                   IsCanonical(World->ChunkDimInMeters.z, Offset.z));
    return(Result);
}

inline b32x
AreInSameChunk(world *World, world_pos *A, world_pos *B)
{
    Assert(IsCanonical(World, A->Offset_));
    Assert(IsCanonical(World, B->Offset_));

    b32x Result = ((A->Chunk.x == B->Chunk.x) &&
                   (A->Chunk.y == B->Chunk.y) &&
                   (A->Chunk.z == B->Chunk.z));
    return(Result);
}

internal void
InitialiseWorld(world *World, v3 ChunkDimInMeters)
{
    World->ChunkDimInMeters = ChunkDimInMeters;
    World->FirstFree = 0;

    for(u32 ChunkIndex = 0;
        ChunkIndex < ArrayCount(World->ChunkHash);
        ++ChunkIndex)
    {
        World->ChunkHash[ChunkIndex].Chunk.x = CHUNK_UNINITIALISED;
        // NOTE(chowie): Memory precaution for additional runs of the
        // game, incase they weren't zero init.
        World->ChunkHash[ChunkIndex].FirstBlock.EntityCount = 0;
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

    // TODO(chowie): Better hash function?
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
// |         ||         |
// |         ||         |
// |   0.5   ||         |
// |    *---->--------->
// |         ||         |
// |         ||         |
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
    Assert(IsCanonical(ChunkDim, *TileRel));
}

// TODO(chowie): Remove offset and keep the 3 indices (only store in chunk)
// NOTE(chowie): Offset is a way of moving things around
inline world_pos
MapIntoChunkSpace(world *World, world_pos BasePos, v3 Offset)
{
    world_pos Result = BasePos;

    // NOTE(chowie): Treat position as if it was updated
    Result.Offset_ += Offset;
    RecanonicaliseCoord(World->ChunkDimInMeters.x, &Result.Chunk.x, &Result.Offset_.x);
    RecanonicaliseCoord(World->ChunkDimInMeters.y, &Result.Chunk.y, &Result.Offset_.y);
    RecanonicaliseCoord(World->ChunkDimInMeters.z, &Result.Chunk.z, &Result.Offset_.z);

    return(Result);
}

// NOTE(chowie): Just a positioning aid, we don't have tiles anymore
inline world_pos
ChunkPosFromTilePos(world *World, v3s AbsTile, v3 AdditionalOffset = V3(0, 0, 0))
{
    world_pos BasePos = {};

    v3 Offset = Hadamard(World->ChunkDimInMeters, V3i(AbsTile));
    world_pos Result = MapIntoChunkSpace(World, BasePos, AdditionalOffset + Offset);
    Assert(IsCanonical(World, Result.Offset_));
    return(Result);
}

// NOTE(chowie): Difference in tiles between the two positions (multiplied by
// however big the tiles are). With the delta between offsets.
inline v3
Subtract(world *World, world_pos *A, world_pos *B)
{
    v3 dTile = {V3i(A->Chunk) - V3i(B->Chunk)};
    v3 Result = Hadamard(World->ChunkDimInMeters, dTile) + (A->Offset_ - B->Offset_);
    return(Result);
}

/*
internal void
AddBlockToFreeList(world *World, world_entity_block *Old)
{
    Old->Next = World->FirstFreeBlock;
    World->FirstFreeBlock = Old;
}
*/

// TODO(chowie): Refer back to HmH debug.cpp StoreEvent for a good way
// to simplify/compress linked-list code
internal void
ChangeEntityLocation(memory_arena *Arena, world *World, u32 EntityIndex,
                     world_pos *OldP, world_pos *NewP)
{
    if(OldP && AreInSameChunk(World, OldP, NewP))
    {
        // NOTE(chowie): Leave entity as it is - no entity block change
    }
    else
    {
        if(OldP)
        {
            // NOTE(chowie): Pull entity out of its current entity block
            world_chunk *Chunk = GetWorldChunk(World, OldP->Chunk);
            Assert(Chunk);
            // STUDY(chowie): Do the hunt here instead of later. We're
            // already have to find the entity after all.
            if(Chunk)
            {
                world_entity_block *FirstBlock = &Chunk->FirstBlock;
                // STUDY(chowie): Sentinel (always have a link), singly-linked list
                for(world_entity_block *Block = FirstBlock;
                    Block;
                    Block = Block->Next)
                {
                    for(u32 Index = 0;
                        Index < Block->EntityCount;
                        ++Index)
                    {
                        // STUDY(chowie): Always keep a free space at
                        // the head end for easy insertion.
                        if(Block->EntityIndex[Index] == EntityIndex)
                        {
                            Assert(FirstBlock->EntityCount > 0); // NOTE(chowie): If it was 0, should've pulled in from its next block
                            Block->EntityIndex[Index] =
                                FirstBlock->EntityIndex[--FirstBlock->EntityCount];
                            // NOTE(chowie): If removing causes the block to go away
                            if(FirstBlock->EntityCount == 0)
                            {
                                if(FirstBlock->Next)
                                {
                                    world_entity_block *NextBlock = FirstBlock->Next;
                                    *FirstBlock = *NextBlock;

                                    NextBlock->Next = World->FirstFree;
                                    World->FirstFree = NextBlock;
                                }
                            }

                            // STUDY(chowie): Acts like a double break!
                            Block = 0;
                            break;
                        }
                    }
                }
            }
        }

        // NOTE(chowie): Insert entity into new entity block. Allocate
        // a chunk even if there wasn't one in that location, say if
        // it was the first entity stored in that section of the world.
        world_chunk *Chunk = GetWorldChunk(World, NewP->Chunk, Arena);
        Assert(Chunk);

        world_entity_block *Block = &Chunk->FirstBlock;
        if(Block->EntityCount == ArrayCount(Block->EntityIndex))
        {
            // NOTE(chowie): Out of room, get a new block! Freelist
            // may not have freed any block, only used.
            world_entity_block *OldBlock = World->FirstFree;
            if(OldBlock)
            {
                World->FirstFree = OldBlock->Next;
            }
            else
            {
                // NOTE(chowie): Memory that is piled up, use up to that amount before freeing.
                OldBlock = PushStruct(Arena, world_entity_block);
            }

            *OldBlock = *Block;
            Block->Next = OldBlock; // NOTE(chowie): This is just freelist deallocate
            Block->EntityCount = 0;
        }

        Assert(Block->EntityCount < ArrayCount(Block->EntityIndex));
        Block->EntityIndex[Block->EntityCount++] = EntityIndex;
    }
}

