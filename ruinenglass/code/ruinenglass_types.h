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
typedef uintptr_t umm; // NOTE(chowie): Memory-sized uint

typedef float r32;
typedef double r64;

#define U32FromPointer(Pointer) ((u32)(umm)(Pointer))
#define PointerFromU32(type, Value) (type *)((umm)Value)

#define Pi32 3.14159265359f
#define Tau32 6.28318530717958647692f
#define Euler32 2.718281828459045f
#define GoldenRatio64 1.61803398874989484820458683436563f

#if RUINENGLASS_SLOW
#define Assert(Expression) if(!(Expression)) {*(volatile int *)0 = 0;}
// TODO(chowie): Explore proper logging functions? Connect with platform_api?
#define Logging(Message) 
#else
#define Assert(Expression)
#define Logging(Message)
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
#define Minimum3(A, B, C) (Minimum(A, Minimum(B, C)))
#define Maximum(A, B) ((A > B) ? (A) : (B))
#define Maximum3(A, B, C) (Maximum(A, Maximum(B, C)))

// NOTE(chowie): Limit macros
#define R32Max FLT_MAX
#define R32Min -FLT_MAX
#define S32Max INT32_MAX
#define S32Min -INT32_MAX
#define U32Max ((u32) - 1)
#define U16Max ((u16) - 1)
#define U8Max ((u8) - 1)

#define Odd(Value) ((Value) & 1)

// TODO(chowie): Pow2? When mapping, there might be (number-theoretic
// reasons) not to be a Pow2. Out the hash function, it directly
// truncates the bits.
#define HashSizePow2 4096

// RESOURCE: https://web.archive.org/web/20211023131624/https://lolengine.net/blog/2012/4/3/beyond-de-bruijn
// RESOURCE: https://web.archive.org/web/20210724051712/https://lolengine.net/attachment/blog/2012/4/3/beyond-de-bruijn/debruijn.cpp
// NOTE(chowie): Beyond De Bruijn: fast binary logarithm of a 10-bit number
// STUDY(chowie): Computing the binary logarithm is
// equivalent to knowing the position of the highest order set
// bit. For instance, log2(0x1) is 0 and log2(0x100) is 8.
// TODO(chowie): Use for mipmapping?
global s32 MagicTable[16] = 
{
    0, 1, 2, 8, -1, 3, 5, 9, 9, 7, 4, -1, 6, -1, -1, -1,
};
inline s32
Log2(u32 Value)
{
    Value |= Value >> 1;
    Value |= Value >> 2;
    Value |= Value >> 4;
    s32 Result = MagicTable[(u32)(Value * 0x5a1a1a2u) >> 28];
    return(Result);
}

// RESOURCE(): Includes other math functions - https://github.com/romeric/fastapprox/tree/master/fastapprox/src
// RESOURCE(): https://stackoverflow.com/questions/9411823/fast-log2float-x-implementation-c
// TODO(chowie): r32 Log2?

// RESOURCE(mineiro): https://web.archive.org/web/20150113003634/http://fastapprox.googlecode.com/svn/trunk/fastapprox/src/fastonebigheader.h
// TODO(chowie): LGamma? Trig? Lambert? Sigmoid?

// TODO(chowie): Do I need 64-bit versions?
// RESOURCE(ankerl): http://martin.ankerl.com/2007/10/04/optimized-pow-approximation-for-java-and-c-c/
// RESOURCE(ekmett): https://github.com/ekmett/approximate/blob/master/cbits/fast.c
// NOTE(from ekmett): These can be _quite_ inaccurate. ~20% in many cases, but being much faster (~7x) may
// * permit more loop iterations of tuning algorithms that only need approximate powers.
union rational_approx
{
    r32 f;
    s32 x;
};
inline r32
Exp(r32 Value)
{
    rational_approx Exp;
    Exp.x = (s32)(12102203*Value + 1064866805);
    return(Exp.f);
}

inline r32
Log(r32 Value)
{
    rational_approx Log = { Value };
    r32 Result = (Log.x - 1064866805)*8.262958405176314e-8f; /* 1 / 12102203.0; */
    return(Result);
}

inline r32
Pow(r32 A, r32 B)
{
    rational_approx Pow = { A };
    Pow.x = (s32)(B*(Pow.x - 1064866805) + 1064866805);
    return(Pow.f);
}

global u8
PowiHighestBitSet[] =
{
    0, 1, 2, 2, 3, 3, 3, 3,
    4, 4, 4, 4, 4, 4, 4, 4,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 6,
    6, 6, 6, 6, 6, 6, 6, 255, // NOTE(from orlp): Anything past 63 overflows with base > 1
    255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255,
};
// RESOURCE(orlp): https://stackoverflow.com/questions/101439/the-most-efficient-way-to-implement-an-integer-based-power-function-powint-int
// RESOURCE(orlp): https://gist.github.com/orlp/3551590
// IMPORTANT(chowie): Must pass in s32, s64 is to not overflow for 16^8
inline s64
Powi(s64 Base, u8 Exp)
{
    // NOTE(from orlp): Use 255 as an overflow/underflow marker
    Assert(Exp && (PowiHighestBitSet[Exp] != 255));

    s64 Result = 1;
    switch(PowiHighestBitSet[Exp])
    {
        // STUDY(chowie): Cascades down
        case 6:
        {
            if(Odd(Exp))
            {
                Result *= Base;
            }
            Exp >>= 1;
            Base *= Base;
        }
        case 5:
        {
            if(Odd(Exp))
            {
                Result *= Base;
            }
            Exp >>= 1;
            Base *= Base;
        }
        case 4:
        {
            if(Odd(Exp))
            {
                Result *= Base;
            }
            Exp >>= 1;
            Base *= Base;
        }
        case 3:
        {
            if(Odd(Exp))
            {
                Result *= Base;
            }
            Exp >>= 1;
            Base *= Base;
        }
        case 2:
        {
            if(Odd(Exp))
            {
                Result *= Base;
            }
            Exp >>= 1;
            Base *= Base;
        }
        case 1:
        {
            if(Odd(Exp))
            {
                Result *= Base;
            }
        }
    }

    return(Result);
}        

// TODO(chowie): Figure out where to use Align16 for code I care about
// the most; For SIMD?
// NOTE(chowie): "(Value-Value)" forces integral promotion to size of Value
#define AlignPow2(Value, Alignment) ((Value + (Alignment - 1)) & ~((Alignment) - 1))
#define Align4(Value) ((Value + 3) & ~3)
#define Align8(Value) ((Value + 7) & ~7)
#define Align16(Value) ((Value + 15) & ~15)

// RESOURCE(fabien): https://fgiesen.wordpress.com/2016/10/26/rounding-up-to-the-nearest-int-k-mod-n/
// NOTE(chowie): Round to nearest congrument k % Alignment
// TODO(chowie): Can I use this for memory allocators, which address falls off alignment by specified distance.
#define AlignPow(Value, Alignment, k) ((Value - k + (Alignment - 1)) & ~((Alignment) - 1) + k)

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
    Assert(Value <= U32Max);
    u32 Result = (u32)Value;
    return(Result);
}

#define BitSet(Bit) (1 << (Bit))

#define FlagSet(A, Flag) (A & Flag)
#define AddFlag(A, Flag) (A |= Flag)
#define ClearFlag(A, Flag) (A &= ~Flag)
#define ToggleFlag(A, Test, Flag) (Test) ? AddFlag(A, Flag) : ClearFlag(A, Flag);
// RESOURCE: https://github.com/gingerBill/gb/blob/master/gb.h
//#define MaskSet(Var, Set, Mask) if(Set) (Var) |= (Mask); else (Var) &= ~(Mask);

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
// NOTE: Math Primitives
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

// STUDY(chowie): Vectors was uppercase, but now lowercase for shader languages!
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
        r32 bxy, byz, bzx; // NOTE(chowie): Biv3
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
            v3 bxyyzzx; // NOTE(chowie): Biv3
            struct
            {
                r32 bxy, byz, bzx;
            };
        };

        r32 s; // NOTE(chowie): Scalar
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

union rect2
{
    struct
    {
        v2 Min;
        v2 Max;
    };
    struct
    {
        v2 Min;
        v2 MaxN; // NOTE(chowie): N is negative
    };
    r32 E[4];
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

// RESOURCE(): https://ktstephano.github.io/rendering/stratusgfx/aabbs
// RESOURCE(): https://ktstephano.github.io/rendering/stratusgfx/lod
// TODO(chowie): View fustrum culling? Need to take into account how
// much as the AABB takes up in screen space. Refer to how HmH sorting
// passing the screen space.
// TODO(chowie): 3D AABB collision detection?
union rect3
{
    struct
    {
        v3 Min;
        v3 Max;
    };
    struct
    {
        v3 Min;
        v3 MaxN;
    };
    r32 E[9];
};

// RESOURCE: Mat mult - https://gist.github.com/rygorous/4172889
union m2x2
{
    // NOTE(chowie): Optimisation to use with v2 Inner, only works
    // because of ROW-MAJOR order (doesn't work with COLUMN-MAJOR)
    struct
    {
        v2 RowA;
        v2 RowB;
    };
    // NOTE(chowie): ROW-MAJOR order - E[Row][Column]
    r32 E[2][2];
    // TODO(chowie): I could add SIMD like this, r32 E[4]; with E[2][2]; in struct?
};

union m3x3
{
    struct
    {
        v3 RowA;
        v3 RowB;
        v3 RowC;
    };
    // NOTE(chowie): ROW-MAJOR order - E[Row][Column]
    r32 E[3][3];
};

// RESOURCE(pervognsen): https://gist.github.com/pervognsen/983649888fa7c9075ae64e37c20f0ab3
// TODO(chowie): IMPORTANT(chowie): m4x4 Transpose
struct m4x4
{
    struct
    {
        v4 RowA;
        v4 RowB;
        v4 RowC;
        v4 RowD;
    };
    // NOTE(chowie): ROW-MAJOR order - E[Row][Column]
    r32 E[4][4];
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
// NOTE(chowie): Multi-threading
//

// STUDY: InterlockedCompareExchange allows thread work to be
// _predicated_, where you care less about the maximum performance of
// the interlocked point. Ideally, we would partion the work into
// large chunk to spend less time figuring out who does the work.
#if COMPILER_MSVC
// NOTE(chowie): MSVC treats "sfence" as WriteBarrier

// STUDY(chowie): Ensures correct timing for platforms who support
// out-of-order writes / doesn't support strong order to writes;
// explicitly serialises code. Guards weakly ordered above the fence.
#define CompletePrevWritesBeforeFutureWrites _WriteBarrier()
#define CompletePrevReadsBeforeFutureReads _ReadBarrier()
inline u32
AtomicIncrementU32(u32 volatile *Value)
{
    u32 Result = _InterlockedIncrement((long *)Value);
    return(Result);
}
inline u32
AtomicCompareExchangeU32(u32 volatile *Value, u32 New, u32 Expected)
{
    u32 Result = _InterlockedCompareExchange((long *)Value, New, Expected);
    return(Result);
}
inline u64
AtomicCompareExchangeU64(u64 volatile *Value, u64 New, u64 Expected)
{
    u64 Result = _InterlockedCompareExchange64((long long *)Value, New, Expected);
    return(Result);
}
inline u64
AtomicExchangeU64(u64 volatile *Value, u64 New)
{
    u64 Result = _InterlockedExchange64((__int64 *)Value, New);
    return(Result);
}
// NOTE: Could be Atomicincrement, but add has more flexibility
// Linux equivalent "(__sync_fetch_and_add(Value, Addend) + Addend)"
inline u64
AtomicAddU64(u64 volatile *Value, u64 Addend)
{
    // NOTE: Returns the original value _prior_ to adding
    u64 Result = _InterlockedExchangeAdd64((__int64 *)Value, Addend);
    return(Result);
}
inline u32
GetThreadID(void)
{
    u8 *ThreadLocalStorage = (u8 *)__readgsqword(0x30);
    u32 ThreadID = *(u32 *)(ThreadLocalStorage + 0x48);

    return(ThreadID);
}
#endif
// TODO(chowie): COMPILER_LLVM

/*

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
*/

#define RUINENGLASS_TYPES_H
#endif
