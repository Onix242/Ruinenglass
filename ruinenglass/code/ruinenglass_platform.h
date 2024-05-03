#if !defined(RUINENGLASS_PLATFORM_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Michael Chow $
   $Notice: $
   ======================================================================== */

/*
  NOTE:

  RUINENGLASS_INTERNAL:
  0 - Build for public release
  1 - Build for developer only

  RUINENGLASS_SLOW:
  0 - No slow code allowed!
  1 - Slow code welcome.
*/

/*
  NOTE(chowie): If you're every feeling stuck / unsure

  IMPORTANT(chowie): When dealing with memory always check Windows' Task
  Manager to ensure the memory allocated is reasonable!

  RESOURCE: https://austinmorlan.com/posts/pass_by_value_vs_pointer/
  TODO(chowie): For performance critical code, check where
  pointer-aliasing could happen!

  STUDY(chowie): Pointer-aliasing is when two pointers could point to
  the same memory and the compiler doesn't know if a _write_ to one of
  those pointers might effect a read from the other pointer. (Assuming
  it's non-volatile).
  *A = *B;
  *D = 5;
  *C = *B;

  STUDY(chowie): Bitshifting a negative value can never equal 0, it
  becomes -1.

  STUDY(chowie): Remember that modulo op transforms an absolute size
  to relative!

  STUDY(chowie): OS like handles, sockets, file streams needs a way to
  talk to you. Either indexes into tables or pointers into kernel
  memory.
*/

// NOTE(chowie): I should not have to do this, but brackets messes up
// with formatting {}
// STUDY(chowie): Extern prevents _C++ name mangling_
#define CExternStart extern "C" {
#define CExternEnd }

#ifdef __cplusplus
CExternStart
#endif

#include "ruinenglass_types.h"

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
struct push_string_amount_result
{
    char *String;
    u32 LengthPushed;
};
// NOTE(chowie): This exists so that strings remain across reloads,
// constant strings would be unloaded with dll.
inline push_string_amount_result
PushString(memory_arena *Arena, char *Source)
{
    push_string_amount_result Result = {};

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

// IMPORTANT(chowie): Don't need to do zero struct, just use "*NewKey = {};"
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

//
// RESOURCE: https://www.youtube.com/watch?v=2wio9UOFcow&list=PLT6InxK-XQvNKTyLXk6H6KKy12UYS_KDL&index=7
// NOTE(chowie): Mr. 4th's Length-based Strings
//

// STUDY(chowie): Treat the string data immutable after construction,
// pointer with size of bytes in a string. Not really modifying bytes
// a lot. The string helpers is easy-to-get right; mutating strings in
// place should be a case-by-case basis.

// #define ClampTop(a,b) Minimum(a,b)
// #define ClampBot(a,b) Maximum(a,b)

// NOTE(chowie): Treat this as standard buffers with buffer-specific functions
struct str8
{
    umm Size;
    u8 *Data;
};

struct str8_node
{
    str8_node *Next;
    str8 String;
};

// NOTE(chowie): Concat strings, splitting string; building a string,
// literal list of strings that are not meant to be joined.
struct str8_list
{
    str8_node *First;
    str8_node *Last;
    umm NodeCount;
    umm TotalSize;
};

internal b32x
Str8IsValid(str8 Source)
{
    b32x Result = (Source.Data != 0);
    return(Result);
}

internal b32x
Str8IsInBounds(str8 Source, umm At)
{
    b32x Result = (At < Source.Size); // TODO(chowie): Null terminated strings, "<="?
    return(Result);
}

internal b32x
Str8AreEqual(str8 A, str8 B)
{
    b32x Result = true;
    if(A.Size != B.Size)
    {
        Result = false;
    }

    for(u64 Index = 0;
        Index < A.Size; // TODO(chowie): Null terminated strings, "<="?
        ++Index)
    {
        if(A.Data[Index] != B.Data[Index])
        {
            Result = false;
        }
    }

    return(Result);
}

internal str8
Str8(u8 *String, umm Size)
{
    str8 Result = {};

    Result.Size = Size;
    Result.Data = String;

    return(Result);
}

// STUDY(chowie): If this was mutable, you would take care that this
// instance does not get mutated. E.g. Uppercasing modifying a piece
// of memory that the compiler has marked as constant.
#define CONSTANT_STRING8(String) Str8((u8 *)(String), sizeof(String) - 1)
#define TYPED_STRING(String) Str8((u8 *)(String), sizeof(*(String)))
#define Str8Expand(String) (int)((String).Size), ((String).Data)

internal str8
Str8Range(u8 *First, u8 *Opl)
{
    str8 Result = {};

    Result.Size = (umm)(Opl - First);
    Result.Data = First;

    return(Result);
}

// TODO(chowie): Is this really StringLength?
internal str8
Str8CStr(u8 *CString)
{
    u8 *CStringPtr = CString;
    for(; *CStringPtr != 0; ++CStringPtr); // STUDY(chowie): StringLength?

    str8 Result = Str8Range(CString, CStringPtr);

    return(Result);
}

// NOTE(chowie): Substrings does not need to do any allocations, as
// this was considered immutable that can come out with the same
// pointer of the same size.
internal str8
Str8Prefix(str8 String, umm Size)
{
    str8 Result = {};
    Result.Size = Minimum(Size, String.Size);
    Result.Data = String.Data;

    return(Result);
}

internal str8
Str8Postfix(str8 String, umm Size)
{
    str8 Result = {};

    umm ClampedSize = Minimum(Size, String.Size);
    umm SkipTo = String.Size - ClampedSize;

    Result.Size = ClampedSize;
    Result.Data = String.Data + SkipTo;

    return(Result);
}

internal str8
Str8Chop(str8 String, umm Amount)
{
    str8 Result = {};

    umm ClampedAmount = Minimum(Amount, String.Size);
    umm SizeRemaining = String.Size - ClampedAmount;

    Result.Size = SizeRemaining;
    Result.Data = String.Data;

    return(Result);
}

internal str8
Str8Skip(str8 String, umm Amount)
{
    str8 Result = {};

    umm ClampedAmount = Minimum(Amount, String.Size);
    umm SizeRemaining = String.Size - ClampedAmount;

    Result.Size = SizeRemaining;
    Result.Data = String.Data + ClampedAmount;

    return(Result);
}

// TODO(chowie): Find out what this does? Is it the first position of
// the substr? Does opl mean last?
internal str8
Str8SubstrOpl(str8 String, umm First, umm Opl)
{
    str8 Result = {};

    umm ClampedOpl = Minimum(Opl, String.Size);
    umm ClampedFirst = Minimum(First, ClampedOpl);

    Result.Size = ClampedOpl - ClampedFirst;
    Result.Data = String.Data + ClampedFirst;

    return(Result);
}

internal str8
Str8SubstrSize(str8 String, umm First, umm Size)
{
    str8 Result = Str8SubstrOpl(String, First, First + Size);

    return(Result);
}

internal b32x
Str8IsSlash(u8 Char)
{
    b32x Result = (Char == '/' || Char == '\\');
    return(Result);
}

internal str8
Str8ChopLastSlash(str8 String)
{
    str8 Result = String;
    if(String.Size > 0)
    {
        // NOTE(chowie): Position is one past last slash
        umm Position = String.Size;
        for(s64 PositionIndex = String.Size - 1;
            PositionIndex >= 0;
            --PositionIndex)
        {
            if(Str8IsSlash(String.Data[PositionIndex]))
            {
                Position = PositionIndex;
                break;
            }
        }

        // NOTE(chowie): Chop resulting string
        Result.Size = Position;
    }

    return(Result);
}

internal u8
Str8Uppercase(u8 Char)
{
    if(('a' <= Char) &&
       (Char <= 'z'))
    {
        Char += (u8)('A' - 'a'); // TODO(chowie): Turn off warning? Should really not have to do this
    }

    return(Char);
}

internal u8
Str8Lowercase(u8 Char)
{
    if(('A' <= Char) &&
       (Char <= 'Z'))
    {
        Char += 'a' - 'A';
    }

    return(Char);
}

// TODO(chowie): Not sure if I should remove this?
#include <memory.h>

// TODO(chowie): Find out where this can be used? Serialiser/Deserialiser
internal b32x
Str8Read(str8 String, umm Offset, void *Dest, umm Size)
{
    b32x Result = false;
    if((Offset + Size) <= String.Size)
    {
        Result = true;
        memcpy(Dest, String.Data + Offset, Size);
    }

    return(Result);
}
#define TYPED_STR8_READ(Data, Offset, Dest) Str8Read((Data), (Offset), (Dest), sizeof(*(Dest)))

// TODO(chowie): Not quite sure if this would be super useful for me?
// NOTE(chowie): Avoids getting an arena involved, put on call
// stack. Careful about which node is being passed in, and not using
// the same node twice. For taking a function which would have taken a
// list into one who takes a single element list.
internal void
Str8ListPushExplicit(str8_list *List, str8 String,
                     str8_node *NodeMemory)
{
    NodeMemory->String = String;
    SLLQueuePush(List->First, List->Last, NodeMemory);
    List->NodeCount++;
    List->TotalSize = String.Size;
}

internal void
Str8ListPush(memory_arena *Arena, str8_list *List,
             str8 String)
{
    str8_node *NodeMemory = PushArray(Arena, 1, str8_node);
    Str8ListPushExplicit(List, String, NodeMemory);
}

typedef struct str8_join
{
    str8 Pre;
    str8 Mid;
    str8 Post;
} str8_join;
internal str8
Str8Join(memory_arena *Arena, str8_list *List,
         str8_join *JoinOptional)
{
    // NOTE(chowie): Join Param
    local_persist str8_join DummyJoin = {};
    str8_join *Join = JoinOptional;
    if(!Join)
    {
        Join = &DummyJoin;
    }

    umm JoinMid = (Join->Mid.Size*(List->NodeCount - 1));
    Assert(JoinMid != 0);

    umm TotalSize = (Join->Pre.Size +
                     Join->Post.Size +
                     JoinMid +
                     List->TotalSize);

    // NOTE(chowie): Build string
    u8 *String = PushArray(Arena, TotalSize + 1, u8);
    u8 *StringPtr = String;

    // NOTE(chowie): Write pre
    memcpy(StringPtr, Join->Pre.Data, Join->Pre.Size);
    StringPtr += Join->Pre.Size;

    // NOTE(chowie): Write mid
    b32x IsMid = false;
    for(str8_node *Node = List->First;
        Node != 0;
        Node = Node->Next)
    {
        if(IsMid)
        {
            memcpy(StringPtr, Join->Mid.Data, Join->Mid.Size);
            StringPtr += Join->Mid.Size;
        }

        // NOTE(chowie): Write node string
        memcpy(StringPtr, Node->String.Data, Node->String.Size);
        StringPtr += Node->String.Size;

        IsMid = true;
    }

    // NOTE(chowie): Write post
    memcpy(StringPtr, Join->Post.Data, Join->Post.Size);
    StringPtr += Join->Post.Size;

    // NOTE(chowie): Write null
    *StringPtr = 0;

    str8 Result = Str8(String, TotalSize);
    return(Result);
}

// NOTE(chowie): Split by one or more characters. Any time a chosen
// character is seen, it marks as a split byte, then omit the word
// between split bytes. If there are empty words, it ignores.
// STUDY(chowie): Omitting the strings means lots of whitespaces means
// it would waste a lot more than expected
// TODO(chowie): Split by string?
internal str8_list
Str8Split(memory_arena *Arena, str8 String,
          u8 *Splits, u32 Count)
{
    str8_list Result = {};

    u8 *StringPtr = String.Data;
    u8 *FirstWord = StringPtr;
    u8 *Opl = String.Data + String.Size;
    for(; StringPtr < Opl;
        ++StringPtr)
    {
        // NOTE(chowie): Split
        u8 Byte = *StringPtr;
        b32x IsSplit = false;
        for(u32 SplitIndex = 0;
            SplitIndex < Count;
            ++SplitIndex)
        {
            if(Byte == Splits[SplitIndex])
            {
                IsSplit = true;
                break;
            }
        }

        if(IsSplit)
        {
            // NOTE(chowie): Try to omit word, advance first word
            if(FirstWord < StringPtr)
            {
                Str8ListPush(Arena, &Result, Str8Range(FirstWord, StringPtr));
            }
            FirstWord = StringPtr + 1;
        }
    }

    // NOTE(chowie): Try to omit final word
    if(FirstWord < StringPtr)
    {
        Str8ListPush(Arena, &Result, Str8Range(FirstWord, StringPtr));
    }

    return(Result);
}

#include <stdarg.h>
// TODO(chowie): Remove stdio for printing!
#include <stdio.h>
// NOTE(chowie): Format strings
// TODO(chowie): Don't use this function yet
// TODO(chowie): I would like to try to replace this with d7sam's
// version, but hopefully passing an arena!
internal str8
Str8Push(memory_arena *Arena, char *Format, va_list Args)
{
    // NOTE(chowie): If need to attempt again
    va_list Args2;
    va_copy(Args2, Args);

    // NOTE(chowie): Build string in 1024 bytes
    umm BufferSize = 1024;
    u8 *Buffer = PushArray(Arena, BufferSize, u8);
    umm ActualSize = vsnprintf((char *)Buffer, BufferSize, Format, Args);

    str8 Result = {};
    if(ActualSize < BufferSize)
    {
        ClearArena(Arena); // TODO(chowie): Pop?
        Result = Str8(Buffer, ActualSize);
    }
    else
    {
        ClearArena(Arena); // TODO(chowie): Pop?
        u8 *FixedBuffer = PushArray(Arena, ActualSize + 1, u8);
        umm FinalSize = vsnprintf((char *)FixedBuffer, ActualSize + 1, Format, Args2);
        Result = Str8(FixedBuffer, FinalSize);
    }

    va_end(Args2);

    return(Result);
}

//
// NOTE: Services that the platform layer provides to game
//

#if RUINENGLASS_INTERNAL

/*
  IMPORTANT(chowie):

  These are not for doing anything in the shipping game - they
  are blocking and the write does not protect against lost data!

  When writing to a critical file, don't overwrite the old file
  because it can fail; it could only partially overwrite (which can
  corrupt the file).

  TODO(chowie):

  Instead, write to a different file with a rolling buffer scheme. A
  and B file on alternating runs of the game. Or use a temp file and
  delete the old file and rename it in its place. Rename would have to
  fail.
*/

#define DEBUG_PLATFORM_FREE_FILE_MEMORY(name) void name(void *Memory)
typedef DEBUG_PLATFORM_FREE_FILE_MEMORY(debug_platform_free_file_memory);

typedef struct debug_read_file_result
{
    u32 ContentsSize;
    void *Contents;
} debug_read_file_result;

#define DEBUG_PLATFORM_READ_ENTIRE_FILE(name) debug_read_file_result name(char *FileName)
typedef DEBUG_PLATFORM_READ_ENTIRE_FILE(debug_platform_read_entire_file);

#define DEBUG_PLATFORM_WRITE_ENTIRE_FILE(name) b32x name(char *FileName, u32 MemorySize, void *Memory)
typedef DEBUG_PLATFORM_WRITE_ENTIRE_FILE(debug_platform_write_entire_file);

#endif

typedef struct platform_api
{
#if RUINENGLASS_INTERNAL
    debug_platform_free_file_memory *DEBUGFreeFileMemory;
    debug_platform_read_entire_file *DEBUGReadEntireFile;
    debug_platform_write_entire_file *DEBUGWriteEntireFile;
#endif
} platform_api;

typedef struct game_memory
{
    // IMPORTANT(chowie): VirtualAlloc is guaranteed to be cleared to 0 at startup!
    FIELD_ARRAY(memory_arena,
    {
        memory_arena Permanent; // NOTE(chowie): What game needs to be run; entity state, player
        memory_arena Transient; // NOTE(chowie): Render stack, cache textures, sounds; recreated and can be loaded off disc
        memory_arena Samples; // TODO(chowie): Make this into a dedicated audio arena -> suballocate room for SoundSamples
        // TODO(chowie): Debug Arena?
        // TODO(chowie): How would I use this? -> "char ArenaBuffer[sizeof(memory_arena)];"
    });
    // TODO(chowie): Flush transient store!

    platform_api PlatformAPI;

    b32x ExecutableReloaded; // TODO(chowie): Find out if this is really necessary!
} game_memory;

//
// NOTE: Services that the game provides to the platform layer.
// Takes - timing, controller/keyboard input, bitmap buffer to use, sound buffer to use.
//

typedef struct game_sound_output_buffer
{
    u32 SamplesPerSecond;
    u32 SampleCount;

    s16 *Samples; // IMPORTANT(chowie): Must be padded to a multiple of 4!
} game_sound_output_buffer;

#define BITMAP_BYTES_PER_PIXEL 4
typedef struct game_offscreen_buffer
{
    void *Memory;
    v2u Dim;
    u32 Pitch;
} game_offscreen_buffer;

//
//
//

typedef struct game_button_state
{
    u32 HalfTransitionCount; // NOTE(chowie): Either up, or down = "half" a keypress
    b32x EndedDown;
} game_button_state;

inline b32x
WasPressed(game_button_state State)
{
    b32x Result = ((State.HalfTransitionCount > 1) ||
                  ((State.HalfTransitionCount == 1) && State.EndedDown));
    return(Result);
}

inline b32x
IsDown(game_button_state State)
{
    b32x Result = (State.EndedDown);
    return(Result);
}

typedef struct game_controller_input
{
    b32x IsConnected;
    b32x IsAnalog;
    v2 StickAverage;

    FIELD_ARRAY(game_button_state,
    {
        game_button_state MoveUp;
        game_button_state MoveDown;
        game_button_state MoveLeft;
        game_button_state MoveRight;

        game_button_state ActionUp;
        game_button_state ActionDown;
        game_button_state ActionLeft;
        game_button_state ActionRight;

        game_button_state LeftShoulder;
        game_button_state RightShoulder;

        game_button_state Back;
        game_button_state Start;
    });
} game_controller_input;

typedef struct game_input
{
    // STUDY(chowie): The wall clock is passed as an input device
    // since time is passing (the elapsed interval). What do you have
    // to record to replay a user's session? Keyboard, mouse and time.
    r32 dtForFrame;

    b32x QuitRequested;

// NOTE(chowie): Includes Keyboard
#define MAX_CONTROLLERS 5
    game_controller_input Controllers[MAX_CONTROLLERS];
    // TODO(chowie): Find better ways for ShiftDown, ControlDown, AltDown etc...
} game_input;

// NOTE(chowie): Processed in Win32, game sees a virtualised
// controller. If the keyboard is functioning like a controller, the
// platform is responsible for that. To detect different keyboards.
inline game_controller_input *
GetController(game_input *Input, u32 ControllerIndex)
{
    Assert(ControllerIndex < ArrayCount(Input->Controllers)); // TODO(chowie): Proper bounds checking?

    game_controller_input *Result = &Input->Controllers[ControllerIndex];
    return(Result);
}

//
// NOTE(chowie): Exported function
//

#define GAME_UPDATE_AND_RENDER(name) void name(game_memory *Memory, game_input *Input, game_offscreen_buffer *Buffer)
typedef GAME_UPDATE_AND_RENDER(game_update_and_render);

#define GAME_GET_SOUND_SAMPLES(name) void name(game_memory *Memory, game_sound_output_buffer *SoundBuffer)
typedef GAME_GET_SOUND_SAMPLES(game_get_sound_samples);

#ifdef __cplusplus
CExternEnd
#endif

// RESOURCE: https://www.computerenhance.com/p/powerful-page-mapping-techniques
// https://github.com/cmuratori/computer_enhance/blob/main/perfaware/part3/listing_0122_write_watch_main.cpp
// TODO(chowie): Can I use any of these page mapping techniques?
// TODO(chowie): Circular buffer for Worker Threads? Probably with VirtualAlloc2 + PagedMemory // RESOURCE: https://www.computerenhance.com/p/powerful-page-mapping-techniques

#define RUINENGLASS_PLATFORM_H
#endif
