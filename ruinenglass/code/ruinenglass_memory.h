#if !defined(RUINENGLASS_MEMORY_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Michael Chow $
   $Notice: $
   ======================================================================== */

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
}

#define PushStruct(Arena, type) (type *)PushSize_(Arena, sizeof(type))
#define PushArray(Arena, Count, type) (type *)PushSize_(Arena, (Count)*sizeof(type))
inline void *
PushSize_(memory_arena *Arena, umm Size)
{
    Assert((Arena->Used + Size) <= Arena->Size);

    void *Result = Arena->Base + Arena->Used;
    Arena->Used += Size;

    return(Result);
}

// TODO(chowie): Check this? And use this!
#define IsInBoundsArray(Count, type) IsInBounds_(Count, (Count)*sizeof(type))
internal b32x
IsInBounds_(u32 Index, u32 Size)
{
    b32x Result = (Index < Size);
    return(Result);
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

inline void
CheckArena(memory_arena *Arena)
{
    Assert(Arena->TempCount == 0);
}

// NOTE(chowie): Similar to begin and end temporary memory; pretty much
// permanent until wanting to clear it.
inline void
ClearArena(memory_arena *Arena)
{
    InitialiseArena(Arena, Arena->Size, Arena->Base);
}

#define RUINENGLASS_MEMORY_H
#endif
