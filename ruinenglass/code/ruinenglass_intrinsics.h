#if !defined(RUINENGLASS_INTRINSICS_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Michael Chow $
   $Notice: $
   ======================================================================== */

inline s32
SignOf(s32 Value)
{
    s32 Result = (Value >= 0) ? 1 : -1;
    return(Result);
}

inline r32
SignOf(r32 Value)
{
    __m128 SignMask = _mm_set1_ps(-0.0f); // NOTE(chowie): -0.0f = 1 << 31
    __m128 One = _mm_set_ss(1.0f);
    __m128 SignBit = _mm_and_ps(_mm_set_ss(Value), SignMask);
    __m128 Combined = _mm_or_ps(One, SignBit);

    r32 Result = _mm_cvtss_f32(Combined);
    return(Result);
}

// RESOURCE(wychmaster): https://stackoverflow.com/questions/57870896/writing-a-portable-sse-avx-version-of-stdcopysign
// RESOURCE(theowl84): Same technique using andnot to negate can be observed here - http://fastcpp.blogspot.com/2011/03/changing-sign-of-float-values-using-sse.html
inline r32
CopySign(r32 Sign, r32 Value)
{
    __m128 SignMask = _mm_set1_ps(-0.0f);
    __m128 ExtractSign = _mm_and_ps(_mm_set_ss(Sign), SignMask);
    __m128 ExtractValue = _mm_andnot_ps(SignMask, _mm_set_ss(Value)); // STUDY(chowie): Equivalent of AbsoluteValue(Value)

    r32 Result = _mm_cvtss_f32(_mm_or_ps(ExtractSign, ExtractValue));
    return(Result);
}

inline r32
SquareRoot(r32 R32)
{
    r32 Result = _mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(R32)));
    return(Result);
}

// RESOURCE(martins): https://handmade.network/forums/t/6940-understanding_basic_simd#21153
// RESOURCE(cbloom): http://cbloomrants.blogspot.com/2010/11/11-20-10-function-approximation-by_20.html
// NOTE(chowie): 1/sqrt(x), replaces normalising vectors
inline r32
ReciprocalSquareRoot(r32 R32)
{
    __m128 Half = _mm_set1_ps(0.5f);
    __m128 Three = _mm_set1_ps(3.0f);
    __m128 Value = _mm_set_ss(R32);
    __m128 Rsqrt = _mm_rsqrt_ss(Value);

    // NOTE(chowie): Newton's
    __m128 Temp = _mm_mul_ps(_mm_mul_ps(Value, Rsqrt), Rsqrt);
    Rsqrt = _mm_mul_ps(_mm_mul_ps(Half, Rsqrt), _mm_sub_ps(Three, Temp));

    r32 Result = _mm_cvtss_f32(Rsqrt);
    return(Result);
}

// RESOURCE(norbert): https://stackoverflow.com/questions/5508628/how-to-absolute-2-double-or-4-floats-using-sse-instruction-set-up-to-sse4
inline r32
AbsoluteValue(r32 R32)
{
    __m128 SignMask = _mm_set1_ps(-0.0f); // NOTE(chowie): -0.0f = 1 << 31
    __m128 ExtractValue = _mm_andnot_ps(SignMask, _mm_set_ss(R32));

    r32 Result = _mm_cvtss_f32(ExtractValue);
    return(Result);
}

// RESOURCE(brumme): https://bits.stephan-brumme.com/absInteger.html
inline s32
AbsoluteValue(s32 S32)
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
inline r32
Round(r32 R32)
{
    r32 Result = _mm_cvtss_f32(_mm_round_ss(_mm_setzero_ps(), _mm_set_ss(R32),
                                            (_MM_FROUND_TO_NEAREST_INT |_MM_FROUND_NO_EXC)));
    return(Result);
}

inline s32
RoundR32ToS32(r32 R32)
{
    s32 Result = _mm_cvtss_si32(_mm_set_ss(R32));
    return(Result);
}

inline u32
RoundR32ToU32(r32 R32)
{
    u32 Result = (u32)_mm_cvtss_si32(_mm_set_ss(R32));
    return(Result);
}

/*
// NOTE(chowie): SSE2 Floor - Thanks martins, Includes INF & NaN
// RESOURCE(martins): https://gist.github.com/mmozeiko/56db3df14ab380152d6875383d0f4afd
internal r32
MartinsFloor(r32 Value)
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
internal r32
Floor(r32 R32)
{
    r32 Result = _mm_cvtss_f32(_mm_floor_ss(_mm_setzero_ps(), _mm_set_ss(R32)));
    return(Result);
}

// NOTE(chowie): Floor for non-negative values, only [0 .. +2147483648) range.
internal r32
FloorPositive(r32 R32)
{
    Assert(SignOf(R32) == 1.0f);
    r32 Result = _mm_cvtss_f32(_mm_cvtepi32_ps(_mm_cvttps_epi32(_mm_set_ss(R32))));
    return(Result);
}

inline r32
Fract(r32 R32)
{
    r32 Result = (R32 - Floor(R32));
    return(Result);
}

inline s32
FloorR32ToS32(r32 R32)
{
    s32 Result = (s32)Floor(R32);
    return(Result);
}

inline u32
FloorR32ToU32(r32 R32)
{
    u32 Result = (u32)FloorPositive(R32);
    return(Result);
}

inline r32
Ceil(r32 R32)
{
    r32 Result = _mm_cvtss_f32(_mm_ceil_ss(_mm_setzero_ps(), _mm_set_ss(R32)));
    return(Result);
}

// TODO(chowie): Don't think I need this!
inline s32
CeilR32ToS32(r32 R32)
{
    s32 Result = _mm_cvtss_si32(_mm_ceil_ss(_mm_setzero_ps(), _mm_set_ss(R32)));
    return(Result);
}

inline s32
TruncateR32ToS32(r32 R32)
{
    s32 Result = (s32)R32;
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

// NOTE(chowie): Requires AVX, otherwise use _mm_add and _mm_sub if unsure about the architecture
// r32 Result = fmaf(MultA, MultB, AddValue);
// NOTE(chowie): "A x B + C". A hidden benefit of fma is that it only rounds once (at the end).
// A * B is maintained with enough accuracy, by the time that C is added, it's much closer to the result of A * B + C.
// NOTE(chowie): DifferenceOfProducts multiplying by a power of 2 is exact to IEEE, Quadratic Formula = DifferenceOfProducts(b, b, 4 * a, c)
inline r32
Fma(r32 A, r32 B, r32 Add)
{
    r32 Result = _mm_cvtss_f32(_mm_fmadd_ss(_mm_set_ss(A), _mm_set_ss(B), _mm_set_ss(Add)));
    return(Result);
}

// RESOURCE(pharr): https://pharr.org/matt/blog/2019/11/03/difference-of-floats
// RESOURCE(njuffa): https://stackoverflow.com/questions/63665010/accurate-floating-point-computation-of-the-sum-and-difference-of-two-products
// RESOURCE(njuffa): https://stackoverflow.com/questions/49191477/companion-to-hypot/58511178#58511178
// NOTE(chowie): A * B - C * D for better accuracy; avoids catatrophic cancellation (high precision calculations without needing to convert to r64)
// TODO(chowie): Convert this to SIMD?
// TODO(chowie): Also use this for quadratic discriminant, 2x2 matrix etc...
inline r32
DifferenceOfProducts(r32 A, r32 B, r32 C, r32 D)
{
    r32 Mult = C*D;
    r32 Error = Fma(-C, D, Mult);
    r32 Difference = Fma(A, B, -Mult);

    r32 Result = Difference + Error;
    return(Result);
}

// NOTE(chowie): A*B + C*D
inline r32
SumOfProducts(r32 A, r32 B, r32 C, r32 D)
{
    r32 Mult = C*D;
    r32 Error = Fma(C, -D, Mult);
    r32 Difference = Fma(A, B, Mult);

    r32 Result = Difference - Error;
    return(Result);
}

// RESOURCE(orlp): https://stackoverflow.com/questions/101439/the-most-efficient-way-to-implement-an-integer-based-power-function-powint-int
// RESOURCE(orlp): https://gist.github.com/orlp/3551590
// TODO(chowie): iPow?

//
// TODO(chowie): Convert all of these to platform-efficent versions and
// remove math.h
//

#include <math.h>
// RESOURCE(ganssle): https://web.archive.org/web/20030429001611/http://www.ganssle.com/approx/approx.pdf
// RESOURCE(): https://github.com/divideconcept/FastTrigo/blob/master/fasttrigo.cpp
// TODO(chowie): Work on approximations

// RESOURCE(fabien): https://fgiesen.wordpress.com/2010/10/21/finish-your-derivations-please/
// TODO(chowie): I don't want to be using angles, right? Hopefully I can replace everything with rational trig
// TODO(chowie): Replace with SSE2 instruction of sin?
inline r32
Sin(r32 Angle)
{
    r32 Result = sinf(Angle);
    return(Result);
}

inline r32
Cos(r32 Angle)
{
    r32 Result = cosf(Angle);
    return(Result);
}

// RESOURCE(nghia ho): https://nghiaho.com/?p=997
// TODO(chowie): Replace atan2?
inline r32
ATan2(r32 Y, r32 X)
{
    r32 Result = atan2f(Y, X);
    return(Result);
}

/*
// RESOURCE: https://learn.microsoft.com/en-us/windows/win32/direct3dhlsl/dx-graphics-hlsl-fmod
inline u32
FMod(u32 X, u32 Y)
{
    u32 Result = (u32)fmod(X, Y);
    return(Result);
}

inline r32
FMod(r32 X, r32 Y)
{
    r32 Result = fmodf(X, Y);
    return(Result);
}

// RESOURCE: https://www.evanmiller.org/mathematical-hacker.html
// TODO(chowie): Any more than can be replaced with intrinsics?
inline s32
Fibonacci(u32 Value)
{
    u32 Result = lround((pow(0.5f + 0.5f * SquareRoot(5.0), Value) - 
                         pow(0.5f - 0.5f * SquareRoot(5.0), Value)) / 
                        SquareRoot(5.0));
    return(Result);
}

inline s32
Factorial(u32 Value)
{
    u32 Result = lround(exp(lgamma(Value + 1)));
    return(Result);
}
*/

#define RUINENGLASS_INTRINSICS_H
#endif
