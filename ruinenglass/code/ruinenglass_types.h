#if !defined(RUINENGLASS_TYPES_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Michael Chow $
   $Notice: $
   ======================================================================== */

//
// NOTE(chowie): Compilers
//

#if !defined(COMPILER_MSVC)
#define COMPILER_MSVC 0
#endif

#if !defined(COMPILER_LLVM)
#define COMPILER_LLVM 0
#endif

#if !COMPILER_MSVC && !COMPILER_LLVM
#if _MSC_VER
#undef COMPILER_MSVC
#define COMPILER_MSVC 1
#else
// TODO(chowie): More compilers
#undef COMPILER_LLVM
#define COMPILER_LLVM 1
#endif
#endif

#if COMPILER_MSVC
#include <intrin.h>
#elif COMPILER_LLVM
#include <x86intrin.h>
#else
#error SSE/NEON optimisations are not available for this platform yet!!
#endif

//
// NOTE(chowie): User-defined types
//

#if !defined(internal)
#define internal static
#endif
#define local_persist static
#define global static

#include <stdint.h>
#include <limits.h>
#include <float.h>

// RESOURCE: https://learn.microsoft.com/en-us/cpp/cpp/data-type-ranges?view=msvc-170
// STUDY(chowie): Sizeof different types in Windows
typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

// RESOURCE: https://hero.handmade.network/forums/code-discussion/t/1449-b32x%252C_memory_index%252C_umm%252C_smm
// NOTE(chowie): Boolean in whatever size is comfortable for the
// compiler but not less than 32 bits (where it could be more efficient).
typedef int_least32_t b32x;
// typedef s32 b32; // NOTE(chowie): Avoids C4800 in Wall

// NOTE(chowie): On systems with segmented memory, size_t used as an
// index would hold only an offset within a segment, but uintptr_t
// would hold both a segment and an offset.
// typedef size_t memory_index;
typedef intptr_t smm;
typedef uintptr_t umm;

typedef float r32;
typedef double r64;

#define Pi32 3.14159265359f
#define Tau32 6.28318530717958647692f
#define GoldenRatio64 1.61803398874989484820458683436563f

#if RUINENGLASS_SLOW
#define Assert(Expression) if(!(Expression)) {*(volatile int *)0 = 0;}
// TODO(chowie): Explore proper logging functions? Connect with platform_api?
#define Log(Message) 
#else
#define Assert(Expression)
#define Log(Message)
#endif

// IMPORTANT(chowie): Assert in places I never expect code to run in practise.
#define InvalidCodePath Assert(!"InvalidCodePath")
#define InvalidDefaultCase default: {InvalidCodePath;} break

// TODO(chowie): Should these always be 64-bit?
// NOTE(chowie): "LL" prevents integral promotion to 32-bits, instead wanting 64-bits
#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value)*1024LL)
#define Gigabytes(Value) (Megabytes(Value)*1024LL)
#define Terabytes(Value) (Gigabytes(Value)*1024LL)

#define ArrayCount(arr) (sizeof((arr)) / (sizeof((arr)[0])))

#define Swap(type, A, B) {type Temp = (A); (A) = (B); (B) = Temp;}

#define Minimum(A, B) ((A < B) ? (A) : (B))
#define Maximum(A, B) ((A > B) ? (A) : (B))
//#define Maximum3(A, B, C) (Max(A, Max(B, C)))
// RESOURCE(bipll, 0_): https://stackoverflow.com/questions/61106977/check-max-value-from-three-variables-by-using-preprocessor-in-c
#define Maximum3(A, B, C) ((A) <= (B) ? (B) <= (C) ? (C) : (B) : (A) <= (C) ? (C) : (A))
// #define Maximum3(A, B, C) ((A) > (B) ? ((A) > (C) ? (A) : ((C) > (B) ? C : (B))) : ((B) > (C) ? (B) : (C)))

// NOTE(chowie): Limit macros
#define R32Maximum FLT_MAX
#define R32Minimum -FLT_MAX
#define U32Maximum UINT32_MAX
#define U16Maximum UINT16_MAX
#define U8Maximum UINT8_MAX

#define Odd(Value) ((Value) & 1)

// TODO(chowie): Figure out where to use Align16 for code I care about
// the most; For SIMD?
// NOTE(chowie): "(Value-Value)" forces integral promotion to size of Value
#define AlignPow2(Value, Alignment) ((Value + (Alignment - 1)) & ~((Alignment) - 1))
#define Align4(Value) ((Value + 3) & ~3)
#define Align8(Value) ((Value + 7) & ~7)
#define Align16(Value) ((Value + 15) & ~15)

inline b32x
IsPow2(u32 Value)
{
    b32x Result = ((Value & ~(Value - 1)) == Value);
    return(Result);
}

// NOTE(chowie): Explicit cast - no data should really be > hi32-bits
inline u32
SafeTruncateU64(u64 Value)
{
    Assert(Value <= U32Maximum);
    u32 Result = (u32)Value;
    return(Result);
}

// RESOURCE: https://github.com/gingerBill/gb/blob/master/gb.h
// TODO(chowie): Utilise these!
#define BitSet(Bit) (1 << (Bit))
#define MaskSet(Var, Set, Mask) do {            \
        if(Set) (Var) |= (Mask);                \
        else    (Var) &= ~(Mask);               \
} while(0)

// RESOURCE: https://handmade.network/p/64/geometer/blog/p/3048-1_year_of_geometer_-_lessons_learnt
// TODO(chowie): Try this out? See how I like this?
#define foreach(type, Value, array) for(type Value = 0; Value < ArrayCount(array); ++Value)

// RESOURCE: https://hero.handmade.network/forums/code-discussion/t/3190-field_array_implementation_union_of_fields_and_array
// NOTE(chowie): From Blake "rationalcoder" Martin. Syntactic sugar
// for vector likes (e.g. v2, v3). The downside is bad introspection;
// usually you wouldn't care for this kind of struct. No need for a
// terminator/fake field check!
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

//
// NOTE: Math types
//

// TODO(chowie): Combine v2u and v2s together?
union v2u
{
    struct
    {
        u32 x, y;
    };
    struct
    {
        u32 Width, Height;
    };
    u32 E[2];
};

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
    s32 E[2];
};

// NOTE(chowie): Vectors was uppercase, but should be lowercase to
// accomodate shader languages!
union v2
{
    struct
    {
        r32 x, y;
    };
    struct
    {
        r32 u, v;
    };
    struct
    {
        r32 Width, Height;
    };
    struct
    {
        r32 Min, Max;
    };
    r32 E[2];
};

union v3u
{
    struct
    {
        u32 x, y, z;
    };
    u32 E[3];
};

union v3s
{
    struct
    {
        s32 x, y, z;
    };
    s32 E[3];
};

union v3
{
    struct
    {
        r32 x, y, z;
    };
    struct
    {
        r32 u, v, w;
    };
    struct
    {
        r32 r, g, b;
    };
    struct
    {
        v2 xy;
        r32 Ignored0_;
    };
    struct
    {
        r32 Ignored1_;
        v2 yz;
    };
    struct
    {
        v2 uv;
        r32 Ignored2_;
    };
    struct
    {
        r32 Ignored3_;
        v2 vw;
    };
    r32 E[3];
};

union v4
{
    struct
    {
        union
        {
            v3 xyz;
            struct
            {
                r32 x, y, z;
            };
        };

        r32 w;
    };
    struct
    {
        union
        {
            v3 rgb;
            struct
            {
                r32 r, g, b;
            };
        };

        r32 a;
    };
    struct
    {
        v2 xy;
        r32 Ignored0_;
        r32 Ignored1_;
    };
    struct
    {
        r32 Ignored2_;
        v2 yz;
        r32 Ignored3_;
    };
    struct
    {
        r32 Ignored4_;
        v2 zw;
        r32 Ignored5_;
    };
    r32 E[4];
};

struct rectangle2
{
    v2 Min;
    v2 Max;
};

struct rectangle3
{
    v3 Min;
    v3 Max;
};

// TODO(chowie): Debug View
#define FILE_AND_LINE__(A, B) A "|" #B
#define FILE_AND_LINE_(A, B) FILE_AND_LINE__(A, B)
#define FILE_AND_LINE FILE_AND_LINE_(__FILE__, __LINE__)

// NOTE(chowie): This is purely for cstrings
// TODO(chowie): This should not be necessary anymore. Remove!
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

// RESOURCE(fabien): https://github.com/fabiensanglard/Shmup/blob/master/engine/src/math.c
inline void
StringReplace(char *String, char Source, char Dest)
{
    for(umm Index = 0;
        Index < StringLength(String);
        ++Index)
    {
        if(String[Index] == Source)
        {
            String[Index] = Dest;
        }
    }
}

// RESOURCE: https://github.com/cmuratori/computer_enhance/blob/main/perfaware/part2/listing_0068_buffer.cpp
// TODO(chowie): De/Allocate buffer?
struct buffer
{
    umm Size;
    u8 *Data;
};
typedef buffer string;

#define CONSTANT_STRING(String) {sizeof(String) - 1, (u8 *)(String)}

internal b32x
BufferIsValid(buffer Source)
{
    b32x Result = (Source.Data != 0);
    return(Result);
}

internal b32x
BufferIsInBounds(buffer Source, umm At)
{
    b32x Result = (At < Source.Size);
    return(Result);
}

internal b32x
BufferAreEqual(buffer A, buffer B)
{
    b32x Result = true;
    if(A.Size != B.Size)
    {
        Result = false;
    }

    for(u64 Index = 0;
        Index < A.Size;
        ++Index)
    {
        if(A.Data[Index] != B.Data[Index])
        {
            Result = false;
        }
    }

    return(Result);
}

//
// NOTE(chowie): Linked Lists from Mr. 4th
//

#define DLLPushBack_NP(f,l,n,Next,Prev) ((f)==0?    \
((f)=(l)=(n),(n)->Next=(n)->Prev=0):\
((n)->Prev=(l),(l)->Next=(n),(l)=(n),(n)->Next=0))
#define DLLPushBack(f,l,n) DLLPushBack_NP(f,l,n,Next,Prev)

#define DLLPushFront(f,l,n) DLLPushBack_NP(l,f,n,Prev,Next)

#define DLLRemove_NP(f,l,n,Next,Prev) ((f)==(n)?\
((f)==(l)?\
((f)=(l)=(0)):\
((f)=(f)->Next,(f)->Prev=0)):\
(l)==(n)?\
((l)=(l)->Prev,(l)->Next=0):\
((n)->Next->Prev=(n)->Prev,\
(n)->Prev->Next=(n)->Next))
#define DLLRemove(f,l,n) DLLRemove_NP(f,l,n,Next,Prev)

#define SLLQueuePush_N(f,l,n,Next) (((f)==0?\
(f)=(l)=(n):\
((l)->Next=(n),(l)=(n))),\
(n)->Next=0)
#define SLLQueuePush(f,l,n) SLLQueuePush_N(f,l,n,Next)

#define SLLQueuePushFront_N(f,l,n,Next) ((f)==0?\
((f)=(l)=(n),(n)->Next=0):\
((n)->Next=(f),(f)=(n)))
#define SLLQueuePushFront(f,l,n) SLLQueuePushFront_N(f,l,n,Next)

#define SLLQueuePop_N(f,l,Next) ((f)==(l)?\
(f)=(l)=0:\
((f)=(f)->Next))
#define SLLQueuePop(f,l) SLLQueuePop_N(f,l,Next)

#define SLLStackPush_N(f,n,Next) ((n)->Next=(f),(f)=(n))
#define SLLStackPush(f,n) SLLStackPush_N(f,n,Next)

#define SLLStackPop_N(f,Next) ((f)==0?0:\
((f)=(f)->Next))
#define SLLStackPop(f) SLLStackPop_N(f,Next)

//
// NOTE(chowie): Multi-threading
//

#if COMPILER_MSVC
#define CompletePreviousReadsBeforeFutureReads _ReadBarrier()
#define CompletePreviousWritesBeforeFutureWrites _WriteBarrier()
inline u32 AtomicCompareExchangeU32(u32 volatile *Value, u32 New, u32 Expected)
{
    u32 Result = _InterlockedCompareExchange((long *)Value, New, Expected);

    return(Result);
}
inline u64 AtomicCompareExchangeU64(u64 volatile *Value, u64 New, u64 Expected)
{
    u64 Result = _InterlockedCompareExchange64((long long *)Value, New, Expected);

    return(Result);
}
inline u64 AtomicExchangeU64(u64 volatile *Value, u64 New)
{
    u64 Result = _InterlockedExchange64((__int64 *)Value, New);

    return(Result);
}
// NOTE: Could be Atomicincrement, but add has more flexibility
// Linux equivalent "(__sync_fetch_and_add(Value, Addend) + Addend)"
inline u64 AtomicAddU64(u64 volatile *Value, u64 Addend)
{
    // NOTE: Returns the original value _prior_ to adding
    u64 Result = _InterlockedExchangeAdd64((__int64 *)Value, Addend);

    return(Result);
}
inline u32 GetThreadID(void)
{
    u8 *ThreadLocalStorage = (u8 *)__readgsqword(0x30);
    u32 ThreadID = *(u32 *)(ThreadLocalStorage + 0x48);

    return(ThreadID);
}
#endif
// TODO(chowie): COMPILER_LLVM

#define RUINENGLASS_TYPES_H
#endif
