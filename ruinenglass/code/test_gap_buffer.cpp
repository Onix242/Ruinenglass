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
#include <intrin.h>

#if !defined(internal)
#define internal static
#endif
#define local_persist static
#define global static

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

#define SID(string) string
typedef u32 sid;

#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value)*1024LL)
#define Gigabytes(Value) (Megabytes(Value)*1024LL)
#define Terabytes(Value) (Gigabytes(Value)*1024LL)

#define Len(arr)(sizeof((arr)) /(sizeof((arr)[0])))
#define Min(A, B) ((A < B) ? (A) : (B))
#define Max(A, B) ((A > B) ? (A) : (B))

#define Assert(Expression) if(!(Expression)) {*(int *)0 = 0;}
#define InvalidCodePath Assert(!"InvalidCodePath")
#define InvalidDefaultCase default: {InvalidCodePath;} break

#define NotImplemented Assert(!"NotImplemented")

#define Swap(type, A, B) {type Temp = (A); (A) = (B); (B) = Temp;}

#define foreachN(type, Value, array) for(type Value = 0; Value < array; ++Value)
#define foreach(type, Value, array) for(type Value = 0; Value < Len(array); ++Value)

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
StringLen(char *String)
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

// TODO(chowie): Not sure if I need this to squish strings
internal void
CatStrings(size_t SourceACount, char *SourceA,
           size_t SourceBCount, char *SourceB,
           size_t DestCount, char *Dest)
{
    // TODO: Dest bound checking
    foreachN(u32, Index, SourceACount)
    {
        *Dest++ = *SourceA++;
    }
    foreachN(u32, Index, SourceBCount)
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
AbsDifferenceClampAboveZero(u32 A, u32 B)
{
    __m128i NewA = _mm_set1_epi32((s32)A);
    __m128i NewB = _mm_set1_epi32((s32)B);
    __m128i AB = _mm_subs_epu8(NewA, NewB);
    __m128i BA = _mm_subs_epu8(NewB, NewA);

    __m128i Result = _mm_or_si128(AB, BA);
    return((u32)_mm_extract_epi8(Result, 0));
}

inline u32
ClearLeftMostSet(u32 value)
{
    Assert(value != 0);
    return(value & (value - 1));
}

struct bit_scan_result
{
    b32x Found;
    u32 Index;
};
inline bit_scan_result
FindLeastSignificantBit(u32 Value) // NOTE(chowie): ctz
{
    bit_scan_result Result = {};
    Result.Found =  _BitScanForward((unsigned long *)&Result.Index, Value);
    return(Result);
}

inline bit_scan_result
FindMostSignificantBit(u32 Value) // NOTE(chowie): clz
{
    bit_scan_result Result = {};
    Result.Found =  _BitScanReverse((unsigned long *)&Result.Index, Value);
    return(Result);
}

//
//
//

// RESOURCE(): https://gist.github.com/Doy-lee/0c4d23af4afc51ea8b03b60d0af9a1fb
// TODO(chowie): Try using error sinks!

// RESOURCE(): https://randygaul.github.io/api-design/2019/04/26/Error-Codes-and-Error-Handling.html
// E.g.
// error_t err = do_something(params);
// if (err.is_error()) {
//     handle_error(err.details);
// }

struct error_code
{
    b32x Code; // NOTE(chowie): 1 is success, 0 is failure
    char *Details; // NOTE(chowie): Don't localise
};

inline b32x
IsError(error_code Error)
{
    b32x Result = (Error.Code == false);
    return(Result);
}

inline error_code
ErrorMake(b32x Code, char *Details)
{
    error_code Result =
    {
        Code,
        Details,
    };

    return(Result);
}

inline error_code
ErrorFailure(char *Details)
{
    error_code Result = ErrorMake(false, Details);
    return(Result);
}

inline error_code
ErrorSuccess(void)
{
    error_code Result = ErrorMake(true, 0);
    return(Result);
}

//
//
//

// RESOURCE(): https://github.com/mojobojo/smaz/blob/master/smaz.c
// TODO(chowie): String compression

// RESOURCE(): https://handmade.network/forums/t/8696-implementing_ctype.h_functions_using_bit_operations.
inline b32x
IsLower(char c)
{
    b32x Result = (c >= 'a') && (c <= 'z');
    return(Result);
}

inline b32x
IsUpper(char c)
{
    b32x Result = (c >= 'A') && (c <= 'Z');
    return(Result);
}

inline b32x
IsDigit(char c)
{
    b32x Result = (c >= '0') && (c <= '9');
    return(Result);
}

inline b32x
IsAlpha(char c)
{
    b32x Result = ((c ^ 0x40) - 1) < 0x5B; // NOTE(chowie): 
    return(Result);
}

inline b32x
IsAlnum(char c)
{
    b32x Result = IsDigit(c) || IsAlpha(c);
    return(Result);
}

inline s32
ToDigit(char c)
{
    s32 Result = c - '0';
    return(Result);
}

inline char
ToChar(u32 i)
{
    char Result = (char)(i + '0');
    return(Result);
}

// RESOURCE(): http://0x80.pl/notesen/2016-01-06-swar-swap-case.html
internal void
ToUpper(char s)
{
    if(IsLower(s))
    {
        s ^= (1 << 5);
    }
}

internal void
ToUpper(char *s, umm n)
{
    foreachN(umm, j, n)
    {
        if(IsLower(s[j]))
        {
            s[j] ^= (1 << 5);
        }
    }
}

internal void
ToLower(char s)
{
    if(IsUpper(s))
    {
        s ^= (1 << 5);
    }
}

internal void
ToLower(char *s, umm n)
{
    foreachN(umm, j, n)
    {
        if(IsUpper(s[j]))
        {
            s[j] ^= (1 << 5);
        }
    }
}

// RESOURCE(): http://0x80.pl/notesen/2016-12-21-swar-digits-validate.html
// COULDDO(chowie): SSE?
internal b32x
IsAllDigits(char* String, umm Size)
{
    b32x Result = true;
    foreachN(umm, i, Size)
    {
        u8 c = (u8)(String[i] - '0');
        if(c > 9)
        {
            Result = false;
            break;
        }
    }

    return(Result);
}

// RESOURCE(): https://theartincode.stanis.me/008-djb2/
// RESOURCE(): https://cowboyprogramming.com/2007/01/04/practical-hash-ids/
// RESOURCE(): https://randygaul.github.io/data-structures/architecture/hash/2015/12/11/Preprocessed-Strings-for-Asset-IDs.html
// TODO(chowie): Try preprocessing step on strings to be made as hash

// NOTE(from randygaul): This allows to hash a prefix string, and then
// feed the result off to hash the suffix. Mick explains this in his
// article this idea of combining hashed values as a useful feature
// for supporting tools and assets. This can be implemented both at
// compile or run-time (though I haven't quite thought of a need to do
// this at compile-time yet)
//
// In other words, given a hash value from a string A (HASH(A)), and
// given another string B, then you can calculate HASH(A+B) by
// calculating HASH(B) using a starting value of HASH(A). This means
// you can calculate HASH(A+B) without ever knowing the actual
// contents of the string A. This is very useful if you have assets
// that have a series of different extensions or other appended
// strings (e.g.: MDL_ROBOT_01_ might have MDL_ROBOT_01_LEFT_ARM,
// MDL_ROBOTO_01_LEFT_LEG, etc.) You can quickly calculate the hash
// values of the string with the extensions without having to know the
// original string.
//
// COULDDO(chowie): Integer hashed strings when assets are "locked for
// release" and store important strings in a debug table.

#define DJB2HASH_MAGICNUMBER 5381

// COULDDO(chowie): Do I care enough to support case-sensitive hash? (Remove ToLower)
inline u32
Hash(u32 Prefix, char Suffix)
{
    ToLower(Suffix);
    u32 Result = ((Prefix << 5) + Prefix) + Suffix;
    return(Result);
}

inline u32
Hash(char Suffix)
{
    u32 Result = Hash(DJB2HASH_MAGICNUMBER, Suffix);
    return(Result);
}

// COULDDO(chowie): I don't think suffix or infix is even possible?

// IMPORTANT(chowie): Case-insensitive hash by default
inline u32
Hash(u32 Prefix, char *Suffix)
{
    u32 Result = Prefix;
    for(char *Scan = Suffix;
        *Scan;
        Scan++)
    {
        Result = Hash(Result, *Scan);
    }

    return(Result);
}

inline u32
Hash(char *Suffix)
{
    u32 Result = Hash(DJB2HASH_MAGICNUMBER, Suffix);
    return(Result);
}

// NOTE(chowie): To guard for collisions/debug table.
inline u32
CheckHash_(u32 HashID, char *Name)
{
    Assert(HashID == Hash(Name));
    return(HashID);
}

#ifdef RUINENGLASS_INTERNAL
#define CheckHash(HashID, Name) CheckHash_(HashID, Name)
#else
#define CheckHash(HashID, Name) HashID
#endif

struct buffer
{
    umm Size;
    u8 *Data;
};
typedef buffer string;

#define CONSTANT_STRING(String) {sizeof(String) - 1, (u8 *)(String)}

internal string
WrapZ(char *Z)
{
    string Result;

    Result.Size = StringLen(Z);
    Result.Data = (u8 *)Z;

    return(Result);
}

internal string
BundleString(char *Z, umm Size)
{
    string Result;

    Result.Size = Size;
    Result.Data = (u8 *)Z;

    return(Result);
}

// RESOURCE(): https://chrisgreendevelopmentblog.wordpress.com/2024/10/03/approaches-for-efficient-unique-symbols-in-c/
enum sym_type : u64
{
    NIL = 0,
    QUIT = 1,
    SAVE = 2,

    INVALID = ~0ull,
};
// TODO(chowie): Preprocessed strings and a generate .h file!
// e.g. constexpr Symbol_t MA_STRENGTH = Symbol_t( 0x55aa348978 );  //Hash function calculated by preprocessor

// COULDDO(chowie): Convert birdfont .ttf to default lowercase?

// COULDDO(chowie): Seems difficult, replace "global char" with "global string"?
global char Radix40ToAscii[]        = "?0123456789_abcdefghijklmnopqrstuvwxyz"; // 38/40
global char ConlangRadix40ToAscii[] = "?(![{,%}]?)_abcdefghijklmnopqrstuvwxyz"; // 38/40

//
// CONLANG STRING ENCODING:
//
// - 12 char max (u8)
//
// _ _|_ _ _ _ _ _ _ _ _ _
// 0 1 2 3 4 5 6 7 8 9 10 11
//
// - byte 0    = Conlang type e.g. Base 10 Numbers, Eurolang, Asialang, Calclang etc
// - byte 1    = Particle of speech/primary tag e.g. verb, noun, place etc
// - byte 2-11 = Data/Conlang word
//

// TODO(chowie): Properly parse string

// TODO(chowie): History chain word list + random fair shuffle?
//    = RESOURCE(): Shuffling a POSET - https://www.youtube.com/watch?v=dr-jUCelobk
//      > Topological sort in two passes odd and even in pairs
//      > Useful for randomising a list fairly while still mailing certain ordered rules.
//      > Might be useful for language learning e.g. transitive verbs are tested before intransitive!
//      > Even/Odd int http://marc-b-reynolds.github.io/math/2022/12/16/ParityWalks.html
//      > https://marc-b-reynolds.github.io/math/2022/01/28/RNGParity.html
//        ~"u  |= 1; return u ^ (u >> 1);" = "return u ^ rotl(u,1)" // 3u
//        ~"u <<= 1; return u ^ (u >> 1);" = "return u ^ rotl(u,1) ^ 1" // 3u + 1

constexpr s32
ToRadix40(char c)
{
    // NOTE(chowie): Normal order, 0-9, Special char, a-z. Optimised
    // if statements for short-circuiting.

    s32 Result = 0;
    if((c >= 'a') && (c <= 'z'))
    {
        Result = 1 + 10 + 1 + (c - 'a');
    }
    else if((c >= 'A') && (c <= 'Z'))
    {
        Result = 1 + 10 + 1 + (c - 'A');
    }
    else if(( c >= '0' ) && ( c <= '9' ))
    {
        Result = 1 + c - '0';
    }
    else
    {
        // NOTE(chowie): Offsets 1 + 10
        switch(c)
        {
            case '(':
            {
                Result = 1;
            } break;

            case '!':
            {
                Result = 1 + 1;
            } break;

            case '[':
            {
                Result = 1 + 2;
            } break;

            case '{':
            {
                Result = 1 + 3;
            } break;

            case ',':
            {
                Result = 1 + 4;
            } break;

            case '%':
            {
                Result = 1 + 5;
            } break;

            case '}':
            {
                Result = 1 + 6;
            } break;

            case ']':
            {
                Result = 1 + 7;
            } break;

            case '?':
            {
                Result = 1 + 8;
            } break;

            case ')':
            {
                Result = 1 + 9;
            } break;

            case '_':
            {
                Result = 1 + 10;
            } break;
        };
    }

    return(Result);
}

internal u64
ToRadix40(string String)
{
    Assert(String.Size <= 12);

    u64 Result = 0;
    for(u8 *Scan = String.Data;
        *Scan;
        Scan++)
    {
        Result = 40*Result + ToRadix40(*Scan);
    }

    return(Result);
}

internal string
FromRadix40(u64 Radix, string Source, char *Ref = Radix40ToAscii) // sym_type Radix
{
    Assert(Radix != 0);
    Assert(Source.Size >= 15);

    string Result = Source;
    *(--Result.Data) = '\0'; // TODO(chowie): Unnecessary? To remove because of pascal-style strings
    while(Radix)
    {
        *(--Result.Data) = Ref[Radix % 40];
        Radix /= 40;
    }

    if(Ref == Radix40ToAscii)
    {
        *(--Result.Data) = '$'; // COULDDO(chowie): Optional symbol indicator, "sym_"?
    }

    return(Result);
}

//
//
//

// RESOURCE: https://www.youtube.com/watch?v=agUiYkvkoVg
// https://0xkiire.com/text_editor_data_structures1/
// TODO(chowie): Implement ropes with unicode!

// RESOURCE(): https://web.archive.org/web/20200917053102/https://github.com/RandyGaul/cute_headers/blob/master/cute_utf.h
// Want to convert between utf-8 and utf-16!
// IMPORTANT(chowie): TODO(chowie): Above library is for game localization specifically!

struct gap_buffer
{
    string Buffer;
    umm Start;
    umm End;
};

internal umm
GapSize(gap_buffer *GapBuffer)
{
    umm Result = GapBuffer->End - GapBuffer->Start;
    return(Result);
}

internal umm
GapLen(gap_buffer *GapBuffer)
{
    umm Result = GapBuffer->Buffer.Size - GapSize(GapBuffer);
    return(Result);
}

#define INITIAL_GAP 16

internal gap_buffer *
InitGap(void)
{
    gap_buffer *GapBuffer = (gap_buffer *)malloc(sizeof(gap_buffer));
    GapBuffer->Buffer.Size = INITIAL_GAP;
    GapBuffer->Buffer.Data = (u8 *)malloc(GapBuffer->Buffer.Size);
    GapBuffer->Start = 0;
    GapBuffer->End = GapBuffer->Buffer.Size;
    return(GapBuffer);
}

internal gap_buffer *
InitGapFromString(char *String)
{
    gap_buffer *GapBuffer = InitGap();
    while(*String)
    {
        GapBuffer->Buffer.Data[GapBuffer->Start++] = *String++;
    }

    return(GapBuffer);
}

internal void
ShiftGapTo(gap_buffer *GapBuffer, umm Cursor)
{
    Assert(Cursor >= 0 && Cursor <= GapLen(GapBuffer));

    if(Cursor < GapBuffer->Start)
    {
        umm Delta = GapBuffer->Start - Cursor;
        memmove(GapBuffer->Buffer.Data + GapBuffer->End - Delta,
                GapBuffer->Buffer.Data + Cursor,
                Delta);
        GapBuffer->Start -= Delta;
        GapBuffer->End -= Delta;
    }
    else if(Cursor > GapBuffer->Start)
    {
        umm Delta = Cursor - GapBuffer->Start;
        memmove(GapBuffer->Buffer.Data + GapBuffer->Start,
                GapBuffer->Buffer.Data + GapBuffer->End,
                Delta);
        GapBuffer->Start += Delta;
        GapBuffer->End += Delta;
    }
}

// NOTE(chowie): Gap shrinks, cursor advances
internal void
InsertCharInternal(gap_buffer *GapBuffer, char C)
{
    GapBuffer->Buffer.Data[GapBuffer->Start++] = C;
}

internal void
InsertChar(gap_buffer *GapBuffer, umm Cursor, char C)
{
    ShiftGapTo(GapBuffer, Cursor);
    InsertCharInternal(GapBuffer, C);
}

internal void
InsertString(gap_buffer *GapBuffer, umm Cursor, char *String)
{
    ShiftGapTo(GapBuffer, Cursor);
    while(*String)
    {
        InsertCharInternal(GapBuffer, *String++);
    }
}

internal void
GapDelete(gap_buffer *GapBuffer, umm Cursor, u32 Count = 1, b32x Pre = true)
{
    ShiftGapTo(GapBuffer, Cursor);
    if(Pre)
    {
        if((GapBuffer->Start + Count) > 0)
        {
            GapBuffer->Start -= Count; // NOTE(chowie): Expand gap to left
        }
    }
    else
    {
        // NOTE(chowie): Post
        if((GapBuffer->End + Count) < GapBuffer->Buffer.Size)
        {
            GapBuffer->End += Count; // NOTE(chowie): Expand gap to right
        }
    }
}

// NOTE(chowie): Random read
internal char
GetGapBuffer(gap_buffer *GapBuffer, umm Cursor)
{
    Assert(Cursor >= 0 && Cursor <= GapLen(GapBuffer));
    char Result = 0;
    if(Cursor < GapBuffer->Start)
    {
        Result = GapBuffer->Buffer.Data[Cursor];
    }
    else
    {
        Result = GapBuffer->Buffer.Data[GapBuffer->End + (Cursor - GapBuffer->Start)];
    }

    return(Result);
}

internal char *
GapBufferToString(gap_buffer *GapBuffer)
{
    umm Len = GapLen(GapBuffer);
    char *Result = (char *)malloc(Len + 1);

    memcpy(Result, GapBuffer->Buffer.Data, GapBuffer->Start);
    umm Right = GapBuffer->Buffer.Size - GapBuffer->End;
    memcpy(Result + GapBuffer->Start, GapBuffer->Buffer.Data + GapBuffer->End, Right);
    Result[Len] = '\0';

    return(Result);
}

internal void
GapBufferDump(gap_buffer *GapBuffer)
{
    char TextBuffer[64];
    _snprintf_s(TextBuffer, sizeof(TextBuffer),
                "capacity=%d gap=[%d,%d) len=%d cursor=%d\n  [",
                GapBuffer->Buffer.Size, GapBuffer->Start, GapBuffer->End,
                GapLen(GapBuffer), GapBuffer->Start);
    OutputDebugStringA(TextBuffer);

    foreachN(s32, i, GapBuffer->Buffer.Size)
    {
        if(i == GapBuffer->Start)
        {
            char TextBufferHeader[8];
            _snprintf_s(TextBufferHeader, sizeof(TextBufferHeader),
                        " |GAP| ");
            OutputDebugStringA(TextBufferHeader);
        }
        if(i >= GapBuffer->Start && i < GapBuffer->End) {
            char TextBufferHeader[4];
            _snprintf_s(TextBufferHeader, sizeof(TextBufferHeader),
                        "_ ");
            OutputDebugStringA(TextBufferHeader);
        }
        else
        {
            char TextBufferHeader[64];
            _snprintf_s(TextBufferHeader, sizeof(TextBufferHeader),
                        "%c", GapBuffer->Buffer.Data[i]);
            OutputDebugStringA(TextBufferHeader);
        }
    }

    char TextBufferHeader[4];
    _snprintf_s(TextBufferHeader, sizeof(TextBufferHeader),
                "]\n");
    OutputDebugStringA(TextBufferHeader);
}

// TODO(chowie): Abstract the text buffer to switch between gap buffers + ropes
// RESOURCE: https://www.youtube.com/watch?v=_8tVObGKQ3k
struct text_buffer
{
    gap_buffer GapBuffer;
    umm Cursor;

    u32 LineCount;
    u32 *Lines;
};

// COULDDO(chowie): Clean-up? O(n) for now!
internal void
CalcLines()
{
}

//
//
//

// TODO(chowie): String version?
// RESOURCE(): http://0x80.pl/notesen/2023-11-20-popcount-suggestions.html
// IMPORTANT(chowie): Alternative to Levenshtein Distance for fuzzy match!
internal void
ByteHistogram(u32 *Histogram, char *s)
{
    foreachN(u32, i, StringLen(s))
    {
        u32 b = s[i];
        Histogram[b]++;
    }
}

#define ASCII_BYTE_HISTOGRAM_MAX 128

// NOTE(chowie): Byte Diff
internal u32
FuzzyMatch(u32 *HistogramA, u32 *HistogramB)
{
    u32 Result = 0;
    foreachN(u32, i, ASCII_BYTE_HISTOGRAM_MAX)
    {
        Result += AbsDifferenceClampAboveZero(HistogramA[i], HistogramB[i]);
    }

    return(Result);
}

//
//
//

// RESOURCE(): http://0x80.pl/notesen/2016-11-28-simd-strfind.html#generic-sse-avx2
internal umm
Substring(char *Source, umm n, char *Needle, umm k)
{
    Assert((k > 0) && (n > 0));

    __m128i First = _mm_set1_epi8(Needle[0]);
    __m128i Last  = _mm_set1_epi8(Needle[k - 1]);

    for(umm i = 0;
        i < n;
        i += 16)
    {
        __m128i FirstBlock = _mm_loadu_si128((__m128i *)(Source + i));
        __m128i LastBlock  = _mm_loadu_si128((__m128i *)(Source + i + k - 1));

        __m128i EqFirst = _mm_cmpeq_epi8(First, FirstBlock);
        __m128i EqLast  = _mm_cmpeq_epi8(Last, LastBlock);

        u16 Mask = (u16)_mm_movemask_epi8(_mm_and_si128(EqFirst, EqLast));

        while(Mask != 0)
        {
            bit_scan_result BitPos = FindLeastSignificantBit((u32)Mask);

            if(memcmp(Source + i + (umm)BitPos.Index + 1, Needle + 1, k - 2) == 0)
            {
                return(i + (umm)BitPos.Index);
            }

            Mask = (u16)ClearLeftMostSet((u32)Mask);
        }
    }

    // TODO(chowie): Error log
    return(999999);
}

int
main()
{
    gap_buffer *Gap = InitGapFromString("Hello world");
    GapBufferDump(Gap);

    InsertString(Gap, 5, " bb");
    GapBufferDump(Gap);

    GapDelete(Gap, GapLen(Gap), 5, true);

    char *s = GapBufferToString(Gap);
    char TextBufferHeader[64];
    _snprintf_s(TextBufferHeader, sizeof(TextBufferHeader),
                "Final: \"%s\"\n", s);
    OutputDebugStringA(TextBufferHeader);

    free(Gap->Buffer.Data);
    free(Gap);

    //
    //
    //

    ToLower(s, StringLen(s));
    char LowerTextBuffer[64];
    _snprintf_s(LowerTextBuffer, sizeof(LowerTextBuffer),
                "To Lower Test: \"%s\"\n", s);
    OutputDebugStringA(LowerTextBuffer);

    //
    //
    //

    char *Word = "rebase";
    char *Misspelled = "revase";

    u32 h1[ASCII_BYTE_HISTOGRAM_MAX] = {};
    ByteHistogram(h1, Word);

    u32 h2[ASCII_BYTE_HISTOGRAM_MAX] = {};
    ByteHistogram(h2, Misspelled);

    u32 Diff = FuzzyMatch(h1, h2);

    char HistogramTextBuffer[32];
    _snprintf_s(HistogramTextBuffer, sizeof(HistogramTextBuffer),
                "Histogram Test: %u\n", Diff);
    OutputDebugStringA(HistogramTextBuffer);
    
    //
    //
    //

    char *Needle = "as";
    umm Found = Substring(Word, StringLen(Word), Needle, StringLen(Needle));

    char SubstringTextBuffer[128];
    _snprintf_s(SubstringTextBuffer, sizeof(SubstringTextBuffer),
                "Substring Test, found at CursorP: %d\n", Found);
    OutputDebugStringA(SubstringTextBuffer);

    //
    //
    //

    // NOTE(chowie): For long strings only like file paths or asset
    // naming? Otherwise, just use radix string (below)
    u32 h0Test = Hash( "str" );
    u32 h1Test = Hash( h0Test, "ing" );
    u32 h2Test = Hash( "string" );

    Assert(h1Test == h2Test);
    CheckHash(h1Test, "string");

    char HashTextBuffer[128];
    _snprintf_s(HashTextBuffer, sizeof(HashTextBuffer),
                "Hash Test, h1: %d and h2: %d\n", h1Test, h2Test);
    OutputDebugStringA(HashTextBuffer);

    switch(h1Test)
    {
        case 0x1C93AFFC: // NOTE(chowie): Example test must convert u32 to hex!
        {
            char SwitchTextBuffer[128];
            _snprintf_s(SwitchTextBuffer, sizeof(SwitchTextBuffer),
                        "Switch Test Success\n");
            OutputDebugStringA(SwitchTextBuffer);
        } break;

        InvalidDefaultCase;
    }

    //
    //
    //

    // RESOURCE(): https://web.archive.org/web/20160207132838/https://github.com/foonathan/string_id
    // You could always do (using a pre-processor):
    // 1) hashed strings (can't retrieve original string value)
    // 2) string interning (only accessible at runtime, not compile time e.g. for switch statements)

    string RadixStringTest = WrapZ("03chrisgreen");
    u64 nValue = ToRadix40(RadixStringTest);

    char FormatTextBuffer[15];
    string FromRadixStringTest = FromRadix40(nValue, BundleString(FormatTextBuffer, sizeof(FormatTextBuffer)));

    _snprintf_s(FormatTextBuffer, sizeof(FormatTextBuffer),
                "%s\n", FromRadixStringTest.Data);
    OutputDebugStringA(FormatTextBuffer);

    return(0);
}

/*
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
    umm Result = Len(GapBuffer) - (GapBuffer->End - GapBuffer->Start); // NOTE(chowie): Does not include gap
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
    u32 Delete = Abs(Count);
    umm NewCursor = Cursor;
    if(Count < 0)
    {
        NewCursor = Max(0, NewCursor - Delete);
    }
    ShiftGapTo(GapBuffer, NewCursor);
    GapBuffer->End = Min(GapBuffer->End + Delete, GapBuffer->Length); // NOTE(chowie): Protect from runoff!
    //--------|-----------------------------------
    //[123456]              [789abc]

    char RemTextLengthBuffer[256];
    _snprintf_s(RemTextLengthBuffer, sizeof(RemTextLengthBuffer),
                "\nInside Gap Buffer Cursor: %d, Contents: %s, Len: %d, Start: %d, End: %d",
                Cursor, GapBuffer->Buffer, GapBuffer->Length, GapBuffer->Start, GapBuffer->End);
    OutputDebugStringA(RemTextLengthBuffer);
}

internal void
InsertChar(gap_buffer *GapBuffer, umm Cursor, u8 Char)
{
    CheckGapSize(GapBuffer, 1);
    ShiftGapTo(GapBuffer, Cursor);
    GapBuffer->Buffer[GapBuffer->Start++] = Char;
    GapBuffer->Length++;

    char AddTextLengthBuffer[256];
    _snprintf_s(AddTextLengthBuffer, sizeof(AddTextLengthBuffer),
                "\nInside Gap Buffer Cursor: %d, Contents: %s, Len: %d, Start: %d, End: %d",
                Cursor, GapBuffer->Buffer, GapBuffer->Length, GapBuffer->Start, GapBuffer->End);
    OutputDebugStringA(AddTextLengthBuffer);
}


// internal void
// InsertString(gap_buffer *GapBuffer, umm Cursor, char *String)
// {
//     CheckGapSize(GapBuffer, StringLen(String));
//     ShiftGapTo(GapBuffer, Cursor);
//     strcpy((char *)GapBuffer->Buffer, String); // TODO(chowie): This is totally not correct!
//     GapBuffer->Start += StringLen(String);
// }
// TODO(chowie): utf-8 support? How does one encode a rune in C? TIMESTAMP: 17:00-18:00
// internal void
// InsertRune(gap_buffer *GapBuffer, umm Cursor, u8 Char)
// {
//     CheckGapSize(GapBuffer, 1);
//     ShiftGapTo(GapBuffer, Cursor);
//     GapBuffer->Buffer[GapBuffer->Start] = Char;
//     GapBuffer->Start += 1;
// }
// 
// // TODO(chowie): How does one insert a string in C? TIMESTAMP: 17:00-18:00
// internal void
// InsertString(gap_buffer *GapBuffer, umm Cursor, char *String)
// {
//     CheckGapSize(GapBuffer, StringLength(String));
//     ShiftGapTo(GapBuffer, Cursor);
//     strcpy(GapBuffer->Buffer, *String); // TODO(chowie): This is totally not correct!
//     GapBuffer->Start += StringLength(String);
// }

int
main(void)
{
    gap_buffer GapBuffer = {};

    GapBufferInit(&GapBuffer, 32);
    InsertChar(&GapBuffer, 0, '0');
    InsertChar(&GapBuffer, 1, '1');
    InsertChar(&GapBuffer, 2, '2');
    InsertChar(&GapBuffer, 3, '3');
    InsertChar(&GapBuffer, 4, '4');
    InsertChar(&GapBuffer, 2, 'A');
    InsertChar(&GapBuffer, 5, '5');

    Remove(&GapBuffer, 2, 2);
    // TODO(chowie): Insertion/Removal of the character is wrong LOL

    //InsertString(&GapBuffer, 4, "Hello");

    return(0);
}
*/

/*
// NOTE(chowie): For scaling with other data types with a union (avoids casting).
typedef struct node_data
{
    s32 Info;
} node_data;

typedef struct node
{
    node_data Data;
    struct node *Next;
} node;

internal node *
Push(node *Node, node_data Data)
{
    node *Result = (node *)malloc(sizeof(node));
    Assert(Result);
    Result->Data = Data;
    Result->Next = Node;
    return(Result);
}

internal node *
Pop(node *Node, node_data *Data)
{
    node *Result = Node;
    if(Result)
    {
        *Data = Node->Data;
        Node = Node->Next;
        free(Result);
    }
    else
    {
        // TODO(chowie): Logging!
    }
    return(Node);
}

internal node_data *
StackTop(node *Node)
{
    return(&Node->Data);
}

internal b32x
StackIsEmpty(node* Node)
{
    return((Node == NULL) ? 1 : 0);
}

int
main(void)
{
    node *Top = {};
    node_data Element;

    Element.Info = 11;
    Top = Push(Top, Element);

    char TestBuffer[256];
    _snprintf_s(TestBuffer, sizeof(TestBuffer),
                "Stack Top: %d\n", StackTop(Top)->Info);
    OutputDebugStringA(TestBuffer);

    Element.Info = 33;
    Top = Push(Top, Element);

    char TestBuffer2[256];
    _snprintf_s(TestBuffer2, sizeof(TestBuffer2),
                "Stack Top2: %d\n", StackTop(Top)->Info);
    OutputDebugStringA(TestBuffer2);

    while(!StackIsEmpty(Top))
    {
        Top = Pop(Top, &Element);

        char PopBuffer[256];
        _snprintf_s(PopBuffer, sizeof(PopBuffer),
                    "Popping: %d\n", Element.Info);
        OutputDebugStringA(PopBuffer);
    }

    return(0);
}
*/

