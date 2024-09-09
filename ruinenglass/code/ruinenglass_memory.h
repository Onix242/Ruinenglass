#if !defined(RUINENGLASS_MEMORY_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Michael Chow $
   $Notice: $
   ======================================================================== */

// TODO(chowie): Move this to shared!
// NOTE(chowie): Don't need to do zero struct, can use "*NewKey = {};"
#define ZeroStruct(Instance) ZeroSize(sizeof(Instance), &(Instance))
#define ZeroArray(Count, Pointer) ZeroSize((Count)*sizeof((Pointer)[0]), Pointer)
inline void
ZeroSize(umm Size, void *Ptr)
{
    u8 *Byte = (u8 *)Ptr;
    while(Size--)
    {
        *Byte++ = 0;
    }
}

// TODO(chowie): Check this? And use this!
#define IsInBoundsArray(Count, type) IsInBounds_(Count, (Count)*sizeof(type))
internal b32x
IsInBounds_(u32 Index, u32 Size)
{
    b32x Result = (Index < Size);
    return(Result);
}

// NOTE(chowie): General purpose mem copy - not for performance!
inline void *
Copy(umm Size, void *SourceInit, void *DestInit)
{
    u8 *Source = (u8 *)SourceInit;
    u8 *Dest = (u8 *)DestInit;
    while(Size--) {*Dest++ = *Source++;}

    return(DestInit);
}

//
// NOTE(chowie): Memory Arenas
//

typedef struct memory_arena
{
    umm Size; // NOTE(chowie): Reserved
    u8 *Base;
    umm Used;

    s32 TempCount; // TODO(chowie): Do I really need this here? Becomes 8-bytes instead of 4, a s64
} memory_arena;

// TODO(chowie): Use this!
typedef struct temporary_memory
{
    memory_arena *Arena;
    umm Used;
} temporary_memory;

internal void
InitialiseArena(memory_arena *Arena, umm Size, u8 *Base)
{
    Arena->Size = Size;
    Arena->Base = Base;
    Arena->Used = 0;
    Arena->TempCount = 0;
}

// NOTE(chowie): Similar to begin and end temporary memory; pretty much
// permanent until wanting to clear it.
inline void
ClearArena(memory_arena *Arena)
{
    InitialiseArena(Arena, Arena->Size, Arena->Base);
}

inline umm
GetAlignmentOffset(memory_arena *Arena, umm Alignment)
{
    umm AlignmentOffset = 0;

    umm ResultPointer = (umm)Arena->Base + Arena->Used;
    umm AlignmentMask = Alignment - 1;
    if(ResultPointer & AlignmentMask)
    {
        AlignmentOffset = Alignment - (ResultPointer & AlignmentMask);
    }

    return(AlignmentOffset);
}

enum arena_push_flag
{
    ArenaFlag_ClearToZero = BitSet(0), // STUDY(chowie): (1 << 0) = 0x1
};
struct arena_push_params
{
    u32 Flags;
    u32 Alignment;
};

inline arena_push_params
DefaultArenaParams(void)
{
    arena_push_params Params;
    Params.Flags = ArenaFlag_ClearToZero;
    Params.Alignment = 4;
    return(Params);
}

inline arena_push_params
NoClear(void)
{
    arena_push_params Params = DefaultArenaParams();
    Params.Flags &= ~ArenaFlag_ClearToZero;
    return(Params);
}

inline arena_push_params
AlignNoClear(u32 Alignment)
{
    arena_push_params Params = DefaultArenaParams();
    // STUDY(chowie): Avoids slow startup as to not clear to zero
    Params.Flags &= ~ArenaFlag_ClearToZero;
    Params.Alignment = Alignment;
    return(Params);
}

inline umm
GetArenaSizeRemaining(memory_arena *Arena, arena_push_params Params = DefaultArenaParams())
{
    umm Result = Arena->Size - (Arena->Used + GetAlignmentOffset(Arena, Params.Alignment));
    return(Result);
}

#define PushStruct(Arena, type, ...) (type *)PushSize_(Arena, sizeof(type), ## __VA_ARGS__)
#define PushArray(Arena, Count, type, ...) (type *)PushSize_(Arena, (Count)*sizeof(type), ## __VA_ARGS__)
#define PushSize(Arena, Size, ...) PushSize_(Arena, Size, ## __VA_ARGS__)
#define PushCopy(Arena, Size, Source, ...) Copy(Size, Source, PushSize_(Arena, Size, ## __VA_ARGS__))

// STUDY(chowie): The alternative would be providing the arena's a way
// to free memory on demand or allow deallocation to the arena.
inline umm
GetEffectiveSizeFor(memory_arena *Arena, umm SizeInit, arena_push_params Params = DefaultArenaParams())
{
    umm Size = SizeInit;

    umm AlignmentOffset = GetAlignmentOffset(Arena, Params.Alignment);
    Size += AlignmentOffset;

    return(Size);
}

inline void *
PushSize_(memory_arena *Arena, umm SizeInit, arena_push_params Params = DefaultArenaParams())
{
    umm Size = GetEffectiveSizeFor(Arena, SizeInit, Params);
    Assert((Arena->Used + Size) <= Arena->Size);

    umm AlignmentOffset = GetAlignmentOffset(Arena, Params.Alignment);
    void *Result = Arena->Base + Arena->Used + AlignmentOffset;
    Arena->Used += Size;

    Assert(Size >= SizeInit);
    if(Params.Flags & ArenaFlag_ClearToZero)
    {
        ZeroSize(SizeInit, Result);
    }

    return(Result);
}

inline void
SubArena(memory_arena *Result, memory_arena *Arena, umm Size, arena_push_params Params = DefaultArenaParams())
{
    Result->Size = Size;
    Result->Base = (u8 *)PushSize_(Arena, Size, Params);
    Result->Used = 0;
    Result->TempCount = 0;
}

// TODO(chowie): Debugging for per-frame arena
inline b32x
ArenaHasRoomFor(memory_arena *Arena, umm SizeInit, arena_push_params Params = DefaultArenaParams())
{
    umm Size = GetEffectiveSizeFor(Arena, SizeInit, Params);
    b32x Result = ((Arena->Used + Size) <= Arena->Size);
    return(Result);
}

// NOTE(chowie): Nicety without having to call StringLength twice on a
// for a function. Returns how much this pushes on -> the string length
struct push_string_len_result
{
    char *String;
    u32 LengthPushed;
};
// NOTE(chowie): This exists so that strings remain across reloads,
// constant strings would be unloaded with dll.
inline push_string_len_result
PushString(memory_arena *Arena, char *Source)
{
    push_string_len_result Result = {};

    // NOTE(chowie): Include null terminator
    u32 Size = 1;
    for(char *At = Source;
        *At;
        ++At)
    {
        ++Size;
    }

    Result.LengthPushed = Size - 1; // NOTE(chowie): Remove null terminator
    Result.String = (char *)PushSize_(Arena, Size);
    for(u32 CharIndex = 0;
        CharIndex < Size;
        ++CharIndex)
    {
        Result.String[CharIndex] = Source[CharIndex];
    }

    return(Result);
}

inline char *
PushZ(memory_arena *Arena, u32 Length, char *Source)
{
    char *Dest = (char *)PushSize_(Arena, Length + 1, NoClear());
    for(u32 CharIndex = 0;
        CharIndex < Length;
        ++CharIndex)
    {
        Dest[CharIndex] = Source[CharIndex];
    }
    Dest[Length] = 0;

    return(Dest);
}

// NOTE(chowie): Thread safety probably not a problem here, memory
// should be come from each thread's arena.
inline temporary_memory
BeginTemporaryMemory(memory_arena *Arena)
{
    temporary_memory Result;

    Result.Arena = Arena;
    Result.Used = Arena->Used;

    ++Arena->TempCount;

    return(Result);
}

inline void
EndTemporaryMemory(temporary_memory TempMemory)
{
    memory_arena *Arena = TempMemory.Arena;

    Assert(Arena->Used >= TempMemory.Used);
    Arena->Used = TempMemory.Used;

    Assert(Arena->TempCount > 0); // NOTE(chowie): Make sure end has not been called more times than begin
    --Arena->TempCount;
}

// NOTE(chowie): Verifies that every frame ends with a balanced number
// of begins and ends.
inline void
CheckArena(memory_arena *Arena)
{
    Assert(Arena->TempCount == 0);
}

/*
// RESOURCE: https://hero.handmade.network/forums/code-discussion/t/7010-asset_system_-_loading_files_in_general_unknown_allocation_size
// TODO(chowie): Not quite sure if I want to do thread-safe arenas. Untested thread safety of PushSize_, Test!
inline void *
PushSize_(memory_arena *Arena, umm Size)
{
    void *Result = 0;
    while(!Result)
    {
        umm Used = Arena->Used;
        umm NewUsed = Used + Size;
        umm Reserved = Arena->Size;

        Assert((NewUsed) <= Arena->Size);
        if(NewUsed < Reserved)
        {
            umm UsedCheck = AtomicCompareExchangeU64(&Arena->Used, NewUsed, Used);

            if(UsedCheck == Used)
            {
                Result = Arena->Base + Used;
                Arena->Used += Size; // TODO(chowie): Does this require atomic add? While there's still have atomic compare exchange?
            }
        }
        else
        {
            break;
        }
    }

    return(Result);
}
*/

#define RUINENGLASS_MEMORY_H
#endif
