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

  RESOURCE(chowie): https://austinmorlan.com/posts/pass_by_value_vs_pointer/
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
*/

#ifdef __cplusplus
extern "C" {
#endif

//
// NOTE: Compilers
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
// NOTE: User-defined types
//

#if !defined(internal)
#define internal static
#endif
#define local_persist static
#define global_variable static

#include <stdint.h>
#include <limits.h>
#include <float.h>

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;
typedef s32 b32; // NOTE(chowie): Avoids C4800 in Wall

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef intptr_t smm;
typedef uintptr_t umm;

typedef size_t memory_index;

typedef float r32;
typedef double r64;

#if COMPILER_LLVM
typedef unsigned __int128 u128;
typedef __int128 s128;
#endif

#define Pi32 3.14159265359f
#define Tau32 6.28318530717958647692f

#if RUINENGLASS_SLOW
#define Assert(Expression) if(!(Expression)) {*(int *)0 = 0;}
#else
#define Assert(Expression)
#endif

// NOTE(chowie): Assert in places I never expect code to run in practise.
#define InvalidCodePath Assert(!"InvalidCodePath")
#define InvalidDefaultCase default: {InvalidCodePath;} break

// RESOURCE: https://handmade.network/p/64/geometer/blog/p/3048-1_year_of_geometer_-_lessons_learnt
// TODO(chowie): Try this out? See how I like this?
#define foreach(type, Value, array) (type Value = 0; Value < ArrayCount(array); ++Value)

// TODO(chowie): Should these always be 64-bit?
#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value)*1024LL)
#define Gigabytes(Value) (Megabytes(Value)*1024LL)
#define Terabytes(Value) (Gigabytes(Value)*1024LL)

#define ArrayCount(arr) (sizeof((arr)) / (sizeof((arr)[0])))

#define Swap(type, A, B) {type Temp = (A); (A) = (B); (B) = Temp;}
#define Minimum(A, B) ((A < B) ? (A) : (B))
#define Maximum(A, B) ((A > B) ? (A) : (B))

// RESOURCE: https://github.com/gingerBill/gb/blob/master/gb.h
// STUDY: Utilise these!
#define BitSet(Bit) (1 << (Bit))
#define MaskSet(Var, Set, Mask) do {            \
        if(Set) (Var) |= (Mask);                \
        else    (Var) &= ~(Mask);               \
} while(0)

#define R32Maximum FLT_MAX
#define R32Minimum -FLT_MAX

// RESOURCE: https://hero.handmade.network/forums/code-discussion/t/3190-field_array_implementation_union_of_fields_and_array
// NOTE(chowie): From Blake "rationalcoder" Martin. Syntactic sugar
// for vector likes (e.g. v2, v3). The downside is bad introspection,
// usually you wouldn't care for this kind of struct.
// TODO(chowie): Field array for memory arenas

#define Concat(A, B) A##B

#define FIELD_ARRAY_(type, struct_definition, counter)                  \
typedef struct struct_definition Concat(_anon_array, counter);           \
union {                                                                 \
    struct struct_definition;                                            \
    type E[sizeof(Concat(_anon_array, counter)) / sizeof(type)];        \
};                                                                      \
static_assert(sizeof(Concat(_anon_array, counter)) % sizeof(type) == 0, \
              "Field Array of type '" #type "' must be a multiple of sizeof(" #type ")")\

#define FIELD_ARRAY(type, struct_definition)             \
FIELD_ARRAY_(type, struct_definition, __COUNTER__)

//
// NOTE: Math types
//

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
    r32 E[2];
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

//
// NOTE: Shared Utils
//
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

//
// NOTE: Services that the platform layer provides to game
//

//
// NOTE: Services that the game provides to the platform layer.
// Takes - timing, controller/keyboard input, bitmap buffer to use, sound buffer to use.
//

typedef struct game_sound_output_buffer
{
    int SamplesPerSecond;
    int SampleCount;

    // IMPORTANT(chowie): Samples must be padded to a multiple of 4!
    s16 *Samples;
} game_sound_output_buffer;

#define BITMAP_BYTES_PER_PIXEL 4
typedef struct game_offscreen_buffer
{
    void *Memory;
    s32 Width;
    s32 Height;
    s32 Pitch;
} game_offscreen_buffer;

#ifdef __cplusplus
}
#endif

// TODO(chowie): Remove this! And link functions!
#include "ruinenglass_shared.h"

#define RUINENGLASS_PLATFORM_H
#endif
