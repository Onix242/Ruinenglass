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

#define Swap(type, A, B) {type Temp = (A); (A) = (B); (B) = Temp;}

inline s32
SignOf(s32 Value)
{
    s32 Result = (Value >= 0) ? 1 : -1;
    return (Result);
}

inline b32x
IsEndOfLine(char C)
{
    b32x Result = ((C == '\n') ||
                   (C == '\r'));

    return(Result);
}

inline b32x
IsWhitespace(char C)
{
    b32x Result = ((C == ' ') ||
                   (C == '\t') ||
                   (C == '\v') ||
                   (C == '\f') ||
                   IsEndOfLine(C));

    return(Result);
}

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
    Cursor = Clamp(0, Cursor, GapBuffer->Length - GapLength); // TODO(chowie): Check if clamped is correct?
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
        umm RequiredBufferSize = Required + GapBuffer->Length - GapLength; // NOTE(chowie): Optimisation, as you know the gap buffer len is the end size!
        u8 *NewBuffer = (u8 *)malloc((2 * RequiredBufferSize) * sizeof(u8)); // NOTE(chowie): Maximum to account for big writes // TODO(chowie): Check if it's correct! TIMESTAMP: 12:44
        memcpy(NewBuffer, GapBuffer->Buffer, GapBuffer->Length); // TODO(chowie): Check if it's correct! It should only take until the end. Off by one?
        GapBuffer->Length = (2 * RequiredBufferSize);
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
    //GapBuffer->Length++;
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
                "\nInside Gap Buffer: %s\n", GapBuffer.Buffer);
    OutputDebugStringA(TextLengthBuffer);
    return(0);
}

#else

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

// NOTE(chowie): Log10 should really start from 0
inline s32
NumDigitsLog10(u32 Value)
{
    u32 Result = 0;
    if(Value < 10000000000)
    {
        Result = ((Value >= 1000000000) ? 10 :
                  (Value >= 100000000) ? 9 :
                  (Value >= 10000000) ? 8 :
                  (Value >= 1000000) ? 7 :
                  (Value >= 100000) ? 6 :
                  (Value >= 10000) ? 5 :
                  (Value >= 1000) ? 4 :
                  (Value >= 100) ? 3 :
                  (Value >= 10) ? 2 : 1);
    }
    else
    {
        Result = ((Value >= 1000000000000000000) ? 19 :
                  (Value >= 100000000000000000) ? 18 :
                  (Value >= 10000000000000000) ? 17 :
                  (Value >= 1000000000000000) ? 16 :
                  (Value >= 100000000000000) ? 15 :
                  (Value >= 10000000000000) ? 14 :
                  (Value >= 1000000000000) ? 13 :
                  (Value >= 100000000000) ? 12 : 11);
    }

    return(Result);
}
#define Base10 10
global_variable r32 Bases[] = { 1, 10, 100, 1000, 10000, 100000, 1000000 };

inline u32
RoundR32ToU32(r32 R32)
{
    u32 Result = (u32)roundf(R32);
    return(Result);
}

// RESOURCE(d7samurai): https://gist.github.com/d7samurai/1d778693ba33bbd2b9d709b209cc0aba
// TODO(chowie): This hideous functions is really convenient! Probably only use this for debugging only!
struct d7sam_concat
{
    d7sam_concat(char* Source) { operator()(Source); }
    d7sam_concat(s32 Value) { operator()(Value); }
    d7sam_concat(r32 Value, u32 Decimals = 2) { operator()(Value, Decimals); }

    u32 CharCount = 0;
    char TextBuffer[CONCAT_BUFFER_SIZE];

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
            TextBuffer[CharCount++] = Source[CharIndex];
        }
        CharCount--;

        return(*this);
    }

    d7sam_concat &
    operator()(s32 Value)
    {
        b32x Negative = false;
        if(Value < 0)
        {
            Negative = true;
            Value = -Value;
        }

        CharCount += NumDigitsLog10(Value) + Negative;
        s32 CharIndex = CharCount;

        TextBuffer[CharIndex--] = 0;
        do {
            TextBuffer[CharIndex--] = '0' + (Value % Base10);
            Value /= Base10;
        } while(Value);

        if(Negative)
        {
            TextBuffer[CharIndex] = '-';
        }

        return(*this);
    }

    d7sam_concat &
    operator()(r32 Value, u32 Decimals = 2)
    {
        b32x Negative = false;
        if(Value < 0)
        {
            Negative = true;
            Value = -Value;
        }

        u32 CastValue = RoundR32ToU32(Bases[Decimals] * Value);
        u32 MaxDecimals = Maximum(NumDigitsLog10(CastValue), (s32)Decimals + 1);
        CharCount += MaxDecimals + Negative + (Decimals > 0);
        u32 CharIndex = CharCount;

        TextBuffer[CharIndex--] = 0;
        do {
            TextBuffer[CharIndex--] = '0' + (CastValue % Base10);
            CastValue /= Base10;
            if(CharIndex == (CharCount - (s32)Decimals - 1))
            {
                TextBuffer[CharIndex--] = '.';
            }
        } while(CastValue || ((CharCount - CharIndex) <= (Decimals ? Decimals + 2 : 0)));

        if(Negative)
        {
            TextBuffer[CharIndex] = '-';
        }

        return(*this);
    }

    operator char* ()
    {
        return(TextBuffer);
    }
};

//
// NOTE(chowie): Start of triangle numbers
//

// TODO(chowie): This can be extended to layers, are there enough
// entity types yet?
enum entity_type : u16
{
    EntityType_Null,

    EntityType_Space,

    EntityType_Hero,
    EntityType_Wall,
//    EntityType_Familiar,
//    EntityType_Monstar,
//    EntityType_Sword,
//    EntityType_Stairwell,

    EntityType_Count,
};

#define ENTITY_PAIR_TABLE_MAX(TableDim) (TableDim * (TableDim + 1) / 2)
struct entity
{
    u32 EntityTypePairTable[ENTITY_PAIR_TABLE_MAX(EntityType_Count)];
    u32 TriangleNumbersTable[EntityType_Count + 1];
};

// RESOURCE(remaley): https://anthropicstudios.com/2020/03/30/symmetric-matrices/
internal u32
MapEntityPairToIndex(u16 A, u16 B)
{
    // TODO(chowie): I assume that if with swap is better than two if min/max
    // STUDY(chowie): This makes entity pairs AB vs BA pair agnostic
    u32 Low = B;
    u32 High = A;
    if(A < B)
    {
        Swap(u32, Low, High);
    }

    u32 Triangle = High * (High + 1) / 2;
    u32 Column = Low;

    u32 Result = Triangle + Column;
    return(Result);
}

struct triangle_number_result
{
    b32x IsTriangleNumber;
    u32 CurrTriangleNumber;
    u32 PrevTriangleNumber;
};
inline triangle_number_result
IsTriangleNumber(entity *Entity, u32 Value)
{
    triangle_number_result Result = {};
    Result.IsTriangleNumber = false;

    // TODO(chowie): Is it possible to combine the prev and current cases?
    for(u32 TriangleIndex = 0;
        TriangleIndex < (EntityType_Count + 1);
        ++TriangleIndex)
    {
        // NOTE(chowie): Grabs whatever the closest previous triangle is
        if(Value == Entity->TriangleNumbersTable[TriangleIndex])
        {
            Result.IsTriangleNumber = true;

            if(Value != 0)
            {
                Result.PrevTriangleNumber = Entity->TriangleNumbersTable[TriangleIndex - 1];
            }
            else
            {
                // TODO(chowie): I hate having an explicit 0 check to guard for underflow!
                Result.PrevTriangleNumber = Entity->TriangleNumbersTable[TriangleIndex];
            }
            break;
        }
        else if(SignOf(Value - Entity->TriangleNumbersTable[TriangleIndex]) == -1)
        {
            // NOTE(chowie): Lookahead if TestValue < TriangleNumber,
            // if so PrevTriangle must be its row
            Result.IsTriangleNumber = false;

            // IMPORTANT(chowie): In exchange for an additional
            // triangle number and loop, no clamps are needed to
            // handle the 0 case! Even the way the signof check is
            // constructed; must always be past the first two index!
            Result.CurrTriangleNumber = Entity->TriangleNumbersTable[TriangleIndex - 1];
            Result.PrevTriangleNumber = Entity->TriangleNumbersTable[TriangleIndex - 2];
            break;
        }
    }

    return(Result);
}

// TODO(chowie): Change to v2enum
struct entity_pair_index_result
{
    u16 Row;
    u16 Col;
};
internal entity_pair_index_result
GetEntityPairFromIndex(entity *Entity, u32 Index)
{
    entity_pair_index_result Result = {};

    triangle_number_result TriangleNumber = IsTriangleNumber(Entity, Index);
    if(TriangleNumber.IsTriangleNumber)
    {
        Result.Col = 0;
    }
    else
    {
        Result.Col = (u16)(Index - TriangleNumber.CurrTriangleNumber);
    }
    Result.Row = (u16)(Index - Result.Col - TriangleNumber.PrevTriangleNumber);

    return(Result);
}

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

        entity Entity = {};

        //
        // NOTE(chowie): At startup
        //

        // NOTE(chowie): Generate tables
        // IMPORTANT(chowie): Think of triangle numbers like OnePastPitch.
        u32 PairTableIndex = 0;
        for(u32 Y = 0;
            Y < EntityType_Count;
            ++Y)
        {
            u32 Pitch = (Y + 1); // NOTE(chowie): N + 1
            for(u32 X = 0;
                X < Pitch;
                ++X, ++PairTableIndex)
            {
                // TODO(chowie): Figure out why ++PairTableIndex in loop body overwrites to the next memory?
                Entity.EntityTypePairTable[PairTableIndex] = PairTableIndex; // NOTE(chowie): Shortcut instead of MapEntityPairToIndex((u16)X, (u16)Y)
            }
            Entity.TriangleNumbersTable[Pitch] = PairTableIndex; // IMPORTANT(chowie): Rows must "begin" as a triangle number, skips first index abusing 0 initialisation
        }

        //
        // NOTE(chowie): At runtime
        //

        u32 EntityPair = MapEntityPairToIndex(EntityType_Wall, EntityType_Wall);
        entity_pair_index_result TestPair = GetEntityPairFromIndex(&Entity, EntityPair);
        char *Test = PushString(&TextArena, d7sam_concat("Testing Unpacking Pairs: ")(TestPair.Row)(", ")(TestPair.Col)("\n"));
        OutputDebugStringA(Test);

        /*
        // NOTE(chowie): String test
        char *Name = "Slim shady?";
        char *Test = PushString(&TextArena, d7sam_concat("Attention! ")(Name)("\n"));
        OutputDebugStringA(Test);
        */

        ClearArena(&TextArena);
    }

    return(0);
}

#endif
