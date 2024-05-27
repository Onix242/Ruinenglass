#if !defined(RUINENGLASS_INTRINSICS_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Michael Chow $
   $Notice: $
   ======================================================================== */

//
// TODO(chowie): Convert all of these to platform-efficent versions and
// remove math.h
//

#include <math.h>

inline s32
SignOf(s32 Value)
{
    s32 Result = (Value >= 0) ? 1 : -1;
    return(Result);
}

inline r32
SignOf(r32 Value)
{
    r32 Result = (Value >= 0.0f) ? 1.0f : -1.0f;
    return(Result);
}

inline r32
Fma(r32 MultA, r32 MultB, r32 AddValue)
{
    r32 Result = fmaf(MultA, MultB, AddValue);
    return(Result);
}

inline u32
Pow(u32 A, u32 B)
{
    u32 Result = (u32)pow(A, B);
    return(Result);
}

// RESOURCE(cbloom): http://cbloomrants.blogspot.com/2010/11/11-20-10-function-approximation-by_20.html
// NOTE(chowie): 1/sqrt(x), replaces normalising vectors
inline r32
InvSquareRoot(r32 R32)
{
    r32 Result = _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(R32)));
    return(Result);
}

// TODO(chowie): Backup? "r32 Result = sqrtf(R32);"
inline r32
SquareRoot(r32 R32)
{
    r32 Result = _mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(R32)));
    return(Result);
}

inline r32
AbsoluteValue(r32 R32)
{
    r32 Result = (r32)fabs(R32);
    return(Result);
}

inline u32
RotateLeft(u32 Value, s32 Amount)
{
#if COMPILER_MSVC
    u32 Result = _rotl(Value, Amount);
#else
    // TODO(chowie): Actually port to other compiler platforms
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
    // TODO(chowie): Actually port to other compiler platforms
    Amount &= 31;
    u32 Result = ((Value >> Amount) | (Value << (32 - Amount)));
#endif

    return(Result);
}

inline s32
RoundR32ToS32(r32 R32)
{
    s32 Result = (s32)roundf(R32);
    return(Result);
}

inline u32
RoundR32ToU32(r32 R32)
{
    u32 Result = (u32)roundf(R32);
    return(Result);
}

// NOTE(chowie): Thanks martins, Includes INF & NaN
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

// NOTE(chowie): Floor for non-negative values, only [0 .. +2147483648) range.
internal r32
MartinsFloorPositive(r32 Value)
{
    __m128 Float = _mm_set_ss(Value);
    __m128 Result = _mm_cvtepi32_ps(_mm_cvttps_epi32(Float));

    return(_mm_cvtss_f32(Result));
}

/*
// TODO(chowie): Sse4?
internal r32
Floor2(r32 Value)
{
    __m128 Float = _mm_set_ss(Value);
    __m128 Result = _mm_floor_ss(Float, Float);

    return(_mm_cvtss_f32(Result));
}
*/

inline s32
FloorR32ToS32(r32 R32)
{
    s32 Result = (s32)MartinsFloor(R32);
    return(Result);
}

inline s32
CeilR32ToS32(r32 R32)
{
    s32 Result = (s32)ceilf(R32);
    return(Result);
}

inline s32
TruncateR32ToS32(r32 R32)
{
    s32 Result = (s32)R32;
    return(Result);
}

inline r32
Sin(r32 Angle)
{
    r32 Result = sinf(Angle); // TODO(chowie): Replace with SSE2 instruction of sin?
    return(Result);
}

inline r32
Cos(r32 Angle)
{
    r32 Result = cosf(Angle);
    return(Result);
}

inline r32
ATan2(r32 Y, r32 X)
{
    r32 Result = atan2f(Y, X);
    return(Result);
}

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

struct bit_scan_result
{
    b32x Found;
    u32 Index;
};
inline bit_scan_result
FindLeastSignificantSetBit(u32 Value)
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

#define RUINENGLASS_INTRINSICS_H
#endif
