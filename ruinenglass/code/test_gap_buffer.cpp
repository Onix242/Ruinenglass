/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Michael Chow $
   $Notice: $
   ======================================================================== */

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <string.h>
#include <windows.h>

#if 0
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t umm;
typedef int32_t s32;
typedef s32 b32;
typedef size_t memory_index;
typedef float r32;
typedef double r64;

#if !defined(internal)
#define internal static
#endif
#define local_persist static
#define global_variable static

#define Minimum(A, B) ((A < B) ? (A) : (B))
#define Maximum(A, B) ((A > B) ? (A) : (B))

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))

inline u32
StringLength(char *String)
{
    u32 Count = 0;
    while(*String++)
    {
        ++Count;
    }
    return(Count);
}

// TODO(chowie): Not sure if I need this to squish strings
internal void
CatStrings(size_t SourceACount, char *SourceA,
           size_t SourceBCount, char *SourceB,
           size_t DestCount, char *Dest)
{
    // TODO: Dest bound checking
    for(u32 Index = 0;
        Index < SourceACount;
        ++Index)
    {
        *Dest++ = *SourceA++;
    }
    for(u32 Index = 0;
        Index < SourceBCount;
        ++Index)
    {
        *Dest++ = *SourceB++;
    }

    *Dest++ = 0; // NOTE: Insertion of NULL terminator
}

inline umm
Clamp(umm Min, umm Value, umm Max)
{
    umm Result = Value;

    if(Result <= Min)
    {
        Result = Min;
    }
    else if(Result > Max)
    {
        Result = Max;
    }

    return(Result);
}

inline u32
AbsoluteValue(u32 U32)
{
    u32 Result = (u32)fabs(U32);
    return(Result);
}

// RESOURCE: https://www.youtube.com/watch?v=agUiYkvkoVg
// TODO(chowie): I wonder if I should split the maxsize of the buffer inside of the gapbuffer itself?
//global_variable umm BufferPosition;
struct gap_buffer
{
    u8 *Buffer;
    umm Length;
    umm Start;
    umm End; // STUDY(chowie): Could use start and len/gap
    // TODO(chowie): Pass an arena allocator in! Check back in at 15:28!
};

internal umm
GetGapBufferLength(gap_buffer *GapBuffer)
{
    umm Result = GapBuffer->Length - (GapBuffer->End - GapBuffer->Start); // NOTE(chowie): Does not include gap
    return(Result);
}

#define GapBufferLenWithGap(GapBuffer) (GapBuffer->Length + GapBufferLength)

internal void
GapBufferInit(gap_buffer *GapBuffer, umm Size)
{
    GapBuffer->Buffer = (u8 *)malloc(Size);
    GapBuffer->Length = 0;
    GapBuffer->Start = 0;
    GapBuffer->End = Size;
}

// NOTE(chowie): Moves the Gap to the cursor position. Cursors are clamped [0,n) where n is the filled count of the buffer.
internal void
ShiftGapTo(gap_buffer *GapBuffer, umm Cursor)
{
    umm GapLength = GapBuffer->End - GapBuffer->Start;
    Cursor = Clamp(Cursor, 0, GapBuffer->Length - GapLength); // TODO(chowie): Check if clamped is correct?
    //printf("GapLength: %d, Cursor: %d\n", GapLength, Cursor); // NOTE(chowie): Accounts for expansion
    if(Cursor != GapBuffer->Start) // NOTE(chowie): Something inside
    {
        if(GapBuffer->Start < Cursor)
        {
            // NOTE(chowie): Gap is before the cursor
            //   v~~~v
            //[12]              [3456789abc]
            //--------|----------------------------------- Gap is BEFORE Cursor
            //[123456]              [789abc]
            umm Delta = Cursor - GapBuffer->Start;
            memcpy(&GapBuffer->Buffer[GapBuffer->Start], &GapBuffer->Buffer[GapBuffer->End], Delta);
            GapBuffer->Start += Delta; // NOTE(chowie): Both buffers must move to the right
            GapBuffer->End += Delta;
        }
        else if(GapBuffer->Start > Cursor)
        {
            // NOTE(chowie): Gap is after the cursor
            //   v~~~v
            //[123456]              [789abc]
            //---|---------------------------------------- Gap is AFTER Cursor
            //[12]              [3456789abc]
            umm Delta = GapBuffer->Start - Cursor;
            memcpy(&GapBuffer->Buffer[GapBuffer->End - Delta], &GapBuffer->Buffer[GapBuffer->Start - Delta], Delta);
            GapBuffer->Start -= Delta; // NOTE(chowie): Both buffers must move to the left
            GapBuffer->End -= Delta;
        }
    }
}

// NOTE(chowie): Verifies the buffer can hold the needed write. Resizes the array if not. By default doubles array size.
internal void
CheckGapSize(gap_buffer *GapBuffer, umm Required)
{
    umm GapLength = GapBuffer->End - GapBuffer->Start;
    if(GapLength < Required)
    {
        ShiftGapTo(GapBuffer, GapBuffer->Length - GapLength);
        umm RequiredBufferSize = Required + GapBuffer->Length - GapLength;
        u8 *NewBuffer = (u8 *)malloc(2 * RequiredBufferSize); // NOTE(chowie): Maximum to account for big writes // TODO(chowie): Check if it's correct! TIMESTAMP: 12:44
        memcpy(NewBuffer, GapBuffer->Buffer, GapBuffer->End); // TODO(chowie): Check if it's correct! It should only take until the end. Off by one?
        free(GapBuffer->Buffer);
        GapBuffer->Buffer = NewBuffer;
        GapBuffer->End = GapBuffer->Length; // NOTE(chowie): All of the good data is in the start-to-middle. IMPORTANT: Gap start does not change here!
    }
}

// NOTE(chowie): Moves the gap to the cursor, then moves the gap pointer beyond count, effectively deleting it.
// NOTE: Do not rely on the gap being 0, remove will leave as-is values behind in the gap  
// IMPORTANT: Does not protect for unicode at present, simply deletes bytes  
internal void
Remove(gap_buffer *GapBuffer, umm Cursor, u32 Count)
{
    u32 Delete = AbsoluteValue(Count);
    umm NewCursor = Cursor;
    if(Count < 0)
    {
        NewCursor = Maximum(0, NewCursor - Delete);
    }
    ShiftGapTo(GapBuffer, NewCursor);
    GapBuffer->End = Minimum(GapBuffer->End + Delete, GapBuffer->Length); // NOTE(chowie): Protect from runoff!
    //--------|-----------------------------------
    //[123456]              [789abc]
}

internal void
InsertChar(gap_buffer *GapBuffer, umm Cursor, u8 Char)
{
    CheckGapSize(GapBuffer, 1);
    ShiftGapTo(GapBuffer, Cursor);
    GapBuffer->Buffer[GapBuffer->Start++] = Char;
    GapBuffer->Length++;
}

internal void
InsertString(gap_buffer *GapBuffer, umm Cursor, char *String)
{
    CheckGapSize(GapBuffer, StringLength(String));
    ShiftGapTo(GapBuffer, Cursor);
    strcpy((char *)GapBuffer->Buffer, String); // TODO(chowie): This is totally not correct!
    GapBuffer->Start += StringLength(String);
}

// TODO(chowie): utf-8 support? How does one encode a rune in C? TIMESTAMP: 17:00-18:00
/*
internal void
InsertRune(gap_buffer *GapBuffer, umm Cursor, u8 Char)
{
    CheckGapSize(GapBuffer, 1);
    ShiftGapTo(GapBuffer, Cursor);
    GapBuffer->Buffer[GapBuffer->Start] = Char;
    GapBuffer->Start += 1;
}

// TODO(chowie): How does one insert a string in C? TIMESTAMP: 17:00-18:00
internal void
InsertString(gap_buffer *GapBuffer, umm Cursor, char *String)
{
    CheckGapSize(GapBuffer, StringLength(String));
    ShiftGapTo(GapBuffer, Cursor);
    strcpy(GapBuffer->Buffer, *String); // TODO(chowie): This is totally not correct!
    GapBuffer->Start += StringLength(String);
}
*/

// TODO(chowie): Ideally want cursor position - length to find the composite string between the two -> left and right of the cursor!
int
main(void)
{
    gap_buffer GapBuffer = {};

    GapBufferInit(&GapBuffer, 2);
    InsertChar(&GapBuffer, 0, '0');
    InsertChar(&GapBuffer, 1, '1');
    InsertChar(&GapBuffer, 2, '2');
    InsertChar(&GapBuffer, 3, '3');
    InsertChar(&GapBuffer, 4, '4');
    InsertChar(&GapBuffer, 2, 'A');
    //InsertString(&GapBuffer, 4, "Hello");
    //Remove(&GapBuffer, 1, 2);
    // TODO(chowie): Insertion/Removal of the character is wrong LOL
    char TextLengthBuffer[256];
    _snprintf_s(TextLengthBuffer, sizeof(TextLengthBuffer),
                "\nInside Gap Buffer: %d\n", GapBuffer);
    OutputDebugStringA(TextLengthBuffer);
    //printf("%.*s", 16, GapBuffer);

    char TextBuffer[256];
    // RESORUCE: https://stackoverflow.com/questions/8170697/printf-a-buffer-of-char-with-length-in-c
    umm BufferLength = GetGapBufferLength(&GapBuffer);
    _snprintf_s(TextBuffer, sizeof(TextBuffer),
                "Buffer Length: %d\n", BufferLength);
    OutputDebugStringA(TextBuffer);
    return(0);
}

#else

#if !defined(internal)
#define internal static
#endif
#define local_persist static
#define global_variable static

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;
typedef int_least32_t b32x;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef intptr_t smm;
typedef uintptr_t umm;

typedef size_t memory_index;

typedef float r32;
typedef double r64;

#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value)*1024LL)
#define Gigabytes(Value) (Megabytes(Value)*1024LL)
#define Terabytes(Value) (Gigabytes(Value)*1024LL)

#define ArrayCount(arr)(sizeof((arr)) /(sizeof((arr)[0])))
#define Minimum(A, B) ((A < B) ? (A) : (B))
#define Maximum(A, B) ((A > B) ? (A) : (B))

#define Assert(Expression) if(!(Expression)) {*(int *)0 = 0;}

inline u32
StringLength(char *String)
{
    u32 Count = 0;
    if(String)
    {
        while(*String++)
        {
            ++Count;
        }
    }

    return(Count);
}

struct memory_arena
{
    umm Size;
    u8 *Base;
    umm Used;
};

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

inline char *
PushString(memory_arena *Arena, char *Source)
{
    // NOTE(chowie): Include the null terminator
    u32 Size = 1;
    for(char *At = Source;
        *At;
        ++At)
    {
        ++Size;
    }

    char *Dest = (char *)PushSize_(Arena, Size);
    for(u32 CharIndex = 0;
        CharIndex < Size;
        ++CharIndex)
    {
        Dest[CharIndex] = Source[CharIndex];
    }

    return(Dest);
}

inline void
ClearArena(memory_arena *Arena)
{
    InitialiseArena(Arena, Arena->Size, Arena->Base);
}

struct game_memory
{
    umm TextStorageSize;
    u8 *TextStorage; // NOTE: REQUIRED to be cleared to zero at startup 
};

#define CONCAT_BUFFER_SIZE 256
struct concat_buffer
{
    s32 CharCount;
    char Buffer[CONCAT_BUFFER_SIZE]; // TODO(chowie): Is it possible to pass this in into an arena?
};

// RESOURCE: https://gist.github.com/d7samurai/1d778693ba33bbd2b9d709b209cc0aba
// TODO(chowie): This hideous functions is really convenient! Probably only use this for debugging only!
struct d7sam_concat
{
    d7sam_concat(char* Source) { operator()(Source); }

    concat_buffer Text = {};

    d7sam_concat &
    operator()(char* Source)
    {
        // TODO(chowie): How do I remove the null terminator?
        // NOTE(chowie): Include null terminator
        u32 Size = StringLength(Source) + 1;

        for(u32 CharIndex = 0;
            CharIndex < Size;
            ++CharIndex)
        {
            Text.Buffer[Text.CharCount++] = Source[CharIndex];
        }
        Text.CharCount--;

        return(*this);
    }

    operator char* ()
    {
        return(Text.Buffer);
    }
};

int
main(void)
{
    game_memory GameMemory = {};
    GameMemory.TextStorageSize = Megabytes(1);
    GameMemory.TextStorage = (u8 *)VirtualAlloc(0, GameMemory.TextStorageSize,
                                                MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE);

    if(GameMemory.TextStorage)
    {
        memory_arena TextArena;
        InitialiseArena(&TextArena, GameMemory.TextStorageSize - sizeof(game_memory),
                        GameMemory.TextStorage + sizeof(game_memory));

        // NOTE(chowie): String test
        char TextLengthBuffer[256];
        char *Name = "Slim shady?";
        char *Test = PushString(&TextArena, d7sam_concat("Attention! ")(Name));

        _snprintf_s(TextLengthBuffer, sizeof(TextLengthBuffer),
                    "Inside Buffer: %s\n", Test);
        OutputDebugStringA(TextLengthBuffer);

        ClearArena(&TextArena);
    }

    // attention, slim shady! there's an error in line 1337 : the error code is 666 and the temperature is -98.6 degrees
    //OutputDebugStringA(d7sam_concat("attention, ")(name)("! there's an error in line ")(line)(" : the error code is ")(666)(" and the temperature is ")(temp, 1)(" degrees\n"));
    return(0);
}

#endif
