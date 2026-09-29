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

typedef float f32;
typedef double f64;

#define enum8(type)  u8
#define enum16(type) u16
#define enum32(type) u32
#define enum64(type) u64

union v2s
{
    struct
    {
        s32 x, y;
    };
    struct
    {
        s32 Min, Max;
    };
    struct
    {
        s32 Width, Height;
    };
    struct
    {
        u32 a, b; // NOTE(chowie): For making equations
    };
    struct
    {
        u32 Start, End; // NOTE(chowie): For animation
    };
    s32 E[2];
};

union rect2i
{
    struct
    {
        v2s Min;
        v2s Max;
    };
    struct
    {
        v2s Min;
        v2s MaxN; // NOTE(chowie): N is negative
    };
    s32 E[4];
};

inline v2s
operator+(v2s A, v2s B)
{
    v2s Result = {A.x + B.x, A.y + B.y};
    return(Result);
}

inline v2s
operator*(s32 A, v2s B)
{
    v2s Result = {A*B.x, A*B.y};
    return(Result);
}

inline v2s
operator-(v2s A, v2s B)
{
    v2s Result = {A.x - B.x, A.y - B.y};
    return(Result);
}

inline v2s &
operator+=(v2s &A, v2s B)
{
    A = A + B;
    return(A);
}

inline v2s &
operator-=(v2s &A, v2s B)
{
    A = A - B;
    return(A);
}

inline v2s
GetDim(rect2i Rect)
{
    v2s Result = Rect.Max - Rect.Min;
    return(Result);
}

inline s32
GetArea(rect2i A)
{
    v2s Dim = GetDim(A);
    s32 Result = Dim.x*Dim.y;
    return(Result);
}

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

// RESOURCE: https://hero.handmade.network/forums/code-discussion/t/3190-field_array_implementation_union_of_fields_and_array
// NOTE(from Blake "rationalcoder" Martin): Syntactic sugar for vector
// likes (e.g. v2, v3). The downside is bad introspection; usually you
// wouldn't care for this kind of struct. No need for a
// terminator/fake field check!
// IMPORTANT(chowie): Use when you want to say MemoryArena[3] or
// Buttons[12] (to process them all together), and to reference
// individually e.g. PermArena, TempArena, AudioArena
#define Glue_(A, B) A##B
#define Glue(A, B) Glue_(A, B)

#define FIELD_ARRAY_(type, struct_definition, counter)                  \
typedef struct struct_definition Glue(_anon_array, counter);            \
union                                                                   \
{                                                                       \
    struct struct_definition;                                           \
    type E[sizeof(Glue(_anon_array, counter)) / sizeof(type)];          \
};                                                                      \
static_assert(sizeof(Glue(_anon_array, counter)) % sizeof(type) == 0,   \
              "Field Array of type '" #type "' must be a multiple of sizeof(" #type ")")\

#define FIELD_ARRAY(type, struct_definition)       \
FIELD_ARRAY_(type, struct_definition, __COUNTER__)

#define foreachN(type, Value, array) for(type Value = 0; Value < array; ++Value)
#define foreach(type, Value, array) for(type Value = 0; Value < Len(array); ++Value)

#define Pow2N(Value) (1 << (Value))
#define BitSet(Bit) Pow2N(Bit)

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
CatStrings(char *SourceA, umm SourceACount,
           char *SourceB, umm SourceBCount,
           char *Dest, umm DestCount)
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

inline u32
RoundF32ToU32(f32 F32)
{
    u32 Result = (u32)_mm_cvtss_si32(_mm_set_ss(F32));
    return(Result);
}

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

#define TEMP_BUFFER_SIZE 512
#define Base10 10
global f32 Bases[] = { 1, 10, 100, 1000, 10000, 100000, 1000000 };
// RESOURCE: https://gist.github.com/d7samurai/1d778693ba33bbd2b9d709b209cc0aba
// TODO(chowie): Convert to using arenas!
// TODO(chowie): This hideous functions is really convenient! Probably only use this for debugging only!
struct d7sam_concat
{
    d7sam_concat(char* Source) { operator()(Source); }
    d7sam_concat(char  Source) { operator()(Source); }
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

    d7sam_concat &
    operator()(char Source)
    {
        TempBuffer[CharCount++] = Source;
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

// COULDDO(chowie): Is prefix or infix function possible?

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

#define CONST_STRING(String) {sizeof(String) - 1, (u8 *)(String)}

internal b32x
StringsAreEqual(string A, string B)
{
    b32x Result = true;
    if(A.Size != B.Size)
    {
        Result = false;
    }

    foreachN(u32, Index, A.Size)
    {
        if(A.Data[Index] != B.Data[Index])
        {
            Result = false;
        }
    }

    return(Result);
}

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

internal string
BundleString(u8 *String, umm Size)
{
    string Result = {};
    Result.Size = Size;
    Result.Data = String;

    return(Result);
}

internal b32x
IsSlash(u8 Char)
{
    b32x Result = (Char == '/' || Char == '\\');
    return(Result);
}

internal b32x
IsDot(u8 Char)
{
    b32x Result = (Char == '.');
    return(Result);
}

internal b32x
IsRadix40SymIndicator(u8 Char)
{
    b32x Result = (Char == '$');
    return(Result);
}

internal string
ChopLastSlash(string String)
{
    string Result = String;
    if(String.Size > 0)
    {
        // NOTE(chowie): Position is one past last slash
        umm Position = String.Size;
        for(s64 PositionIndex = String.Size - 1;
            PositionIndex >= 0;
            --PositionIndex)
        {
            if(IsSlash(String.Data[PositionIndex]))
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

internal string
ChopLastDot(string String)
{
    string Result = String;
    if(String.Size > 0)
    {
        // NOTE(chowie): Position is one past last slash
        umm Position = String.Size;
        for(umm PositionIndex = String.Size - 1;
            PositionIndex >= 0;
            --PositionIndex)
        {
            if(IsDot(String.Data[PositionIndex]))
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

// NOTE(chowie): SkipLastSlash should mirror ChopLastSlash
internal string
SkipLastSlash(string String)
{
    string Result = String;
    if(String.Size > 0)
    {
        // NOTE(chowie): Position is one past last slash
        umm PositionIndex = String.Size - 1;
        for(;
            PositionIndex >= 0;
            --PositionIndex)
        {
            if(IsSlash(String.Data[PositionIndex]))
            {
                break;
            }
        }

        Result.Data = String.Data + PositionIndex;
        Result.Size = String.Size - PositionIndex;
    }

    return(Result);
}

internal string
SkipLastDot(string String)
{
    string Result = String;
    if(String.Size > 0)
    {
        // NOTE(chowie): Position is one past last slash
        umm PositionIndex = String.Size - 1;
        for(;
            PositionIndex >= 0;
            --PositionIndex)
        {
            if(IsDot(String.Data[PositionIndex]))
            {
                break;
            }
        }

        Result.Data = String.Data + PositionIndex;
        Result.Size = String.Size - PositionIndex;
    }

    return(Result);
}

// NOTE(chowie): Radix40 '$'
internal string
SkipLastRadix40SymIndicator(string String)
{
    string Result = String;
    if(String.Size > 0)
    {
        // NOTE(chowie): Position is one past last slash
        umm PositionIndex = 0;
        for(;
            PositionIndex < String.Size;
            PositionIndex++)
        {
            if(IsRadix40SymIndicator(String.Data[PositionIndex]))
            {
                break;
            }
        }

        String.Data = String.Data + PositionIndex;
    }

    return(Result);
}

// TODO(chowie): This is wrong, I think SizeRemaining needs to go
internal string
Chop(string String, umm Amount)
{
    umm ClampedAmount = Min(Amount, String.Size);
    umm SizeRemaining = String.Size - ClampedAmount;

    string Result = {};
    Result.Size = SizeRemaining;
    Result.Data = String.Data;

    return(Result);
}

internal string
Skip(string String, umm Amount)
{
    umm ClampedAmount = Min(Amount, String.Size);
    umm SizeRemaining = String.Size - ClampedAmount;

    string Result = {};
    Result.Size = SizeRemaining;
    Result.Data = String.Data + ClampedAmount;

    return(Result);
}

// STUDY(chowie): Opl = one past last
internal string
SubstrOpl(string String, umm First, umm Opl)
{
    umm ClampedOpl = Min(Opl, String.Size);
    umm ClampedFirst = Min(First, ClampedOpl);

    string Result = {};
    Result.Size = ClampedOpl - ClampedFirst;
    Result.Data = String.Data + ClampedFirst;

    return(Result);
}

internal string
SubstrSize(string String, umm First, umm Size)
{
    string Result = SubstrOpl(String, First, First + Size);

    return(Result);
}

internal string
Range(u8 *First, u8 *Opl)
{
    string Result = {};

    Result.Size = (umm)(Opl - First);
    Result.Data = First;

    return(Result);
}

internal string
Prefix(string String, umm Size)
{
    string Result = {};
    Result.Size = Min(Size, String.Size);
    Result.Data = String.Data;

    return(Result);
}

internal string
Postfix(string String, umm Size)
{
    string Result = {};

    umm ClampedSize = Min(Size, String.Size);
    umm SkipTo = String.Size - ClampedSize;

    Result.Size = ClampedSize;
    Result.Data = String.Data + SkipTo;

    return(Result);
}

internal string
StripSymIndicatorRadix40IfPossible(string Source)
{
    if(IsRadix40SymIndicator(Source.Data[0]))
    {
        Source = Skip(Source, 1);
    }

    return(Source);
}


//
//
//

struct memory_arena
{
    umm Size;
    u8 *Base;
    umm Used;
};

internal void
InitArena(memory_arena *Arena, umm Size, u8 *Base)
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

internal string
PushString(memory_arena *Arena, string String)
{
    string Result = {0};
    Result.Data = PushArray(Arena, String.Size + 1, u8);
    Result.Size = String.Size;

    memcpy(Result.Data, String.Data, String.Size);
    Result.Data[Result.Size] = 0;

    return(Result);
}

internal string *
StringCreateFromStringZ(memory_arena *Arena, u8 *StringZ, umm Count)
{
    string *String = PushStruct(Arena, string);
    
    String->Data = PushArray(Arena, Count, u8);
    String->Size = Count;

    memcpy(String->Data, StringZ, (umm)Count);

    return(String);
}

// inline string
// PushString(memory_arena *Arena, string Source)
// {
//     u8 *Dest = (u8 *)PushSize_(Arena, Source.Size);
//     foreachN(u32, CharIndex, Source.Size)
//     {
//         Dest[CharIndex] = Source.Data[CharIndex];
//     }
//     Dest[Source.Size] = 0;
// 
//     string Result = {Source.Size, Dest};
//     return(Result);
// }

inline void
ClearArena(memory_arena *Arena)
{
    InitArena(Arena, Arena->Size, Arena->Base);
}

//
//
//

inline b32x
IsUpper(u8 c)
{
    b32x Result = (c >= 'A') && (c <= 'Z');
    return(Result);
}

internal void
ToLower(string String)
{
    foreachN(umm, j, String.Size)
    {
        if(IsUpper(String.Data[j]))
        {
            String.Data[j] ^= (1 << 5);
        }
    }
}

//
//
//

// TODO(chowie): History chain word list + random fair shuffle?
//    = RESOURCE(): Shuffling a POSET - https://www.youtube.com/watch?v=dr-jUCelobk
//      > Topological sort in two passes odd and even in pairs
//      > Useful for randomising a list fairly while still mailing certain ordered rules.
//      > Might be useful for language learning e.g. transitive verbs are tested before intransitive!
//      > Even/Odd int http://marc-b-reynolds.github.io/math/2022/12/16/ParityWalks.html
//      > https://marc-b-reynolds.github.io/math/2022/01/28/RNGParity.html
//        ~"u  |= 1; return u ^ (u >> 1);" = "return u ^ rotl(u,1)" // 3u
//        ~"u <<= 1; return u ^ (u >> 1);" = "return u ^ rotl(u,1) ^ 1" // 3u + 1

// COULDDO(chowie): Convert birdfont .ttf to default lowercase letters?

// TODO(chowie): Add "@"?
// IMPORTANT(chowie): API works under assumption of Radix40ToAscii[],
// convert between custom formats e.g. ConlangRadix40ToAscii[]
global string Radix40ToAscii        = CONST_STRING("?0123456789_.abcdefghijklmnopqrstuvwxyz"); // 39/40
global string ConlangRadix40ToAscii = CONST_STRING("?(![{,%}]?)_.abcdefghijklmnopqrstuvwxyz"); // 39/40
static_assert(sizeof(Radix40ToAscii) == sizeof(ConlangRadix40ToAscii));

// NOTE(chowie): Case-insenstive hash by default
internal s32
ToRadix40(u8 c)
{
    // NOTE(chowie): Normal order, 0-9, Special char, a-z.
    // Below is optimised by most frequent/typeable on a keyboard.

    s32 Result = 0;
    if((c >= 'a') && (c <= 'z'))
    {
        Result = 1 + 11 + 1 + (c - 'a');
    }
    else if((c >= 'A') && (c <= 'Z')) // NOTE(chowie): Poor man's ToLower, implicit
    {
        Result = 1 + 11 + 1 + (c - 'A');
    }
    else if(( c >= '0' ) && ( c <= '9' ))
    {
        Result = 1 + c - '0';
    }
    else
    {
        // NOTE(chowie): Mirrors position of numbers in array, offsets 1 + 11.
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
            case ' ':
            {
                Result = 1 + 10;
            } break;

            case '.':
            {
                Result = 1 + 11;
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

// COULDDO(chowie): Is prefix or infix function possible?
inline u64
ToRadix40(u64 Prefix, string Suffix)
{
    u64 Result = Prefix;
    for(u8 *Scan = Suffix.Data;
        *Scan;
        Scan++)
    {
        Result = 40*Result + ToRadix40(*Scan);
    }

    return(Result);
}

internal u64
ToRadix40Range(string String, umm Start = 0, umm End = 0)
{
    Assert(End <= 12);
    Assert(AbsDifferenceClampAboveZero((u32)Start, (u32)End) <= 12);

    u64 Result = 0;

    umm Count = 0 + Start;
    for(u8 *Scan = String.Data + Start;
        *Scan && (Count < End);
        Scan++, Count++)
    {
        Result = 40*Result + ToRadix40(*Scan);
    }

    return(Result);
}

internal u64
ToRadix40Chop(string String, umm End)
{
    Assert(End <= 12);

    u64 Result = 0;

    umm Count = 0;
    for(u8 *Scan = String.Data;
        *Scan && (Count < End);
        Scan++, Count++)
    {
        Result = 40*Result + ToRadix40(*Scan);
    }

    return(Result);
}

internal u64
ToRadix40Skip(string String, umm Start)
{
    Assert(String.Size <= 12);
    Assert(Start <= String.Size);

    u64 Result = 0;
    for(u8 *Scan = String.Data + Start;
        *Scan;
        Scan++)
    {
        Result = 40*Result + ToRadix40(*Scan);
    }

    return(Result);
}

internal string
FromRadix40(memory_arena *Arena, u64 Radix, b32x SymIndicator = false, string Ref = Radix40ToAscii)
{
    string Result = {};
    Result.Data = PushArray(Arena, 12, u8);

    *(--Result.Data) = '\0'; // TODO(chowie): Unnecessary? To remove because of pascal-style strings
    while(Radix)
    {
        *(--Result.Data) = Ref.Data[Radix % 40];
        Radix /= 40;
        Result.Size++;
    }

    if(SymIndicator && (Ref.Data == Radix40ToAscii.Data))
    {
        *(--Result.Data) = '$'; // COULDDO(chowie): Optional symbol indicator, "sym_"?
        Result.Size++;
    }

    return(Result);
}

// NOTE(chowie): Appends chars at end of string like djb2 hash()
// COULDDO(chowie): Improve adding a single character without fromradix40()?
inline u64
ToRadix40Sing(u64 Prefix, u64 Suffix)
{
    u64 Result = Prefix;
    Result = 40*Result + Suffix;

    return(Result);
}
// TODO(chowie): Slow-path, converts with from. Check if this is wrong!
inline u64
ToRadix40Mult(memory_arena *Arena, u64 Prefix, u64 Suffix)
{
    string FromRadixStringTest = FromRadix40(Arena, Suffix, false);
    u64 nValue = ToRadix40(Prefix, FromRadixStringTest);

    return(nValue);
}

// TODO(chowie): Autoconvert to bitset with preprocessor?

// IMPORTANT(chowie): Allows mix and match words of different conlang
// families together
enum confamily_mode : u8
{
    ConFamily_InDev,

    ConFamily_Euro,
    ConFamily_Asia,
    ConFamily_Calc,
    ConFamily_Base10,

    ConFamilyCount,
};

// NOTE(chowie): POS = Parts of Speech
// IMPORTANT(chowie): Only __one__ primary tag can be active on a
// word's definition. Although, words can have multiple primary tags
// only one of its definition can be active (in text).
enum conpos_tag : u8
{
    ConPOS_Invalid, // NOTE(chowie): Word not found/incomplete (still accepts string)

    ConPOS_Nn, // Noun
    ConPOS_Vb, // Verb
    ConPOS_Qn, // Question
    ConPOS_Cl, // Classifier
    ConPOS_Pt, // Particle
    ConPOS_Cj, // Conjunction
    ConPOS_Pc, // Place
    ConPOS_Pp, // Preposition
    ConPOS_Dt, // Time
    ConPOS_Em, // Emotion
    ConPOS_On, // Onomatpoeia
    ConPOS_Nm, // Numeral

    ConPOS_Count,
};

// TODO(chowie): Not sure if I'd rather this to keep the enum and type
// last two characters in? Constexpr?
#define Stringify(x) #x
#define LAST_TWO_CHARS(x) (&Stringify(x)[sizeof(Stringify(x)) - 3])

// STUDY(chowie): This is called compile-time (string) hash. Notably, this needs a loop!
// RESOURCE(): https://craigulmer.com/hobbies/index.cgi?id=160116_Verifying_C_CompileTime_Hashing
// RESOURCE(): https://chrisgreendevelopmentblog.wordpress.com/2024/10/03/approaches-for-efficient-unique-symbols-in-c/
#define $ConPos_Nn 0x42a // ToRadix40(CONST_STRING(LAST_TWO_CHARS(ConPos_Nn)))
#define $ConPos_Vb 0x55e // ToRadix40(CONST_STRING(LAST_TWO_CHARS(ConPos_Vb)))
#define $ConPos_Qn 0x4a2 // ToRadix40(CONST_STRING(LAST_TWO_CHARS(ConPos_Qn)))

// TODO(chowie): Preprocessed strings and a generate .h file!
// e.g. constexpr Symbol_t MA_STRENGTH = Symbol_t( 0x55aa348978 );  //Hash function calculated by preprocessor

// TODO(chowie): Why does u64 bitflag doesn't work?
// TODO(chowie): Try ryan fleury's tagging system
// TODO(chowie): Run this via the game's dialogue world-state belief system like Inkle?
// IMPORTANT(chowie): Conlangs can have as many flags/subtags, but
// aren't encoded in the word. Must search hash table! These tags aids
// game to evalute how to interpret a word's meaning/intent
enum conpos_flags
{
    ConPOSF_Cmd = BitSet(0), // Command
    ConPOSF_Pl = BitSet(1), // Plural
    ConPOSF_Neg = BitSet(2), // Negation
    ConPOSF_Name = BitSet(3), // Name

    ConPOSF_Fml = BitSet(4), // Formal
    ConPOSF_Col = BitSet(5), // Collective Noun
    ConPOSF_Deg = BitSet(6), // Numeric Degree
    ConPOSF_Perf = BitSet(7), // Performance (Encouragement)
    ConPOSF_Rep = BitSet(8), // Repetition
    ConPOSF_Fill = BitSet(9), // Filler words e.g. hm, um etc
    ConPOSF_Soc = BitSet(10), // Social words
    ConPOSF_Cau = BitSet(11), // Caution
    ConPOSF_Move = BitSet(12), // Motion
    ConPOSF_Rest = BitSet(13), // Rest
    ConPOSF_Aff = BitSet(14), // Affect
    ConPOSF_Met = BitSet(15), // Meterological
    ConPOSF_Bcpl = BitSet(16), // Body Corporeal
    ConPOSF_Give = BitSet(17), // Giving
    ConPOSF_Att = BitSet(18), // Attention
    ConPOSF_Spk = BitSet(19), // Speaking
    ConPOSF_Hmm = BitSet(20), // Thinking
    ConPOSF_Thx = BitSet(21), // Thanks
    ConPOSF_Lik = BitSet(22), // Like
    ConPOSF_Pref = BitSet(23), // Preference
    ConPOSF_Fun = BitSet(24), // Amusement
    ConPOSF_Comp = BitSet(25), // Competing
    ConPOSF_Phr = BitSet(26), // Phrase
    ConPOSF_Interj = BitSet(27), // Interjection

    ConPOSF_Hedge = BitSet(28), // Hedging/Guessing/Predicting
//    Conlang_POSF_Phatic = BitSet(30), //
//    Conlang_POSF_Interr = BitSet(31), //
//    Conlang_POSF_Fact = BitSet(32), //
//    Conlang_POSF_Impera = BitSet(33), //
//    Conlang_POSF_Hypo = BitSet(34), //
//    Conlang_POSF_ = BitSet(), //
};

struct conword_name
{
    enum8(confamily_mode) Family;
    string POS;
    string Word;
};

internal u64
ConwordNameToRadix40(confamily_mode Family, string POS, string Conword)
{
    u64 Result = 0;
    if(Conword.Size <= 10)
    {
        // NOTE(chowie): 'Family + 1' to support enum starting at 0
        conword_name Name = {Radix40ToAscii.Data[Family + 1], POS, Conword};
        // COULDDO(chowie): Can I simplify this step (concat to convert to string)?
        // Hopefully this should get covered by the gap buffer?
        // COULDDO(chowie): Unfortunate I have to convert to char
        // inorder to concat, then back to string. Simplify?
        string ReconstructString = WrapZ(d7sam_concat((char)Name.Family)((char *)Name.POS.Data)((char *)Name.Word.Data));
        Result = ToRadix40(ReconstructString);
    }
    else
    {
        // TODO(chowie): Log error
    }

    return(Result);
}

//
// File Formats
//

// IMPORTANT: TODO(chowie): Make sure the naming uses some software
// that gives you a template when you give the name. Or use software
// that allows a default naming convention?

//
// 3D ENVIRONMENT ART MODULAR KIT RADIX40 ENCODING:
//
// - 12 char max
//
// _ _|_ _ _ _|_|_|_|_|_ _
// 0 1 2 3 4 5 6 7 8 9 10 11
//
// - byte 0-1   = Basekit Name e.g. Ut (Utility = kits made of other kits, e.g. arch walls with pipes), Pi (Pipe), In (Industrial), De (Deco = MultiLevel Interior/Exterior), Bu (Building), Tu (Steam Tunnel = Vertical), Cav (Cave), Pr (Prop/Set Piece), Gl (Glue = Doorframes, blending between different kits/tiles), He (Hero = Inflexible custom set pieces)
// - byte 2-5   = Subkit Name e.g. In Pr Basekit: Mach (Machine), Foli (Foliage), Gree (Greebles/Decals), Hole (Hole e.g. Wall with punched holes)
// - byte 6     = Piece Type e.g. 1 (1 way corridor), 2 (hallway turn/corner), 3 (junction), 4 (junction)
// - byte 7     = Spatial Type e.g. I (Indoor), O (Outdoor), S (Semi-enclosed/Threshold spaces e.g. porches, verandahs)
// - byte 8     = Tiling Direction e.g. 0 (None), V (Vertically), H (Horizontally), N, S, E, W (Cardinal Directions)
// - byte 9     = Animation Repligram Trigger e.g. 0 (None, Static/Decal), P (on proximity of player/mob entity), N (autonomously = waits for nature/time e.g. tree shrivelling), F (Fire), R (Rain), W (Water), O (Oxygen), D (Damage), I (Illness)
// - byte 10-11 = Variant Code e.g. a1, b1, c1 etc (instead of 00, 01, 02 etc), nicer to view for artist
//

//
// 2D/3D ENVIRONMENT STATIC TEXTURE ART (+ DESIGN ICONS AND ILLUSTRATION) RADIX40 ENCODING:
//
// - 12 char max
//
// _ _|_ _ _ _ _|_|_|_|_ _
// 0 1 2 3 4 5 6 7 8 9 10 11
//
// - byte 0-1   = Texture Category/Purpose Name e.g. Li (Lighting/Sky/Astral/Weather), Ar (Architectural), La (Landscape/Flora/Aquatic/Liquids), Np (NPCs/Fauna), Pr (Props/Functional/Interactable/Misc), Dv (Technical/Dev e.g. Collision), or Meta Placement e.g. Sp (Splash), Ma (Main Menu), Lo (Logo), UI (UI Icons), HU (HUD), Wo (In-game/World), Bi (In-game/World with Billboards), Co (Concept Art)
// - byte 2-6   = Texture Name (flexible range = start byte 3 then chop 4 bytes off end!)
// - byte 7     = Texture Map e.g. D (Diffuse), N (Normal), S (Specular), A (AO = Ambient Occlusion)
// - byte 8     = Shape/Decorative Type e.g. L (Large 1m block), S (Small 0.5m Block), F (FX 1m Block), X (Fletched/3-Cross Deco), T (Triangle Deco), B (Box Deco), Z (Z Deco), A (3-Axis-Aligned Deco), P (Panel/Plane/Decal/Design Icon/Illustration)
// - byte 9     = Face Direction e.g. 0 (None), T (Tiling Texture/Seamless), V (Vertical), H (Horizontal), N, S, E, W, U, D (Cardinal Directions), B (Billboard)
// - byte 10-11 = Variant Code e.g. a1, b1, c1 etc (instead of 00, 01, 02 etc), nicer to view for artist
//
// NOTE(chowie): No animation trigger for textures
// RESOURCE(): Sub-category ideas - https://ayemteezy.github.io/minecraft-storage-organization-cheatsheet/
//

//
// 2D/3D CHARACTER ART MODULAR KIT (+ ANIMATION FRAME/VIDEO CLIPS) RADIX40 ENCODING:
//
// - 12 char max
//
// _ _|_ _ _ _ _|_|_ _ _ _
// 0 1 2 3 4 5 6 7 8 9 10 11
//
// - byte 0-1   = Character NPC/Character Class/Prop Name e.g. Bi (Birds), El (Elephant), In (Insects), Pr (Prop/Set Piece e.g. Umbrella, Cigarette) or Animation Category e.g. L (Locomotion), C (Combat), E (Interact), N (Narrative), A (NPC/AI), X (Cinematic)
// - byte 2-6   = Character Body Part Category Name
// - byte 7     = Animation Repligram Trigger e.g. 0 (None, Static/Decal), T (on proximity, entity/mob trigger), N (autonomously = waits for nature/time e.g. tree shrivelling), F (Fire), R (Rain), W (Water), O (Oxygen), D (Damage), I (Illness)
// - byte 9-11  = Variant Code e.g. a01, b01, c01 etc, nicer to view for artist
//

//
// CONWORD RADIX40 ENCODING:
//
// - 12 char max
//
// _|_ _|_ _ _ _ _ _ _ _ _
// 0 1 2 3 4 5 6 7 8 9 10 11
//
// - byte 0    = Conlang Type e.g. Base 10 Numbers, Eurolang, Asialang, Calclang etc
// - byte 1-2  = Parts of Speech (primary tag) e.g. verb, noun, place etc
// - byte 3-11 = Conlang Word
//

//
// FONT RADIX40 ENCODING:
//
// - 12 char max
//
// _ _|_ _ _ _ _|_|_|_|_ _
// 0 1 2 3 4 5 6 7 8 9 10 11
//
// - byte 0-1   = Font Category/Real-World Country Name e.g. cl (conlang), eu (Euro), as (Asia) etc.
// - byte 2-6   = Typeface Name (flexible range = start byte 3 then chop 4 bytes off end!)
// - byte 7     = Font Hierarchy e.g. H (Heading), B (Body), U (UI)
// - byte 8     = Font Flesh e.g. S (Serif), P (San-Serif/Plain), L (Slab Serif), H (Handwritten), M (Monospaced), C (Script)
// - byte 9     = Form Model e.g. D (Dynamic), R (Rational), G (Geometric)
// - byte 10-11 = Variant/Weight Code e.g. a1, b1, c1 etc (instead of 00, 01, 02 etc), nicer to view for artist
//
// NOTE(chowie): byte 8-9 is exists to know how to match the pairing
//

//
// SOUND BANK RADIX40 ENCODING:
//
// - 12 char max
//
// _ _|_ _ _ _ _|_|_|_|_ _
// 0 1 2 3 4 5 6 7 8 9 10 11
//
// - byte 0-1   = Sound Bank Category Name e.g. We (Weather), So (Soundscape), Mu (Music Score), Mo (Movement e.g. footsteps), Do (Doors), An (Animal), Am (Ambience), Sp (Speech), Sc (Script), UI (UI), Co (Collision), Ms (Misc.)
// - byte 2-6   = Sound Name (flexible range = start byte 3 then chop 4 bytes off end!)
// - byte 7     = Spatial Type e.g. 0 (None/Mono), I (Indoor), O (Outdoor), S (Semi-enclosed/Threshold spaces e.g. porches, verandahs)
// - byte 8     = Spatial Distance Type e.g. 0 (None/Mono), C (Close), R (Rear), X (Far), D (Distant), F (Front), B (Back)
// - byte 9     = Sound Trigger e.g. 0 (None, Immediate), T (on proximity, entity/mob trigger), N (animated autonomously = waits for nature/time e.g. tree shrivelling), F (Fire), R (Rain), W (Water), O (Oxygen), D (Damage), I (Illness)
// - byte 10-11 = Variant Code e.g. ll (ll for looped), otherwise default to a1, b1, c1 etc
//
// TODO(chowie): Can work with AABB and/or spherical collision, might
// need LOD if box is large?
// TODO(chowie): A* back to player to find if sounds should hit the
// player or occluded.
// NOTE(chowie): Typically you need a string bank
//

// RESOURCE(): 'Fallout 4's' Modular Level Design 2016 - https://gdcvault.com/play/1023202/-Fall
// IMPORTANT TODO(chowie): Make sure CTRL + Mouse Wheel works to swap
// variants/kits instead of a menu! CTRL + Right Click to pull up category?
// IMPORTANT TODO(chowie): Helper markers (in editor) on objects to
// help click them easier without needing to turn the camera!
// COULDDO(chowie): I can't imagine scenes would be so complex that
// we'll need to build layers?
// NOTE(chowie): Rendering kits generally might be a pain
// IMPORTANT(chowie): When making kits!
// 1) Basekit, don't make a greybox kit, blank walls doesn't help
// 2) COULDDO(chowie): Make in pieces, a corner walls instead of made
// from 1 piece (easy to render/optimise), it could be a corner made
// via a wall, corner, ceiling, floor for granular/variation combo.
// However, object count explodes! Would rather swap by texture?
//
//                                W______________T
//                                /             /|                 /|
//                               /             / |                / |
//                              /             /  |               /  |
//            open             /             /   |              /   |
//        ______________     H/_____________/Y   |             /    |
//       /             /      |             |    |             |    | 
//      /             /       |             |    |             |    |
//     /             /        |             |    |     unround |    | round
//    /             /         |             |    |             |    |
//   /_____________/          |   ~         |    |M            |    |
//        closed              |             |   /              |   /
//                            |             |  /               |  /
//                            |             | /                | /
//                           X|_____________|/V                |/
//                                 AIOU
//
//                                        back
//                             _____________
//                            |             |
//                            |             |
//                            |             |
//                            |             |
//                            |    front    |
//                            |             |
//                            |             |
//                            |             |
//                            |_____________|
//

// TODO(chowie): This should be automated by pre-proceessor at startup!
// STUDY(chowie): For many compilers e.g. gcc, clang, msvc, "$" isn't a
// reserved symbol!
#define $ttf   0xc8512 // ToRadix40(CONST_STRING(".ttf"))
#define $bmp   0xc1384 // ToRadix40(CONST_STRING(".bmp")) // NOTE(chowie): Test format
#define $png   0xc6b23 // ToRadix40(CONST_STRING(".png"))
#define $wav   ToRadix40(CONST_STRING(".wav"))
#define $obj   ToRadix40(CONST_STRING(".obj"))
#define $csv   ToRadix40(CONST_STRING(".csv")) // NOTE(chowie): Localization
#define $gltf  ToRadix40(CONST_STRING(".gltf"))
#define $glsl  ToRadix40(CONST_STRING(".glsl"))
#define $slang ToRadix40(CONST_STRING(".slang"))

// NOTE(chowie): Internal formats
#define $rui   ToRadix40(CONST_STRING(".rui")) // NOTE(chowie): Binary asset packer format
#define $rsg   ToRadix40(CONST_STRING(".rsg")) // NOTE(chowie): Saved Game
#define $rpl   ToRadix40(CONST_STRING(".rpl")) // NOTE(chowie): Animation (Repligram)
#define $kpl   ToRadix40(CONST_STRING(".klp")) // NOTE(chowie): Narrative

//
//
//

// RESOURCE: https://www.youtube.com/watch?v=agUiYkvkoVg
// https://0xkiire.com/text_editor_data_structures1/
// TODO(chowie): Implement ropes with unicode!

// RESOURCE(): https://web.archive.org/web/20200917053102/https://github.com/RandyGaul/cute_headers/blob/master/cute_utf.h
// Want to convert between utf-8 and utf-16!
// IMPORTANT(chowie): TODO(chowie): Above library is unicode for game localization specifically!

struct gap_buffer
{
    buffer Buffer;
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
InitGap(memory_arena *Arena)
{
    gap_buffer *Result = PushStruct(Arena, gap_buffer);
    Result->Buffer.Size = INITIAL_GAP;
    Result->Buffer.Data = PushArray(Arena, Result->Buffer.Size, u8);
    Result->Start = 0;
    Result->End = Result->Buffer.Size;

    return(Result);
}

internal gap_buffer *
InitGapWrapZ(memory_arena *Arena, string String)
{
    gap_buffer *Result = InitGap(Arena);
    foreachN(u32, i, String.Size)
    {
        Result->Buffer.Data[Result->Start++] = *String.Data++;
    }

    return(Result);
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

// COULDDO(chowie): Implement gap buffer replace string?
internal void
ReplaceChar(gap_buffer *GapBuffer, umm Cursor, char C)
{
    Assert(Cursor >= 0 && Cursor <= GapLen(GapBuffer));

    GapBuffer->Buffer.Data[Cursor] = C;
}

internal void
InsertString(gap_buffer *GapBuffer, umm Cursor, string String)
{
    ShiftGapTo(GapBuffer, Cursor);
    foreachN(u32, i, String.Size)
    {
        InsertCharInternal(GapBuffer, *String.Data++);
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
GapBufferToStringZ(memory_arena *Arena, gap_buffer *GapBuffer)
{
    umm Len = GapLen(GapBuffer);
    char *Result = PushArray(Arena, Len + 1, char);

    memcpy(Result, GapBuffer->Buffer.Data, GapBuffer->Start);
    umm Right = GapBuffer->Buffer.Size - GapBuffer->End;
    memcpy(Result + GapBuffer->Start, GapBuffer->Buffer.Data + GapBuffer->End, Right);
    Result[Len] = '\0';

    return(Result);
}

internal string
GapBufferToWrapZ(memory_arena *Arena, gap_buffer *GapBuffer)
{
    umm Len = GapLen(GapBuffer);
    string Result = BundleString(PushArray(Arena, Len + 1, u8), Len + 1);

    memcpy(Result.Data, GapBuffer->Buffer.Data, GapBuffer->Start);
    umm Right = GapBuffer->Buffer.Size - GapBuffer->End;
    memcpy(Result.Data + GapBuffer->Start, GapBuffer->Buffer.Data + GapBuffer->End, Right);
    Result.Data[Len] = '\0';

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

        if(i >= GapBuffer->Start && i < GapBuffer->End)
        {
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

// TODO(chowie): Include this into the asset packer as a separate
// pack file so that everything can be localised/converted to .csv!
enum symgenerator_radix40_type
{
    SymGeneratorRadix40Type_General,
    SymGeneratorRadix40Type_Conword,
    SymGeneratorRadix40Type_GapBuffer,
    SymGeneratorRadix40Type_Repligram,

    SymGeneratorRadix40Type_Count,
};

struct symgenerator_radix40_source_general
{
    string String;
};

struct symgenerator_radix40_source_conword
{
    u8 Family;
    string POS;
    string Word; // NOTE(chowie): Max 9 chars
};

struct symgenerator_radix40_source_repligram
{
    u64 RepligramRadix40;
    u64 ConwordShortDevMeaningRadix40;
};

struct symgenerator_radix40_source_gapbuffer
{
    gap_buffer *Gap;
};

struct symgenerator_radix40_source
{
    symgenerator_radix40_type Type;
    union
    {
        symgenerator_radix40_source_general General;
        symgenerator_radix40_source_conword Conword;
        symgenerator_radix40_source_repligram Repligram;
        symgenerator_radix40_source_gapbuffer GapBuffer; // NOTE(chowie): Multiline conword
    };
};

// TODO(chowie): Split between gamemode_world and editor. Notably,
// conword and gapbuffer will be used for gameplay lots. However, char
// and env art, font and sound will probably not need to be parsed!
// Just default to basic hashing/references.
struct added_radix40
{
    symgenerator_radix40_source Source;
};

//
//
//

// RESOURCE(): http://0x80.pl/notesen/2023-11-20-popcount-suggestions.html
// IMPORTANT(chowie): Alternative to Levenshtein Distance for fuzzy match!
internal void
ByteHistogram(u32 *Histogram, string String)
{
    foreachN(u32, i, String.Size)
    {
        u32 b = String.Data[i];
        Histogram[b]++;
    }
}

// NOTE(chowie): Byte Diff
#define ASCII_BYTE_HISTOGRAM_MAX 128
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
// COULDDO(chowie): Threshold? 0 = "full" match, 1 = partial match (but accept as "full")
internal umm
Substring(string Source, string Needle) // NOTE(chowie): Needle can be of any size
{
    Assert((Needle.Size > 0) && (Source.Size > 0));

    __m128i First = _mm_set1_epi8(Needle.Data[0]);
    __m128i Last  = _mm_set1_epi8(Needle.Data[Needle.Size - 1]);

    for(umm i = 0;
        i < Source.Size;
        i += 16)
    {
        __m128i FirstBlock = _mm_loadu_si128((__m128i *)(Source.Data + i));
        __m128i LastBlock  = _mm_loadu_si128((__m128i *)(Source.Data + i + Needle.Size - 1));

        __m128i EqFirst = _mm_cmpeq_epi8(First, FirstBlock);
        __m128i EqLast  = _mm_cmpeq_epi8(Last, LastBlock);

        u16 Mask = (u16)_mm_movemask_epi8(_mm_and_si128(EqFirst, EqLast));

        while(Mask != 0)
        {
            bit_scan_result BitPos = FindLeastSignificantBit((u32)Mask);

            if(memcmp(Source.Data + i + (umm)BitPos.Index + 1, Needle.Data + 1, Needle.Size - 2) == 0)
            {
                return(i + (umm)BitPos.Index);
            }

            Mask = (u16)ClearLeftMostSet((u32)Mask);
        }
    }

    // TODO(chowie): Error log
    return(999999);
}

enum con_writingdir
{
    ConWritingDir_Ltr,
    ConWritingDir_Rtl,
    ConWritingDir_Boustrophedon,
    ConWritingDir_Vertical,
};

enum con_writingsys
{
    ConWritingSys_Alphabet,
    ConWritingSys_AlphabeticSyllabary,
    ConWritingSys_Syllabary,
};

// NOTE(chowie): Card game notation
enum panelpuzzle_type
{
    PanelPuzzleType_Hand,
    PanelPuzzleType_Board,
    PanelPuzzleType_SideBoard,
    PanelPuzzleType_Gizmo, // NOTE(chowie): Interactables (not text)
};

// NOTE(chowie): E.g. Layout = (3),(3)
// Brackets = player-facing punctation
// Numbers = spaces + demi-spaces
// , = newline
struct panelpuzzle
{
    panelpuzzle_type Type;
    confamily_mode Mode;
    rect2i Dim;
    u64 LayoutRadix40; // NOTE(chowie): Layout for punctuation and characters
};

enum panel_mode
{
    PanelMode_World,
    PanelMode_UI,
    PanelMode_Conword, // NOTE(chowie): Some panels are attached to certain words like verbs
};

// TODO(chowie): Pack and unpack radix40 in FieldArray .E loop
struct panel
{
    panelpuzzle *Puzzle;
    panel_mode Mode;
    b32x IsPlayerEditable;

    u64 *ConwordsByLineRadix40;
    u32 ConwordsByLineRadix40Count;

    // TODO(chowie): Make this a UI lister-panel implementation with filters like the Witness game
    union
    {
        FIELD_ARRAY(u64,
        {
            u64 LongNameRadix40; // NOTE(chowie): PanelID (hashed)
            u64 ShortNameRadix40; // NOTE(chowie): PanelID (hashed)
        });
    };

    string Desc; // TODO(chowie): Pre-processed via DJB2Hash()
};

// TODO(chowie): Do I need this?
struct panel_radix40string
{
    FIELD_ARRAY(string,
    {
        string LongName;
        string ShortName;
    });
};

// TODO(chowie): Pack and unpack radix40 in FieldArray .E loop
// RESOURCE(): https://www.degruyterbrill.com/document/doi/10.1515/lingty-2019-0030/html
// RESOURCE(): https://www.reddit.com/r/conlangs/comments/15qabie/origins_of_root_words/
struct conword_slot
{
    FIELD_ARRAY(u64,
    {
        u64 NameRadix40; // NOTE(chowie): conpos_tag is baked into NameRadix40
        u64 ArchiacNameRadix40; // NOTE(chowie): Optional
        u64 ShortDevMeaningRadix40;
    });
    string LongMeaning; // TODO(chowie): Pre-processed via DJB2Hash()
    u64 Flags;
    u8 FairmathAssociation; // NOTE(chowie): 0 = -tive, 1 = +tive
    u8 FairmathRarity; // IMPORTANT(chowie): Rarity changes throughout game
    // NOTE(chowie): 0 = common words (e.g. I/Me, He, Go, Names, Places)
    //               1 = specific/archiac words (activities, objects, some Places)
};

struct conword
{
    FIELD_ARRAY(conword_slot,
    {
        conword_slot Primary;
        conword_slot Secondary;
    });

    // NOTE(chowie): Optional
    b32x HasPanel;
    u8 PanelSlotIndex; // NOTE(chowie): Either slot 0 or 1
    panel *Panel;
};

struct conword_origin
{
    // COULDDO(chowie): Branching origin instead of linear?
    FIELD_ARRAY(conword,
    {
        conword Euro;
        conword Asia;
        conword Calc;
    });
};

internal panelpuzzle
PanelPuzzleParams(panelpuzzle_type Type, confamily_mode Mode, rect2i Dim, u64 LayoutRadix40)
{
    panelpuzzle Result = {};
    Result.Type = Type;
    Result.Mode = Mode;
    Result.Dim = Dim;
    Result.LayoutRadix40 = LayoutRadix40;
    // TODO(chowie): Don't know how to write this assert yet with radix40
    // Assert(Result.Layout.Size <= GetArea(Result.Dim));
    // NOTE(chowie): Parity of a 3-size panel is ._._._.
    // Dots = punctuation
    // Underscores = character
    // Thus, there should be (number of punctuation + 1) than characters
    return(Result);
}

internal panel
InitPanel(panelpuzzle *Params, panel_mode Mode, b32x IsPlayerEditable = true)
{
    panel Result = {};
    Result.Puzzle = Params;
    Result.Mode = Mode;
    Result.IsPlayerEditable = IsPlayerEditable;
}

// NOTE(chowie): Compare definition is the same by hash of radix40
// to test if has same definition (by origin)
// COULDDO(chowie): Loop always runs once, skip? What about secondary?
//           foreach(u32, SlotIndex, Origin.E->E)
//           {
//               conword_slot *Slot = Origin.E->E + SlotIndex;
//           }
internal u32
LengthOfShareConwordMeaningByOrigin(conword_origin *Origin)
{
    u32 Result = 0;
    u64 Compare = Origin->Euro.Primary.ShortDevMeaningRadix40;
    if(Compare != 0)
    {
        foreach(u32, OriginIndex, Origin->E)
        {
            conword *Conword = Origin->E + OriginIndex;
            if((Compare == Conword->Primary.ShortDevMeaningRadix40) || (Compare == Conword->Secondary.ShortDevMeaningRadix40))
            {
                Result += 1;
            }
            else
            {
                break;
            }
        }
    }

    return(Result);
}

// COULDDO(chowie): Specific by tag e.g. verb switches to secondary slot meaning?
internal void
ShareConwordMeaningByOrigin(conword_origin *Origin, u64 MeaningRadix40)
{
    foreach(u32, OriginIndex, Origin->E)
    {
        conword *Conword = Origin->E + OriginIndex;
        Conword->Primary.ShortDevMeaningRadix40 = MeaningRadix40;
    }
}

// TODO(chowie): Parse/Walk backwards, then forwards! Just like FromRadix40!
enum token_radix40_type
{
    // Shared
    TokenRadix40_VariantCode,

    // 3D Environment Art
    TokenRadix40_Env_Basekit,
    // TokenRadix40_Env_Subkit,
    TokenRadix40_Env_Piece,
    TokenRadix40_Env_Spatial,
    TokenRadix40_Env_TilingDir,
    TokenRadix40_Env_AnimTrigger,

    // 2D Texture Art
    TokenRadix40_Tex_Category,
    // TokenRadix40_Tex_Name,
    TokenRadix40_Tex_Map,
    TokenRadix40_Tex_Shape,
    TokenRadix40_Tex_FaceDir,

    // 2D/3D Character Art
    TokenRadix40_Char_Category,
    // TokenRadix40_Char_Name,
    TokenRadix40_Char_AnimTrigger,

    // Conword
    TokenRadix40_Con_Family,
    TokenRadix40_Con_POS,

    // Font
    TokenRadix40_Font_Category,
    // TokenRadix40_Font_Name,
    TokenRadix40_Font_Hierarchy,
    TokenRadix40_Font_Flesh,
    TokenRadix40_Font_Model,

    // Sound
    TokenRadix40_Sound_Category,
    // TokenRadix40_Sound_Name,
    TokenRadix40_Sound_Spatial,
    TokenRadix40_Sound_Dist,
    TokenRadix40_Sound_AnimTrigger,
};
struct token_radix40
{
    token_radix40_type Token;
    string Text;
};

struct tokenizer_radix40
{
    symgenerator_radix40_source Source;
    string At;
};

// IMPORTANT(chowie): Assumes strings will be parsed in piecemeal, and
// not as a whole file that needs stripping whitespaces etc. Because
// radix40 is limited to only 12 chars max!
internal token_radix40
GetRadix40Token(tokenizer_radix40 *Tokenizer)
{
    token_radix40 Token = {};
    Token.Text.Size = 1;
    Token.Text.Data = Tokenizer->At.Data;

    u8 C = Tokenizer->At.Data[0];
    ++Tokenizer->At.Data;
    switch(C)
    {
        // TODO(chowie): Not implemented!
        case 'C':
        {
        } break;

        InvalidDefaultCase;
    }

    return(Token);
}

internal void
ParseConword(tokenizer_radix40 *Tokenizer)
{
}

internal void
FromRadix40Bundle(memory_arena *Arena, string *Bundle, conword_slot *Slot)
{
    foreach(u32, SlotIndex, Slot->E)
    {
        u64 *Radix = Slot->E + SlotIndex;
        string Unpack = FromRadix40(Arena, *Radix);

        string *Stock = Bundle + SlotIndex;
        *Stock = PushString(Arena, Unpack);
    }
}

// TODO(chowie): Fix this because I think there's pointer-aliasing going on!
internal void
FromRadix40Bundle(memory_arena *Arena, string *Bundle, conword Conword)
{
    u32 Count = 0;
    foreach(u32, DefinitionIndex, Conword.E)
    {
        for(u32 SlotIndex = 0;
            SlotIndex < Len(Conword.Primary.E);
            SlotIndex++, Count++)
        {
            u64 *Radix = &Conword.E[DefinitionIndex].E[SlotIndex];
            string Unpack = FromRadix40(Arena, *Radix);

            Bundle[Count] = PushString(Arena, Unpack);
        }
    }
}

//
//
//

int CALLBACK
WinMain(HINSTANCE Instance,
        HINSTANCE PrevInstance,
        LPSTR     CommandLine,
        int       ShowCode)
{
    LPVOID BaseAddress = 0;

    memory_arena TextArena;
    InitArena(&TextArena, Megabytes(64), (u8 *)VirtualAlloc(BaseAddress, Megabytes(64),
                                                            MEM_RESERVE|MEM_COMMIT, PAGE_READWRITE));

    gap_buffer *Gap = InitGapWrapZ(&TextArena, CONST_STRING("Hello world"));
    GapBufferDump(Gap);

    InsertString(Gap, 5, CONST_STRING(" bb"));
    GapBufferDump(Gap);

    umm GapCursor = GapLen(Gap);
    GapDelete(Gap, GapCursor, 5, true);
    ReplaceChar(Gap, 1, 'a');

    string GapString = GapBufferToWrapZ(&TextArena, Gap);

    char TextBufferHeader[64];
    _snprintf_s(TextBufferHeader, sizeof(TextBufferHeader),
                "Final~ \"%s\"\n", GapString.Data);
    OutputDebugStringA(TextBufferHeader);

    // TODO(chowie): Way to save out gap to be reused later?

    //
    //
    //

    ToLower(GapString);

    char LowerTextBuffer[64];
    _snprintf_s(LowerTextBuffer, sizeof(LowerTextBuffer),
                "To Lower Test~ \"%s\"\n", GapString.Data);
    OutputDebugStringA(LowerTextBuffer);

    //
    //
    //

    // NOTE(chowie): For long strings only like file paths or asset
    // naming? Otherwise, just use radix string (below)
    u32 h0Test = Hash("str");
    u32 h1Test = Hash(h0Test, "ing");
    u32 h2Test = Hash("string");

    Assert(h1Test == h2Test);
    CheckHash(h1Test, "string");

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

    // TODO(chowie): Enforce enum Dictionary Editor vs gapbuffer In-game

    // TODO(chowie):
    // - Read in each gapbuff string/line
    // - Convert each to u64 radix
    // - De/Compress array of u64

    u64 nValue = 0;

    symgenerator_radix40_source Source = {};
    Source.Type = SymGeneratorRadix40Type_Conword;
    switch(Source.Type)
    {
        //
        // IMPORTANT(chowie): Editor
        //

        case SymGeneratorRadix40Type_General:
        {
            // TODO(chowie): Use this for code to shaders/material pipeline

            // NOTE(chowie): Below is an example for gameplay code
            // NOTE(chowie): Allows suffixes
            nValue = ToRadix40(WrapZ("MA_")); // Ex. Group/Folder
            nValue = ToRadix40(nValue, WrapZ("Rarity")); // Ex. Property

            Assert(nValue == ToRadix40(WrapZ("MA_Rarity")));

            // NOTE(chowie): Allows prefixes
//            u64 nValuePrefix = ToRadix40(WrapZ("Lister"));
//            u64 nValueSuffix = ToRadix40(WrapZ("Panel"));
//
//            nValue = ToRadix40Mult(&TextArena, nValuePrefix, nValueSuffix);
        } break;

        // COULDDO(chowie): Generalise? Just concat.. really?
        case SymGeneratorRadix40Type_Conword:
        {
            // NOTE(chowie): Typical situation, developers/modders typing in-editor
            nValue = ConwordNameToRadix40(ConFamily_Euro, WrapZ("Vb"), WrapZ("(chrisgr)"));
        } break;
        
        case SymGeneratorRadix40Type_Repligram:
        {
            // TODO(chowie): Fill in proper
// - byte 0-1   = Basekit Name e.g. Ut (Utility = kits made of other kits, e.g. arch walls with pipes), Pi (Pipe), In (Industrial), De (Deco = MultiLevel Interior/Exterior), Bu (Building), Tu (Steam Tunnel = Vertical), Cav (Cave), Pr (Prop/Set Piece), Gl (Glue = Doorframes, blending between different kits/tiles), He (Hero = Inflexible custom set pieces)
// - byte 2-5   = Subkit Name e.g. In Pr Basekit: Mach (Machine), Foli (Foliage), Gree (Greebles/Decals), Hole (Hole e.g. Wall with punched holes)
// - byte 6     = Piece Type e.g. 1 (1 way corridor), 2 (hallway turn/corner), 3 (junction), 4 (junction)
// - byte 7     = Spatial Type e.g. I (Indoor), O (Outdoor), S (Semi-enclosed/Threshold spaces e.g. porches, verandahs)
// - byte 8     = Tiling Direction e.g. 0 (None), V (Vertically), H (Horizontally), N, S, E, W (Cardinal Directions)
// - byte 9     = Animation Repligram Trigger e.g. 0 (None, Static/Decal), P (on proximity of player/mob entity), N (autonomously = waits for nature/time e.g. tree shrivelling), F (Fire), R (Rain), W (Water), O (Oxygen), D (Damage), I (Illness)
// - byte 10-11 = Variant Code e.g. a1, b1, c1 etc (instead of 00, 01, 02 etc), nicer to view for artist
//            nValue = ConwordNameToRadix40(ConFamily_Euro, WrapZ("Vb"), WrapZ("(chrisgr)"));;
        } break;

        //
        // IMPORTANT(chowie): Game
        //

        case SymGeneratorRadix40Type_GapBuffer:
        {
            // NOTE(chowie): Typical situation, players typing in-game
            nValue = ToRadix40Chop(BundleString(Gap->Buffer.Data, GapLen(Gap)), GapLen(Gap));
        } break;

        InvalidDefaultCase;
    }

    //
    //
    //

    // IMPORTANT TODO(chowie): Parse/extract data from string file formats!

    string FromRadixStringTest = FromRadix40(&TextArena, nValue, false);

    char FormatTextBuffer[64];
    _snprintf_s(FormatTextBuffer, sizeof(FormatTextBuffer),
                "Reconstructed String~ %s\n", FromRadixStringTest.Data);
    OutputDebugStringA(FormatTextBuffer);

    string ParseString = StripSymIndicatorRadix40IfPossible(FromRadixStringTest);
    switch(Source.Type)
    {
        case SymGeneratorRadix40Type_Conword:
        {
            // TODO(chowie): Convert to struct? With output types?
            u64 ParseRadix40 = ToRadix40Range(ParseString, 0, 1);
            switch(ToDigit(Radix40ToAscii.Data[ParseRadix40]))
            {
                case ConFamily_InDev:
                {
                } break;

                case ConFamily_Euro:
                {
                } break;

                case ConFamily_Asia:
                {
                } break;

                case ConFamily_Calc:
                {
                } break;

                case ConFamily_Base10:
                {
                } break;

                InvalidDefaultCase;
            }

            // NOTE(chowie): Conword Tag
            ParseRadix40 = ToRadix40Range(ParseString, 1, 3);
            string TestEnumString = FromRadix40(&TextArena, ParseRadix40);
            switch(ParseRadix40)
            {
                case $ConPos_Nn:
                {
                } break;

                case $ConPos_Vb:
                {
                } break;

                case $ConPos_Qn:
                {
                } break;

                InvalidDefaultCase;
            }

            char ConwordTagTextBuffer[64];
            _snprintf_s(ConwordTagTextBuffer, sizeof(ConwordTagTextBuffer),
                        "ConwordTag~ %s\n", TestEnumString.Data);
            OutputDebugStringA(ConwordTagTextBuffer);

            // NOTE(chowie): String Name
            ParseRadix40 = ToRadix40Range(ParseString, 3, ParseString.Size);
            string ConwordString = FromRadix40(&TextArena, ParseRadix40, false, ConlangRadix40ToAscii);

            char ConwordTextBuffer[32];
            _snprintf_s(ConwordTextBuffer, sizeof(ConwordTextBuffer),
                        "Conword~ %s\n", ConwordString.Data);
            OutputDebugStringA(ConwordTextBuffer);
        } break;

        case SymGeneratorRadix40Type_Repligram:
        {
        };

        default:
        {
            // TODO(chowie): Log unrecognised type
        } break;
    }

    // TODO(chowie): Parse string into args, exported to symgenerator_source
    // And exported depending on state! Switch is used to render ordinarily.

    //
    //
    //

    // COULDDO(chowie): Convert all Conword with conword_slots for FIELD_ARRAY .E?
    conword *Conword = PushStruct(&TextArena, conword);

    conword_slot *Primary = &Conword->Primary;
    Primary->NameRadix40 = nValue;
    Primary->ArchiacNameRadix40 = 0; // Intentionally blank string
    Primary->ShortDevMeaningRadix40 = ToRadix40(WrapZ("to go"));

    conword_slot *Secondary = &Conword->Secondary;
    Secondary->NameRadix40 = ToRadix40(WrapZ("0nnVHAIYA")); // TODO(chowie): Strip brackets from primary?
    Secondary->ArchiacNameRadix40 = 0; // Intentionally blank string
    Secondary->ShortDevMeaningRadix40 = ToRadix40(WrapZ("rain"));

    //

    // COULDDO(chowie): API to show both? Change to 'string **Bundle'?
    u32 SlotCount = Len(Primary->E); // NOTE(chowie): Because field array, any Len(E) will do

    string *PrimaryBundle = PushArray(&TextArena, SlotCount, string);
    FromRadix40Bundle(&TextArena, PrimaryBundle, Primary);

    foreachN(u32, BundleIndex, SlotCount)
    {
        string *Stock = PrimaryBundle + BundleIndex;

        char BundleBuffer[64];
        _snprintf_s(BundleBuffer, sizeof(BundleBuffer),
                    "Bundle String~ %s\n", Stock->Data);
        OutputDebugStringA(BundleBuffer);
    }

    string *SecondaryBundle = PushArray(&TextArena, SlotCount, string);
    FromRadix40Bundle(&TextArena, SecondaryBundle, Secondary);

    foreachN(u32, BundleIndex, SlotCount)
    {
        string *Stock = SecondaryBundle + BundleIndex;

        char BundleBuffer[64];
        _snprintf_s(BundleBuffer, sizeof(BundleBuffer),
                    "Bundle String~ %s\n", Stock->Data);
        OutputDebugStringA(BundleBuffer);
    }

    //

    // NOTE(chowie): Fuzzy match criteria
//    1. remove i-th character,
//    2. replace i-th with an ASCII letters,
//    3. swap i-th and j-th characters,
//    4. insert at i-th position an ASCII letter.

//    string Word = CONST_STRING("rebase");
//    string Misspelled = CONST_STRING("rebsae");
//
//    u32 h1[ASCII_BYTE_HISTOGRAM_MAX] = {};
//    ByteHistogram(h1, Word);
//
//    u32 h2[ASCII_BYTE_HISTOGRAM_MAX] = {};
//    ByteHistogram(h2, Misspelled);
//
//    u32 Diff = FuzzyMatch(h1, h2);
//
//    char HistogramTextBuffer[64];
//    _snprintf_s(HistogramTextBuffer, sizeof(HistogramTextBuffer),
//                "Fuzzy Match Test~ %u\n", Diff);
//    OutputDebugStringA(HistogramTextBuffer);

    u32 SlotHistogram1[ASCII_BYTE_HISTOGRAM_MAX] = {};
    string *PrimaryStock = PrimaryBundle + 0;
    ByteHistogram(SlotHistogram1, *PrimaryStock);

    u32 SlotHistogram2[ASCII_BYTE_HISTOGRAM_MAX] = {};
    string *SecondaryStock = SecondaryBundle + 0;
    ByteHistogram(SlotHistogram2, *SecondaryStock);

    u32 SlotHistogramDiff = FuzzyMatch(SlotHistogram1, SlotHistogram2);
    
    char SlotHistogramTextBuffer[64];
    _snprintf_s(SlotHistogramTextBuffer, sizeof(SlotHistogramTextBuffer),
                "Slot Fuzzy Match~ %u\n", SlotHistogramDiff);
    OutputDebugStringA(SlotHistogramTextBuffer);

    string SlotNeedle = PushString(&TextArena, WrapZ("IYA"));
    ToLower(SlotNeedle);
    umm SlotSubstringPosition = Substring(*SecondaryStock, SlotNeedle);

    char SlotSubstringTextBuffer[128];
    _snprintf_s(SlotSubstringTextBuffer, sizeof(SlotSubstringTextBuffer),
                "SlotSubstring Test at CursorP~ %d\n", SlotSubstringPosition);
    OutputDebugStringA(SlotSubstringTextBuffer);

    //
    //
    //

    // NOTE(chowie): For any asset
    string File = CONST_STRING("folder/texture/LaLogDLHa1.png");

    // NOTE(chowie): Cleanup file path
    string LastSlash = SkipLastSlash(File);
    string TextureFileType = SkipLastDot(File);

    char LastDotBuffer[64];
    _snprintf_s(LastDotBuffer, sizeof(LastDotBuffer),
                "FileType~ %s\n", TextureFileType.Data);
    OutputDebugStringA(LastDotBuffer);

    //

    // IMPORTANT(chowie): Chop() finds position, reducing its size and
    // "doesn't chop"

    // TODO(chowie): Store FileNameHash to asset packer struct, then
    // you can convert to radix40 on-demand.
    u64 TextureFileTypeRadix40 = ToRadix40(TextureFileType);

    string FileDotTest = ChopLastDot(LastSlash);
    u64 FileNameRadix40 = ToRadix40Range(FileDotTest, 1, FileDotTest.Size); // NOTE(chowie): Optimisation to remove first slash, but would otherwise ignore it

    switch(TextureFileTypeRadix40)
    {
        case $ttf:
        {
        } break;

        case $bmp:
        {
        } break;

        case $png:
        {
        } break;

        InvalidDefaultCase;
    }

    string StringFileName = FromRadix40(&TextArena, FileNameRadix40);

    char ConvertBuffer[64];
    _snprintf_s(ConvertBuffer, sizeof(ConvertBuffer),
                "Cut Radix40~ %s\n", StringFileName.Data);
    OutputDebugStringA(ConvertBuffer);

    // COULDDO(chowie): String undo stack?

    //
    //
    //

    // TODO(chowie):
    // - Parser/tokenizer
    // - Save string hash radix40 in asset packer struct

    // COULDDO(chowie):
    // - Timeline (tag time with string)?
    // - Instead of X for shape in texture format naming convention,
    //   use Y? Need to update blockbench!

    //
    //
    //

    ClearArena(&TextArena);

    return(0);
}

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

