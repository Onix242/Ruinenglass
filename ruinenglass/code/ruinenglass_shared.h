#if !defined(RUINENGLASS_SHARED_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Michael Chow $
   $Notice: $
   ======================================================================== */

#include "ruinenglass_intrinsics.h"
#include "ruinenglass_math.h"

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

inline b32x
StringsAreEqual(char *A, char *B)
{
    // NOTE: To pass in null pointers!
    b32x Result = (A == B);

    if(A && B)
    {
        // NOTE: Could do (*A++ == *B++) instead
        while(*A && *B && (*A == *B))
        {
            ++A;
            ++B;
        }
        // NOTE: Could do ((*A == *B) && (*A == 0)) instead
        Result = ((*A == 0) && (*B == 0));
    }

    return(Result);
}

inline b32x
StringsAreEqual(umm ALength, char *A, char *B)
{
    b32x Result = false;
    if(B)
    {
        char *At = B;
        for(umm Index = 0;
            Index < ALength;
            ++Index, ++At)
        {
            if((*At == 0) ||
               (A[Index] != *At))
            {
                return(false);
            }
        }

        Result = (*At == 0);
    }
    else
    {
        Result = (ALength == 0);
    }

    return(Result);
}

inline b32x
StringsAreEqual(umm ALength, char *A, umm BLength, char *B)
{
    // NOTE: To pass in null pointers!
    b32x Result = (ALength == BLength);

    if(Result)
    {
        Result = true;
        for(u32 Index = 0;
            Index < ALength;
            ++Index)
        {
            if(A[Index] != B[Index])
            {
                Result = false;
                break;
            }
        }
    }

    return(Result);
}

internal void
CatStrings(umm SourceACount, char *SourceA,
           umm SourceBCount, char *SourceB,
           umm DestCount, char *Dest)
{
    // TODO(chowie): Dest bound checking?
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

    *Dest++ = 0; // NOTE(chowie): Insertion of NULL terminator
}

// Probably put "umm CheckLength = StringLength(String);" back rather than args
inline char *
StringReverse(char *String)
{
    umm CheckLength = StringLength(String);
    char *Source = String + 0;
    char *Dest = String + CheckLength - 1;
    CheckLength /= 2;
    while(CheckLength--)
    {
        Swap(char, *Source, *Dest);
        Source++, Dest--;
    }
    return(String);
}

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

/* NOTE(chowie): Sample string usage
   char *name = "slim shady";
   int   line = 1337;
   float temp = -98.567f;
   //OutputDebugStringA(d7sam_concat(line)(" attention!\n"));
   //OutputDebugStringA(d7sam_concat(" attention!")(line)("\n"));

   // attention, slim shady! there's an error in line 1337 : the error code is 666 and the temperature is -98.6 degrees
   OutputDebugStringA(d7sam_concat("attention, ")(name)("! there's an error in line ")(line)(" : the error code is ")(666)(" and the temperature is ")(temp, 1)(" degrees\n"));
*/

#define CONCAT_BUFFER_SIZE 256
#define Base10 10
global r32 Bases[] = { 1, 10, 100, 1000, 10000, 100000, 1000000 };
// RESOURCE: https://gist.github.com/d7samurai/1d778693ba33bbd2b9d709b209cc0aba
// TODO(chowie): Convert to using arenas!
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
            if(CharIndex == (CharCount - Decimals - 1))
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

/* STUDY(chowie): Alternative to printf. Takes sizeof(Buffer) prevents
   passing a buffer that's too small that would overwrite the end of
   memory.

   char TextBuffer[256];
   _snprintf_s(TextBuffer, sizeof(TextBuffer),
   "Last Frame Time: %.02fms/f\n", MSPerFrame);
   OutputDebugStringA(TextBuffer);
*/

#define RUINENGLASS_SHARED_H
#endif
