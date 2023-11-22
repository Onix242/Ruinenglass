#if !defined(RUINENGLASS_INTRINSICS_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Talia ... & Michael Chow $
   $Notice: $
   ======================================================================== */

//
// TODO(chowie): Convert all of these to platorm-efficent versions and
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
SquareRoot(r32 R32)
{
    r32 Result = sqrtf(R32);
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

inline s32
FloorR32ToS32(r32 R32)
{
    s32 Result = (s32)floorf(R32);
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
    r32 Result = sinf(Angle);
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

struct bit_scan_result
{
    b32 Found;
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

#define RUINENGLASS_INTRINSICS_H
#endif
