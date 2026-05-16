#if !defined(RUINENGLASS_INTRINSICS_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Michael Chow $
   $Notice: $
   ======================================================================== */

#if !defined _MSC_VER
#define B_NAN __builtin_nanf("")
#else
#define B_NAN (-(float)(1e300 * 1e300 * 0))
#endif

inline s32
SignOf(s32 Value)
{
    s32 Result = (Value >= 0) ? 1 : -1;
    return(Result);
}

inline f32
SignOf(f32 Value)
{
    __m128 SignMask = _mm_set1_ps(-0.0f); // NOTE(chowie): -0.0f = 1 << 31
    __m128 One = _mm_set_ss(1.0f);
    __m128 SignBit = _mm_and_ps(_mm_set_ss(Value), SignMask);
    __m128 Combined = _mm_or_ps(One, SignBit);

    f32 Result = _mm_cvtss_f32(Combined);
    return(Result);
}

inline f64
SignOf(f64 Value)
{
    __m128d SignMask = _mm_set1_pd(-0.0f); // NOTE(chowie): -0.0f = 1 << 31
    __m128d One = _mm_set_sd(1.0f);
    __m128d SignBit = _mm_and_pd(_mm_set_sd(Value), SignMask);
    __m128d Combined = _mm_or_pd(One, SignBit);

    f64 Result = _mm_cvtsd_f64(Combined);
    return(Result);
}

// RESOURCE(wychmaster): https://stackoverflow.com/questions/57870896/writing-a-portable-sse-avx-version-of-stdcopysign
// RESOURCE(theowl84): Same technique using andnot to negate can be observed here - http://fastcpp.blogspot.com/2011/03/changing-sign-of-float-values-using-sse.html
inline f32
CopySign(f32 Sign, f32 Value)
{
    __m128 SignMask = _mm_set1_ps(-0.0f);
    __m128 ExtractSign = _mm_and_ps(_mm_set_ss(Sign), SignMask);
    __m128 ExtractValue = _mm_andnot_ps(SignMask, _mm_set_ss(Value)); // STUDY(chowie): Equivalent of Abs(Value)

    f32 Result = _mm_cvtss_f32(_mm_or_ps(ExtractSign, ExtractValue));
    return(Result);
}

inline f32
Sqrt(f32 F32)
{
    f32 Result = _mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(F32)));
    return(Result);
}

inline f64
Sqrt(f64 F64)
{
    f64 Result = _mm_cvtsd_f64(_mm_sqrt_pd(_mm_set_sd(F64)));
    return(Result);
}

// RESOURCE(martins): https://handmade.network/forums/t/6940-understanding_basic_simd#21153
// RESOURCE(cbloom): http://cbloomrants.blogspot.com/2010/11/11-20-10-function-approximation-by_20.html
// NOTE(chowie): 1/sqrt(x), replaces normalising vectors
inline f32
InvSqrt(f32 F32)
{
    __m128 Half = _mm_set1_ps(0.5f);
    __m128 Three = _mm_set1_ps(3.0f);
    __m128 Value = _mm_set_ss(F32);
    __m128 Rsqrt = _mm_rsqrt_ss(Value);

    // NOTE(chowie): Newton's
    __m128 Temp = _mm_mul_ps(_mm_mul_ps(Value, Rsqrt), Rsqrt);
    Rsqrt = _mm_mul_ps(_mm_mul_ps(Half, Rsqrt), _mm_sub_ps(Three, Temp));

    f32 Result = _mm_cvtss_f32(Rsqrt);
    return(Result);
}

// RESOURCE(norbert): https://stackoverflow.com/questions/5508628/how-to-absolute-2-double-or-4-floats-using-sse-instruction-set-up-to-sse4
inline f32
Abs(f32 F32)
{
    __m128 SignMask = _mm_set1_ps(-0.0f); // NOTE(chowie): -0.0f = 1 << 31
    __m128 ExtractValue = _mm_andnot_ps(SignMask, _mm_set_ss(F32));

    f32 Result = _mm_cvtss_f32(ExtractValue);
    return(Result);
}

// RESOURCE(brumme): https://bits.stephan-brumme.com/absInteger.html
inline s32
Abs(s32 S32)
{
    s32 Shift = S32 >> 31;
    s32 Result = (S32 ^ Shift) - Shift; // NOTE(chowie) s32 Result = (Value < 0) ? -Value : Value;
    return(Result);
}

// TODO(chowie): Is this actually useful?
// RESOURCE(wojciech mula): http://0x80.pl/notesen/2018-03-11-sse-abs-unsigned.html
// NOTE(from mula): Maximum(A - B, 0); saturated arithmetic. Clamps to
// zero if subtraction is negative. Calculates two saturated
// subtracts, one for A - B and B - A; merge them with bitwise or.
// It's safe, because one of the subtract results is zero.
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
RotateLeft(u32 Value, s32 Amount)
{
#if COMPILER_MSVC
    u32 Result = _rotl(Value, Amount);
#else
    // TODO(chowie): Port to other compiler platforms
    Amount &= 31;
    u32 Result = ((Value << Amount) | (Value >> (32 - Amount)));
#endif

    return(Result);
}

inline u32
RotateRight(u32 Value, s32 Amount)
{
#if COMPILER_MSVC
    u32 Result = _rotr(Value, Amount);
#else
    // TODO(chowie): Port to other compiler platforms
    Amount &= 31;
    u32 Result = ((Value >> Amount) | (Value << (32 - Amount)));
#endif

    return(Result);
}

// NOTE(chowie): SSE4.1
inline f32
Round(f32 F32)
{
    f32 Result = _mm_cvtss_f32(_mm_round_ss(_mm_setzero_ps(), _mm_set_ss(F32),
                                            (_MM_FROUND_TO_NEAREST_INT |_MM_FROUND_NO_EXC)));
    return(Result);
}

inline s32
RoundF32ToS32(f32 F32)
{
    s32 Result = _mm_cvtss_si32(_mm_set_ss(F32));
    return(Result);
}

inline u32
RoundF32ToU32(f32 F32)
{
    u32 Result = (u32)_mm_cvtss_si32(_mm_set_ss(F32));
    return(Result);
}

/*
// NOTE(chowie): SSE2 Floor - Thanks martins, Includes INF & NaN
// RESOURCE(martins): https://gist.github.com/mmozeiko/56db3df14ab380152d6875383d0f4afd
internal f32
MartinsFloor(f32 Value)
{
    __m128 SignBit = _mm_set1_ps(-0.0f);
    __m128 One = _mm_set1_ps(1.0f);
    __m128 MaxValue = _mm_set1_ps(2147483648.0f); // TODO(chowie): 8388608.f if needed

    // NOTE(chowie): (float)(int)Float;
    __m128 Float = _mm_set_ss(Value);
    __m128 Result = _mm_cvtepi32_ps(_mm_cvttps_epi32(Float));

    // RESOURCE(christer): https://web.archive.org/web/20120119131226/http://realtimecollisiondetection.net/blog/?p=90
    // STUDY(chowie): This is an example of branchless selection
    // NOTE(chowie): if(Float < Result) Result -= 1;
    Result = _mm_sub_ss(Result, _mm_and_ps(_mm_cmplt_ss(Float, Result), One));

    // NOTE(chowie): if(!(2**31 > Abs(Float))) Result = Float;
    __m128 Mag = _mm_cmpgt_ss(MaxValue, _mm_andnot_ps(SignBit, Float));
    Result = _mm_or_ps(_mm_and_ps(Mag, Float), _mm_andnot_ps(Mag, Float));

    return(_mm_cvtss_f32(Result));
}
*/

// NOTE(chowie): SSE 4.1
internal f32
Floor(f32 F32)
{
    f32 Result = _mm_cvtss_f32(_mm_floor_ss(_mm_setzero_ps(), _mm_set_ss(F32)));
    return(Result);
}

// NOTE(chowie): This has better compability than converting floor positive to f64
internal f64
Floor(f64 F64)
{
    f64 Result = _mm_cvtsd_f64(_mm_floor_sd(_mm_setzero_pd(), _mm_set_sd(F64)));
    return(Result);
}

// NOTE(chowie): Floor for non-negative values, only [0 .. +2147483648) range.
internal f32
FloorPositive(f32 F32)
{
    Assert(SignOf(F32) == 1.0f);
    f32 Result = _mm_cvtss_f32(_mm_cvtepi32_ps(_mm_cvttps_epi32(_mm_set_ss(F32))));
    return(Result);
}

internal f64
FloorPositive(f64 F64)
{
    Assert(SignOf(F64) == 1.0f);
    f64 Result = _mm_cvtsd_f64(_mm_cvtepi64_pd(_mm_cvttpd_epi64(_mm_set_sd(F64))));
    return(Result);
}

// NOTE(chowie): Exact same as modf
inline f32
Fract(f32 F32)
{
    f32 Result = (F32 - Floor(F32));
    return(Result);
}

inline f32
FractPositive(f32 F32)
{
    f32 Result = (F32 - FloorPositive(F32));
    return(Result);
}

inline f64
FractPositive(f64 F64)
{
    f64 Result = (F64 - FloorPositive(F64));
    return(Result);
}

inline b32x
IsInteger(f32 F32)
{
    b32x Result = (Fract(F32) == 0.0f);
    return(Result);
}

inline b32x
IsIntegerPositive(f32 F32)
{
    b32x Result = (FractPositive(F32) == 0.0f);
    return(Result);
}

inline b32x
IsIntegerPositive(f64 F64)
{
    b32x Result = (FractPositive(F64) == 0.0f);
    return(Result);
}

inline s32
FloorF32ToS32(f32 F32)
{
    s32 Result = (s32)Floor(F32);
    return(Result);
}

inline u32
FloorF32ToU32(f32 F32)
{
    u32 Result = (u32)FloorPositive(F32);
    return(Result);
}

inline u16
FloorF32ToU16(f32 F32)
{
    u16 Result = (u16)FloorPositive(F32);
    return(Result);
}

inline u64
FloorF64ToU64(f64 F64)
{
    u64 Result = (u64)Floor(F64);
    return(Result);
}

inline f32
Ceil(f32 F32)
{
    f32 Result = _mm_cvtss_f32(_mm_ceil_ss(_mm_setzero_ps(), _mm_set_ss(F32)));
    return(Result);
}

// TODO(chowie): Don't think I need this!
inline s32
CeilF32ToS32(f32 F32)
{
    s32 Result = _mm_cvtss_si32(_mm_ceil_ss(_mm_setzero_ps(), _mm_set_ss(F32)));
    return(Result);
}

inline s32
TruncateF32ToS32(f32 F32)
{
    s32 Result = (s32)F32;
    return(Result);
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

#if COMPILER_MSVC
    Result.Found =  _BitScanForward((unsigned long *)&Result.Index, Value);
#else    
    for(u32 Test = 0;
        Test < 32;
        ++Test)
    {
        if(Value & (1 << Test))
        {
            Result.Index = Test;
            Result.Found = true;
            break;
        }
    }
#endif

    return(Result);
}

inline bit_scan_result
FindMostSignificantBit(u32 Value) // NOTE(chowie): clz
{
    bit_scan_result Result = {};

#if COMPILER_MSVC
    Result.Found =  _BitScanReverse((unsigned long *)&Result.Index, Value);
#else
    for(u32 Test = 32;
        Test > 32;
        --Test)
    {
        if(Value & (1 << (Test - 1)))
        {
            Result.Index = Test - 1;
            Result.Found = true;
            break;
        }
    }
#endif

    return(Result);
}

// RESOURCE(): FMA improvements - https://momentsingraphics.de/FMA.html
// NOTE(chowie): Requires AVX, otherwise use _mm_add and _mm_sub if unsure about the architecture
// f32 Result = fmaf(MultA, MultB, AddValue);
// NOTE(chowie): "A x B + C". A hidden benefit of fma is that it only rounds once (at the end).
// A * B is maintained with enough accuracy, by the time that C is added, it's much closer to the result of A * B + C.
// NOTE(chowie): DifferenceOfProducts multiplying by a power of 2 is exact to IEEE, Quadratic Formula = DifferenceOfProducts(b, b, 4 * a, c)
inline f32
Fma(f32 A, f32 B, f32 Add)
{
    f32 Result = _mm_cvtss_f32(_mm_fmadd_ss(_mm_set_ss(A), _mm_set_ss(B), _mm_set_ss(Add)));
    return(Result);
}

// RESOURCE(pharr): https://pharr.org/matt/blog/2019/11/03/difference-of-floats
// RESOURCE(njuffa): https://stackoverflow.com/questions/63665010/accurate-floating-point-computation-of-the-sum-and-difference-of-two-products
// RESOURCE(njuffa): https://stackoverflow.com/questions/49191477/companion-to-hypot/58511178#58511178
// NOTE(chowie): A*B - C*D for better accuracy; avoids catatrophic cancellation (high precision calculations without needing to convert to r64)
// TODO(chowie): Convert this to SIMD?
// TODO(chowie): Also use this for quadratic discriminant, 2x2 matrix etc...
inline f32
DifferenceOfProducts(f32 A, f32 B, f32 C, f32 D)
{
    f32 Mult = C*D;
    f32 Error = Fma(-C, D, Mult);
    f32 Difference = Fma(A, B, -Mult);

    f32 Result = Difference + Error;
    return(Result);
}

// NOTE(chowie): A*B + C*D
inline f32
SumOfProducts(f32 A, f32 B, f32 C, f32 D)
{
    f32 Mult = C*D;
    f32 Error = Fma(C, -D, Mult);
    f32 Difference = Fma(A, B, Mult);

    f32 Result = Difference - Error;
    return(Result);
}

//
// TODO(chowie): Convert all of these to platform-efficent versions and
// remove math.h
//

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

// TODO(chowie): Do I need 64-bit versions?
// RESOURCE(ankerl): http://martin.ankerl.com/2007/10/04/optimized-pow-approximation-for-java-and-c-c/
// RESOURCE(ekmett): https://github.com/ekmett/approximate/blob/master/cbits/fast.c
// NOTE(from ekmett): These can be _quite_ inaccurate. ~20% in many cases, but being much faster (~7x) may
// * permit more loop iterations of tuning algorithms that only need approximate powers.
// NOTE(chowie): Exp must be between -700 to 700
union rational_approx
{
    f32 f;
    s32 x;
};
inline f32
Exp(f32 Value)
{
    rational_approx Exp;
    Exp.x = (s32)(12102203*Value + 1064866805);
    return(Exp.f);
}

inline f32
Log(f32 Value)
{
    rational_approx Log = { Value };
    f32 Result = (Log.x - 1064866805)*8.262958405176314e-8f; /* 1 / 12102203.0; */
    return(Result);
}

// RESOURCE(): https://www.johndcook.com/blog/2025/06/24/log-ish/
// "I could see how this could be very handy. Often you want something
// like a logarithmic scale, not for the exact properties of the
// logarithm but because it brings big numbers closer in. And for big
// values of x there's little difference between log(x) vs log(1 + x).
//
// The function above is linear near the origin, literally linear for
// negative values and approximately linear for small positive values.
//
// I've occasionally needed something like a log scale, but one that
// would handle values that dip below zero. This transformation would
// be good for that. If data were equally far above and below zero,
// I'd use something like arctangent instead."
// STUDY(chowie): This is how you 'patch functions together' with a mediocre fit
// IMPORTANT(chowie): Beware these functions have a slight bump at the
// transition point, 0!
// f(x) = { log(1 + x) if x > 0
//        { x          if x <= 0
inline f32
Logish(f32 Value)
{
    f32 Result = Value;
    if(Value > 0)
    {
        Result = Log(1 + Value);
    }
    return(Result);
}

// TODO(chowie): Use this for smoothing + lerp for mountain gen not perlin
// noise or erosion https://www.youtube.com/watch?v=gsJHzBTPG0Y
// NOTE(chowie): This is more effective "1 + 1/(1 + x)"
inline f32
LogishPos(f32 Value)
{
    f32 Result = Log(1 + Value);
    return(Result);
}

// NOTE(chowie): This is basically "1 + 1/(1 + x)" if you only care about f32 0-1
inline f32
LogishPosCheap(f32 Value)
{
    f32 Result = Value - (Value*Value)/2;
    return(Result);
}

// NOTE(chowie): This is more effective "1/(1 + x)"
inline f32
InvLogishPos(f32 Value)
{
    f32 Result = 1 - Log(Value);
    return(Result);
}

// NOTE(chowie): This is basically "1/(1 + x)" if you only care about f32 0-1
inline f32
InvLogishPosCheap(f32 Value)
{
    f32 Result = 1 - Value + (Value*Value)/2;
    return(Result);
}

// RESOURCE(mineiro): https://web.archive.org/web/20150113003634/http://fastapprox.googlecode.com/svn/trunk/fastapprox/src/fastonebigheader.h
// TODO(chowie): Trig? Lambert?
inline f32
Lgamma(f32 Value)
{
    f32 Result = -0.0810614667f - Value - Log(Value) + (0.5f + Value)*Log(1.0f + Value);
    return(Result);
}

// RESOURCE(): https://www.johndcook.com/blog/tag/neural-networks/
// NOTE(chowie): Also called "logistic function" or "s-curve", similar to tanh(x*k)
inline f32
Sigmoid(f32 Value)
{
    f32 Result = 1/(1 + Exp(-Value));
    return(Result);
}

// NOTE(chowie): Comparable to C-standard pow. There was a "more
// precise" version but this was more precise for the circle. I assume
// because of FMA rounding at the end (instead of after each math
// op). See my notes of Fma instruction.
inline f32
Pow(f32 A, f32 B)
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

//
// Trig
//
// IMPORTANT(chowie): Uses turns!

// RESOURCE(ganssle): https://web.archive.org/web/20030429001611/http://www.ganssle.com/approx/approx.pdf
// RESOURCE(): https://github.com/divideconcept/FastTrigo/blob/master/fasttrigo.cpp
// TODO(chowie): Work on approximations

// RESOURCE(): https://handmade.network/forums/wip/t/1439-sse2_implementations_of_tan,_cot,_atan,_atan2
// TODO(chowie): Trig functions examples

// RESOURCE(fabien): https://fgiesen.wordpress.com/2010/10/21/finish-your-derivations-please/
// TODO(chowie): Replace with SSE2 instruction of sin?

// RESOURCE(): https://github.com/blat-blatnik/Snippets/blob/main/math.c
// NOTE(chowie): Thanks to blatnik, unlicence license for trig math
#define TURNS(deg) (deg)/360.0f

inline f32
F32FromBits(u32 x)
{
#if _MSC_VER
    return _castu32_f32(x);
#else
    union { u32 u; f32 f; } fu = { x };
    return fu.f;
#endif
}

inline u32
BitsFromF32(f32 x)
{
#if _MSC_VER
    return _castf32_u32(x);
#else
    union { f32 f; u32 u; } fu = { x };
    return fu.u;
#endif
}

internal v2
SinCos(f32 turns)
{
    // RESOURCE(): https://marc-b-reynolds.github.io/math/2020/03/11/SinCosPi.html
    // range reduction to [0,1/8]: sin/cos(x) = sin/cos(x + 2pi), sin/cos(x) = -sin/cos(x + pi) sin(x) = cos(x + pi/2)
    f32 range = Round(4*turns);
    u32 quadrant = (u32)(u64)range;
    f32 x = turns - 0.25f*range;
    f32 x2 = x*x;

    // set up range reconstruction
    u32 sign_x = (quadrant >> 1) << 31;
    u32 sign_y = (quadrant << 31) ^ sign_x;
    quadrant &= 1;

    // sollya> fpminimax(sin(2*pi*x), [|1,3,5,7|], [|24...|], [|0;1/8|], f32ing, relative);
    // max error = 5.382e-9
    f32 s = -75.83747100830078125f;
    s = s*x2 + 81.6046142578125f;
    s = s*x2 - 41.34175872802734375f;
    s = s*x2 + 6.283185482025146484375f;
    s = s*x;

    // sollya> fpminimax(cos(2*pi*x), [|0,2,4,6|], [|24...|], [|0;1/8|], f32ing, relative);
    // max error = 5.960e-8
    f32 c = -83.49729156494140625f;
    c = c*x2 + 64.9187469482421875f;
    c = c*x2 - 19.7391338348388671875f;
    c = c*x2 + 0.999999940395355224609375f;

    // reconstruct full range
    s = F32FromBits(BitsFromF32(s) ^ sign_y);
    c = F32FromBits(BitsFromF32(c) ^ sign_x);

    v2 Result = {s, c};
    return(Result);
}

// RESOURCE(nghia ho): https://nghiaho.com/?p=997
// COULDDO(chowie): Use the above?
//      float y = b_sin(turns);
//      float x = b_cos(turns);
//      float t = b_atan2(y, x);
internal f32
Atan2(f32 y, f32 x)
{
    // https://mazzo.li/posts/vectorized-atan2.html modified to correctly handle -0
    // range reduce to [0,1]: atan(x) = pi/2 - atan(1/x)
    s32 swap = Abs(x) < Abs(y);
    f32 num = swap ? x : y;
    f32 den = swap ? y : x;
    f32 yoverx = num / den;

    // range reduce to [0,1/4]: atan(x) = b + atan((x - k) / (1 + kx))
    // https://basesandframes.files.wordpress.com/2016/05/fast-math-functions_p2.pdf#page=35
    f32 abs = Abs(yoverx);
    f32 k = (abs < 0.5f) ? 0.25f : 0.75f;
    f32 b = (abs < 0.5f) ? 0.03898956518868466f : 0.10241638234956672f;
    f32 input = (abs - k) / (1 + k*abs);

    // rvaluate atan(x) polynomial in [0,1/4]
    // sollya> fpminimax(atan(x)/(2*pi), [|1,3,5,7|], [|24...|], [|1e-50;1/4|], f32ing, relative);
    // max error = 2.998e-10
    f32 input2 = input*input;
    f32 angle = -2.05062441527843475341796875e-2f;
    angle = angle*input2 + 3.17338518798351287841796875e-2f;
    angle = angle*input2 - 5.30500970780849456787109375e-2f;
    angle = angle*input2 + 0.15915493667125701904296875f;
    angle = angle*input;

    // reconstruct full range
    angle = CopySign(b + angle, yoverx);
    f32 unswap_angle = CopySign(0.25f, yoverx) - angle;
    angle = swap ? unswap_angle : angle;
    f32 quadrant_correction = CopySign(0.5f, y);
    angle += BitsFromF32(x) & 0x80000000 ? quadrant_correction : 0;
    return(angle);
}

internal f32
Exp2(f32 x)
{
    // range reduce: 2^x = 2^i*2^f, where i is the integer part of x and f is the fraction
    f32 i = Floor(x);
    f32 f = x - i;

    // compute 2^f via polynomial approximation
    // sollya> fpminimax(2^x, [|0,1,2,3,4,5,6|], [|24...|], [|0;1|], f32ing, relative);
    // max error = 4.293e-9
    f32 exp2f = 2.15564403333701193332672119140625e-4f;
    exp2f = exp2f*f + 1.248489017598330974578857421875e-3f;
    exp2f = exp2f*f + 9.67352092266082763671875e-3f;
    exp2f = exp2f*f + 5.54862879216670989990234375e-2f;
    exp2f = exp2f*f + 0.240229070186614990234375f;
    exp2f = exp2f*f + 0.69314706325531005859375f;
    exp2f = exp2f*f + 1.0f;

    // compute 2^i by directly loading i into the exponent
    s32 exponent = (s32)i + 127;
    exponent = exponent < 0 ? 0 : exponent;
    exponent = exponent > 255 ? 255 : exponent;
    f32 exp2i = F32FromBits(exponent << 23);

    return(exp2f*exp2i);
}

// RESOURCE(): https://stackoverflow.com/questions/9411823/fast-log2float-x-implementation-c
// COULDDO(chowie): Try using the above instead?
internal f32
Log2(f32 x)
{
    // range reduce: log2(x) = log2(2^e*1.f) = e + log2(1.f)
    u32 bits = BitsFromF32(x);
    f32 e = (f32)(bits >> 23) - 127;
    f32 f = F32FromBits((bits & 0x007FFFFF) | 0x3F800000); // set exponent to 0

    // sollya> fpminimax(1+log2(x+1), [|0,1,2,3,4,5,6|], [|24...|], [0;1]);
    // max error = 2.587e-6
    f -= 1;
    f32 log2f = -2.701638080179691314697265625e-2f;
    log2f = log2f*f + 0.12492744624614715576171875f;
    log2f = log2f*f - 0.2808862030506134033203125f;
    log2f = log2f*f + 0.4587285518646240234375f;
    log2f = log2f*f - 0.71829402446746826171875f;
    log2f = log2f*f + 1.44253671169281005859375f;
    log2f = log2f*f + 0.00000131130218505859375f; // -1 because this is 1+log2

    // reconstruct full range
    f32 log2 = e + log2f;
    return(e > 127 ? B_NAN : log2); // log(negative) = NaN
}

// TODO(chowie): Eventually remove "Turns" in naming of function
inline f32
Sin(f32 Turns)
{
    f32 Result = SinCos(Turns).x;
    return(Result);
}

inline f32
Cos(f32 Turns)
{
    f32 Result = SinCos(Turns).y;
    return(Result);
}

inline f32
Tan(f32 Turns)
{
    v2 Result = SinCos(Turns);
    return(Result.x / Result.y);
}

inline f32
AsinInternal(f32 y)
{
    f32 Result = Atan2(y, Sqrt(1 - y*y));
    return(Result);
}

inline f32
Asin(f32 Turns)
{
    f32 Result = AsinInternal(Cos(Turns));
    return(Result);
}

// TODO(chowie): Grep acosf!
inline f32
AcosInternal(f32 x)
{
    f32 Result = Atan2(Sqrt(1 - x*x), x);
    return(Result);
}

inline f32
Acos(f32 Turns)
{
    f32 Result = AcosInternal(Sin(Turns));
    return(Result);
}

inline f32
Atan(f32 yoverx)
{
    f32 Result = Atan2(yoverx, 1);
    return(Result);
}

inline f32
Sinh(f32 turns)
{
    f32 expx = Exp2(turns*9.064720283654387f); // log2(e)*tau
    f32 Result = (expx - 1 / expx)*0.5f;
    return(Result);
}

inline f32
Cosh(f32 turns)
{
    f32 expx = Exp2(turns*9.064720283654387f); // log2(e)*tau
    f32 Result = (expx + 1 / expx)*0.5f;
    return(Result);
}

inline f32
Tanh(f32 turns)
{
    f32 exp2x = Exp2(turns*18.129440567308773f); // 2*log2(e)*tau
    f32 Result = (exp2x - 1) / (exp2x + 1);
    return(Result);
}

inline f32
Asinh(f32 y)
{
    f32 Result = Log2(y + Sqrt(y*y + 1)) / 9.064720283654387f; // log2(e)*tau
    return(Result);
}

inline f32
Acosh(f32 x)
{
    f32 Result = Log2(x + Sqrt(x*x - 1)) / 9.064720283654387f; // log2(e)*tau
    return(Result);
}

inline f32
Atanh(f32 yoverx)
{
    f32 Result = Log2((1 + yoverx) / (1 - yoverx)) / 18.129440567308773f; // 2*log2(e)*tau
    return(Result);
}

inline f32
ExpExact(f32 x)
{
    f32 Result = Exp2(x*1.4426950408889634f); // log2(e)
    return(Result);
}

inline f32
LogExact(f32 x)
{
    f32 Result = Log2(x) / 1.4426950408889634f; // log2(e)
    return(Result);
}

inline f32
Log10Exact(f32 x)
{
    f32 Result = Log2(x) / 3.3219280948873626f; // log2(10)
    return(Result);
}

inline f32
PowExact(v2 Value)
{
    f32 Result = Exp2(Log2(Value.x)*Value.y);
    return(Result);
}

// RESOURCE: https://www.evanmiller.org/mathematical-hacker.html
// NOTE(chowie): Original article has lround
inline u32
Fibonacci(u32 Value)
{
    u32 Result = RoundF32ToU32((Pow(0.5f + 0.5f*Sqrt(5.0f), (f32)Value) - 
                                Pow(0.5f - 0.5f*Sqrt(5.0f), (f32)Value)) / 
                                Sqrt(5.0f));
    return(Result);
}

inline u32
Factorial(u32 Value)
{
    u32 Result = RoundF32ToU32(Exp(Lgamma((f32)Value + 1)));
    return(Result);
}

// #include <math.h>

/*
// RESOURCE: https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-fmod
inline f32
FMod(f32 X, f32 Y)
{
    f32 Result = fmodf(X, Y);
    return(Result);
}
*/

#define RUINENGLASS_INTRINSICS_H
#endif
