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
#include "ruinenglass_random.h"
#include "ruinenglass_hash.h"
#include "ruinenglass_stats.h"

//
// String
//

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
        foreachN(u32, Index, ALength)
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

internal char
ToLowercase(char Char)
{
    char Result = Char;

    if((Result >= 'A') && (Result <= 'Z'))
    {
        Result += 'a' - 'A';
    }

    return(Result);
}

inline b32x
StringsAreEqualLowercase(umm ALength, char *A, umm BLength, char *B)
{
    // NOTE: To pass in null pointers!
    b32x Result = (ALength == BLength);

    if(Result)
    {
        Result = true;
        foreachN(u32, Index, ALength)
        {
            if(ToLowercase(A[Index]) != ToLowercase(B[Index]))
            {
                Result = false;
                break;
            }
        }
    }

    return(Result);
}

internal b32x
StringsAreEqual(string A, char *B)
{
    b32x Result = StringsAreEqual(A.Size, (char *)A.Data, B);
    return(Result);
}

internal b32x
StringsAreEqual(string A, string B)
{
    b32x Result = StringsAreEqual(A.Size, (char *)A.Data, B.Size, (char *)B.Data);
    return(Result);
}

internal u32
StringHashOf(char *Z)
{
    u32 Result = 0;
    while(*Z)
    {
        Result = DJB2Hash(*Z++);
    }

    return(Result);
}

internal u32
StringHashOf(string String)
{
    u32 HashValue = 0;
    
    foreachN(umm, Index, String.Size)
    {
        HashValue = DJB2Hash(String.Data[Index]);
    }
    
    return(HashValue);
}

// NOTE(chowie): atoi
internal s32
S32FromZ(char *At)
{
    s32 Result = 0;

    while((*At >= '0') &&
          (*At <= '9'))
    {
        Result *= 10; // STUDY(chowie): Read first things we see of string, assumes a digit. For every place we move, digits (accumulator) should increase up by pow of 10
        Result += (*At - '0');
        ++At;
    }

    return(Result);
}

internal void
CatStrings(umm SourceACount, char *SourceA,
           umm SourceBCount, char *SourceB,
           umm DestCount, char *Dest)
{
    // TODO(chowie): Dest bound checking?
    foreachN(u32, Index, SourceACount)
    {
        *Dest++ = *SourceA++;
    }

    foreachN(u32, Index, SourceBCount)
    {
        *Dest++ = *SourceB++;
    }

    *Dest++ = 0; // NOTE(chowie): Insertion of NULL terminator
}

// Probably put "umm CheckLength = StringLength(String);" back rather than args
inline char *
StringReverse(char *String)
{
    umm CheckLength = StringLen(String);
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
/* NOTE(chowie): Could write it like
   while(Start < End)
   {
       --End;
       char Temp = *End;
       *End = *Start;
       *Start = Temp;
       ++Start;
   }
*/

// RESOURCE(): Not that good - https://stackoverflow.com/questions/497018/is-there-a-function-to-round-a-float-in-c-or-do-i-need-to-write-my-own#comment312776_497037
inline f32
RoundDecimal(f32 Value, u8 Decimals = 2)
{
    s32 Log10 = (s32)Powi(10, Decimals);
    f32 Result = Round(Log10*Value) / Log10;
    return(Result);
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

// RESOURCE(): https://gist.github.com/d7samurai/140807683843a06195c33494e3546a84
// TODO(chowie): Add the string to float conversion!

/* NOTE(chowie): Sample string usage
   char *name = "slim shady";
   int   line = 1337;
   float temp = -98.567f;
   //OutputDebugStringA(d7sam_concat(line)(" attention!\n"));
   //OutputDebugStringA(d7sam_concat(" attention!")(line)("\n"));

   // attention, slim shady! there's an error in line 1337 : the error code is 666 and the temperature is -98.6 degrees
   OutputDebugStringA(d7sam_concat("attention, ")(name)("! there's an error in line ")(line)(" : the error code is ")(666)(" and the temperature is ")(temp, 1)(" degrees\n"));
*/

// STUDY(chowie): It's easier to print out to a temp buffer, to do
// whatever you want and reconstruct the number e.g. you can take a
// high number and pad to low number
#define TEMP_BUFFER_SIZE 1024
#define Base10 10
global f32 Bases[] = { 1, 10, 100, 1000, 10000, 100000, 1000000 };
// RESOURCE: https://gist.github.com/d7samurai/1d778693ba33bbd2b9d709b209cc0aba
// TODO(chowie): Convert to using arenas!
// TODO(chowie): This hideous functions is really convenient! Probably only use this for debugging only!
struct d7sam_concat
{
    d7sam_concat(char* Source) { operator()(Source); }
    d7sam_concat(s32 Value) { operator()(Value); }
    d7sam_concat(f32 Value, u32 Decimals = 2) { operator()(Value, Decimals); }

    u32 CharCount = 0;
    char TempBuffer[TEMP_BUFFER_SIZE];

    d7sam_concat &
    operator()(char* Source)
    {
        // TODO(chowie): How do I remove the null terminator?
        // NOTE(chowie): Include null terminator
        u32 Size = StringLen(Source) + 1;

        foreachN(u32, CharIndex, Size)
        {
            TempBuffer[CharCount++] = Source[CharIndex];
        }
        CharCount--;

        return(*this);
    }

    // TODO(chowie): Compare this vs HmH OpenGLParseNumber
    d7sam_concat &
    operator()(s32 Value)
    {
        b32x Negative = (Value < 0);
        if(Negative)
        {
            Value = -Value;
        }

        CharCount += NumDigitsLog10(Value) + Negative;
        s32 CharIndex = CharCount;

        // STUDY(chowie): Process digits backwards; must flip buffer!
        TempBuffer[CharIndex--] = 0; // STUDY(chowie): Null terminate at the end (even if buffer is full)
        do {
            TempBuffer[CharIndex--] = '0' + (Value % Base10); // STUDY(chowie): Value % Base -> 1st Digit
            Value /= Base10; // STUDY(chowie): Value / Base -> Moves one digit down e.g. 123 -> 12
        } while(Value);

        if(Negative)
        {
            TempBuffer[CharIndex] = '-';
        }

        return(*this);
    }

    d7sam_concat &
    operator()(f32 Value, u32 Decimals = 2)
    {
        b32x Negative = (Value < 0);
        if(Negative)
        {
            Value = -Value;
        }

        u32 CastValue = RoundF32ToU32(Bases[Decimals]*Value);
        u32 MaxDecimals = Max(NumDigitsLog10(CastValue), (s32)Decimals + 1);
        CharCount += MaxDecimals + Negative + (Decimals > 0);
        u32 CharIndex = CharCount;

        TempBuffer[CharIndex--] = 0;
        do {
            TempBuffer[CharIndex--] = '0' + (CastValue % Base10);
            CastValue /= Base10; // STUDY(chowie): For floats, dividing by 10 is destructive and precision may be loss because 10 isn't a power of 2!
            if(CharIndex == (CharCount - Decimals - 1))
            {
                TempBuffer[CharIndex--] = '.';
            }
        } while(CastValue || ((CharCount - CharIndex) <= (Decimals ? Decimals + 2 : 0)));

        if(Negative)
        {
            TempBuffer[CharIndex] = '-';
        }

        return(*this);
    }

    operator char* ()
    {
        return(TempBuffer);
    }
};

// TODO(chowie): String To Float - https://gist.github.com/d7samurai/140807683843a06195c33494e3546a84
/*
f64 string_to_float(char* str)
{
    f64 num = 0.0;
    f64 mul = 1.0;
    s32    len = 0;
    s32    dec = 0;

    while (str[len]) if (str[len++] == '.') dec = 1;

    for (s32 idx = len - 1; idx >= 0; idx--)
    {
        if      (str[idx] == '-') num = -num;
        else if (str[idx] == '.') dec = 0; 
        else if (dec)
        {
            num += str[idx] - '0';
            num *= 0.1;
        }
        else
        {
            num += (str[idx] - '0') * mul;
            mul *= 10.0;
        }
    }

    return num;
}
*/

/* STUDY(chowie): Alternative to printf. Takes sizeof(Buffer) prevents
   passing a buffer that's too small that would overwrite the end of
   memory.

   char TextBuffer[256];
   _snprintf_s(TextBuffer, sizeof(TextBuffer),
   "Last Frame Time: %.02fms/f\n", MSPerFrame);
   OutputDebugStringA(TextBuffer);
*/

//
// Sort
//
// RESOURCE(): https://web.archive.org/web/20190821115842/https://gist.github.com/mmozeiko/0bd42648536b164f5dc8
// RESOURCE(): https://www.toptal.com/developers/sorting-algorithms
// RESOURCE(): https://hero.handmade.network/forums/code-discussion/t/984-day_229__what_about_radix_sort

internal void
InsertionSort(f32* Entries, u32 Start, u32 End)
{
    for(u32 Index = Start + 1;
        Index < End;
        Index++)
    {
        f32 Value = Entries[Index];
        if(Entries[Index - 1] > Value)
        {
            u32 Index2 = Index;
            do
            {
                Entries[Index2] = Entries[Index2 - 1];
            }
            while(--Index2 > Start && Entries[Index2 - 1] > Value);
            Entries[Index2] = Value;
        }
    }
}

internal void
InsertionSort(u32* Entries, u32 Start, u32 End)
{
    for(u32 Index = Start + 1;
        Index < End;
        ++Index)
    {
        u32 Value = Entries[Index];
        if(Entries[Index - 1] > Value)
        {
            u32 Index2 = Index;
            do
            {
                Entries[Index2] = Entries[Index2 - 1];
            }
            while(--Index2 > Start && Entries[Index2 - 1] > Value);
            Entries[Index2] = Value;
        }
    }
}

inline u32
F32ToU32Key(f32 Key)
{
    u32 KeyBits = *((u32*)&Key);
    u32 Mask = -(s32)(KeyBits >> 31) | 0x80000000;
    KeyBits ^= Mask;
    return(KeyBits);
}

inline u32
GetByteN(u32 Value, u32 ByteIndex)
{
    return((Value >> (8*ByteIndex)) & 0xFF);
}

// NOTE(chowie): Unstable and is a little slower than just normal
// radix, but you have to allocate memory of course...
internal void
RadixSortInPlaceImplOpt(f32 *Entries, u32 Start, u32 End, u32 DigitIndex)
{
    u32 Counts[256] = {};

    // NOTE(chowie): First pass - count how many of each key
    for(u32 Index = Start;
        Index < End;
        ++Index)
    {
         u32 Key = F32ToU32Key(Entries[Index]);
         u32 Digit = GetByteN(Key, DigitIndex);
         Counts[Digit]++;
    }

    // NOTE(chowie): Change counts to offset
    // NOTE(chowie): E.g. SortKey 0 -> 3 of them, SortKey 2 -> 5.
    // 0 | 0 | 0 | 2 | 2 | 2 | 2 | 2
    u32 Offsets[256];
    Offsets[0] = Start;
    for(u32 Index = 1;
        Index < 256;
        ++Index)
    {
        Offsets[Index] = Counts[Index - 1] + Offsets[Index - 1];
    }

    // NOTE(chowie): Second pass - place elements into the right location
    foreachN(u32, Index, 256)
    {
        while(Counts[Index] > 0)
        {
            u32 Original = Offsets[Index];
            u32 Source = Original;
            f32 Key = Entries[Source];
            do
            {
                u32 Digit = GetByteN(F32ToU32Key(Key), DigitIndex);
                u32 Target = Offsets[Digit]++;
                Counts[Digit]--;

                Swap(f32, Entries[Target], Key);

                Source = Target;
            }
            while(Source != Original);
        }
    }

    if(DigitIndex > 0)
    {
        foreachN(u32, Index, 256)
        {
            u32 NewStart = (Index == 0 ? Start : Offsets[Index - 1]);
            u32 NewEnd = Offsets[Index];
            if(NewEnd - NewStart > 1)
            {
                if(NewEnd - NewStart < 64)
                {
                    InsertionSort(Entries, NewStart, NewEnd);
                }
                else
                {
                    RadixSortInPlaceImplOpt(Entries, NewStart, NewEnd, DigitIndex - 1);
                }
            }
        }
    }
}

// RESOURCE(): https://www.codercorner.com/RadixSortRevisited.htm
// NOTE(chowie): Use for:
// - Sorting transparent polygons
// - Collision detection e.g. sweep and prune
// - Histograms
internal void
RadixSortInPlaceOpt(f32 *Entries, u32 Count)
{
    RadixSortInPlaceImplOpt(Entries, 0, Count, 3);
}

// COULDDO(chowie): If the list gets larger, transforms this into 4
// passes. 256->2048 to process 11-bits at a time. Thus you only need,
// 3 arrays, not 4. (for million+ data to sort).
// NOTE(chowie): Improves over radix at count = 4096
internal void
RadixSort5n(f32 *Entries, f32* Temp, u32 Count)
{
    f32 *Source = Entries;

    // NOTE(chowie): First pass - count how many of each key
    u32 Counts[4][256] = {};
    foreachN(u32, Index, Count)
    {
        u32 Key = F32ToU32Key(Source[Index]);
        Counts[0][GetByteN(Key, 0)]++;
        Counts[1][GetByteN(Key, 1)]++;
        Counts[2][GetByteN(Key, 2)]++;
        Counts[3][GetByteN(Key, 3)]++;
    }

    foreachN(u32, DigitIndex, 4)
    {
        u32 TotalCount = 0;
        foreachN(u32, Index, 256)
        {
            u32 CurrentCount = Counts[DigitIndex][Index];
            Counts[DigitIndex][Index] = TotalCount;
            TotalCount += CurrentCount;
        }
    }

    // NOTE(chowie): Second pass - place elements into the right location
    foreachN(u32, DigitIndex, 4)
    {
        foreachN(u32, Index, Count)
        {
            u32 Key = F32ToU32Key(Source[Index]);
            u32 Digit = GetByteN(Key, DigitIndex);

            Temp[Counts[DigitIndex][Digit]++] = Source[Index];
        }

        Swap(f32 *, Source, Temp);
    }
}

internal void
RadixSort5n(u32 *Entries, u32* Temp, u32 Count)
{
    u32 *Source = Entries;

    // NOTE(chowie): First pass - count how many of each key
    u32 Counts[4][256] = {};
    foreachN(u32, Index, Count)
    {
        Counts[0][GetByteN(Source[Index], 0)]++;
        Counts[1][GetByteN(Source[Index], 1)]++;
        Counts[2][GetByteN(Source[Index], 2)]++;
        Counts[3][GetByteN(Source[Index], 3)]++;
    }

    foreachN(u32, DigitIndex, 4)
    {
        u32 TotalCount = 0;
        foreachN(u32, Index, 256)
        {
            u32 CurrentCount = Counts[DigitIndex][Index];
            Counts[DigitIndex][Index] = TotalCount;
            TotalCount += CurrentCount;
        }
    }

    // NOTE(chowie): Second pass - place elements into the right location
    foreachN(u32, DigitIndex, 4)
    {
        foreachN(u32, Index, Count)
        {
            u32 Digit = GetByteN(Source[Index], DigitIndex);

            Temp[Counts[DigitIndex][Digit]++] = Source[Index];
        }

        Swap(u32 *, Source, Temp);
    }
}

#define RUINENGLASS_SHARED_H
#endif
