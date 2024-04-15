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

inline memory_index
Clamp(memory_index Min, memory_index Value, memory_index Max)
{
    memory_index Result = Value;

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
//global_variable memory_index BufferPosition;
struct gap_buffer
{
    u8 *Buffer;
    memory_index Length;
    memory_index Start;
    memory_index End; // STUDY(chowie): Could use start and len/gap
    // TODO(chowie): Pass an arena allocator in! Check back in at 15:28!
};

internal memory_index
GetGapBufferLength(gap_buffer *GapBuffer)
{
    memory_index Result = GapBuffer->Length - (GapBuffer->End - GapBuffer->Start); // NOTE(chowie): Does not include gap
    return(Result);
}

#define GapBufferLenWithGap(GapBuffer) (GapBuffer->Length + GapBufferLength)

internal void
GapBufferInit(gap_buffer *GapBuffer, memory_index Size)
{
    GapBuffer->Buffer = (u8 *)malloc(Size);
    GapBuffer->Length = 0;
    GapBuffer->Start = 0;
    GapBuffer->End = Size;
}

// NOTE(chowie): Moves the Gap to the cursor position. Cursors are clamped [0,n) where n is the filled count of the buffer.
internal void
ShiftGapTo(gap_buffer *GapBuffer, memory_index Cursor)
{
    memory_index GapLength = GapBuffer->End - GapBuffer->Start;
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
            memory_index Delta = Cursor - GapBuffer->Start;
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
            memory_index Delta = GapBuffer->Start - Cursor;
            memcpy(&GapBuffer->Buffer[GapBuffer->End - Delta], &GapBuffer->Buffer[GapBuffer->Start - Delta], Delta);
            GapBuffer->Start -= Delta; // NOTE(chowie): Both buffers must move to the left
            GapBuffer->End -= Delta;
        }
    }
}

// NOTE(chowie): Verifies the buffer can hold the needed write. Resizes the array if not. By default doubles array size.
internal void
CheckGapSize(gap_buffer *GapBuffer, memory_index Required)
{
    memory_index GapLength = GapBuffer->End - GapBuffer->Start;
    if(GapLength < Required)
    {
        ShiftGapTo(GapBuffer, GapBuffer->Length - GapLength);
        memory_index RequiredBufferSize = Required + GapBuffer->Length - GapLength;
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
Remove(gap_buffer *GapBuffer, memory_index Cursor, u32 Count)
{
    u32 Delete = AbsoluteValue(Count);
    memory_index NewCursor = Cursor;
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
InsertChar(gap_buffer *GapBuffer, memory_index Cursor, u8 Char)
{
    CheckGapSize(GapBuffer, 1);
    ShiftGapTo(GapBuffer, Cursor);
    GapBuffer->Buffer[GapBuffer->Start++] = Char;
    GapBuffer->Length++;
}

internal void
InsertString(gap_buffer *GapBuffer, memory_index Cursor, char *String)
{
    CheckGapSize(GapBuffer, StringLength(String));
    ShiftGapTo(GapBuffer, Cursor);
    strcpy((char *)GapBuffer->Buffer, String); // TODO(chowie): This is totally not correct!
    GapBuffer->Start += StringLength(String);
}

// TODO(chowie): utf-8 support? How does one encode a rune in C? TIMESTAMP: 17:00-18:00
/*
internal void
InsertRune(gap_buffer *GapBuffer, memory_index Cursor, u8 Char)
{
    CheckGapSize(GapBuffer, 1);
    ShiftGapTo(GapBuffer, Cursor);
    GapBuffer->Buffer[GapBuffer->Start] = Char;
    GapBuffer->Start += 1;
}

// TODO(chowie): How does one insert a string in C? TIMESTAMP: 17:00-18:00
internal void
InsertString(gap_buffer *GapBuffer, memory_index Cursor, char *String)
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
    memory_index BufferLength = GetGapBufferLength(&GapBuffer);
    _snprintf_s(TextBuffer, sizeof(TextBuffer),
                "Buffer Length: %d\n", BufferLength);
    OutputDebugStringA(TextBuffer);
    return(0);
}
