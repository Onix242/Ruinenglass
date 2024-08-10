#if !defined(RUINENGLASS_RANDOM_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Michael Chow $
   $Notice: $
   ======================================================================== */

struct random_series
{
    u32 Index;
};

// NOTE(chowie): Required in -O2. Otherwise, the series is uninitialised!
inline random_series
RandomSeed(u32 Value)
{
    random_series Series;
    Series.Index = (Value);

    return(Series);
}

// TODO(chowie): Better random with PCG?
// TODO(chowie): Note how easy it is to convert to SIMD, not so much if also wanting to rotate too
// RESOURCE(wikipedia): https://en.wikipedia.org/wiki/Xorshift
// NOTE(chowie): The state must be initialized to non-zero
internal u32
RandomXorshift(random_series *Series)
{
    // NOTE(chowie): Algorithm "xor" from p. 4 of Marsaglia, "Xorshift RNGs"
    u32 Result = Series->Index;

    Result ^= Result << 13;
    Result ^= Result >> 17;
    Result ^= Result << 5;

    Series->Index = Result;

    return(Result);
}

inline u32
RandomChoice(random_series *Series, u32 ChoiceCount)
{
    u32 Result = (RandomXorshift(Series) % ChoiceCount);
    return(Result);
}

// TODO(chowie): Check this for bias!
// NOTE: Normal (0 to 1)
// NOTE(chowie): Masked off lower bits
inline r32
RandomUnilateral(random_series *Series)
{
    r32 Result = (r32)((u32)(RandomXorshift(Series) >> 1) / ((u32)(U32Max >> 1)));
    return(Result);
}

// NOTE: Binormal (-1 to 1) 
inline r32
RandomBilateral(random_series *Series)
{
    r32 Result = 2.0f * RandomUnilateral(Series) - 1.0f;
    return(Result);
}

inline r32
RandomBetween(random_series *Series, v2 Range)
{
    r32 Result = Lerp(Range.Min, RandomUnilateral(Series), Range.Max);
    return(Result);
}

inline s32
RandomBetween(random_series *Series, v2s Range)
{
    // NOTE(chowie): We want to be one over the Max, we want to include
    // the case where 1 can produce a 0 or 1 instead of always getting
    // the min.
    s32 Result = Range.Min + (s32)(RandomXorshift(Series) % ((Range.Max + 1) - Range.Min));
    return(Result);
}

// RESOURCE: https://github.com/apple/swift/pull/39143
// NOTE(chowie): (0 to n) No divisions, branchless
// TODO(chowie): Test!
inline u32
RandomLemireUniform(random_series *Series, u32 Bounds)
{
    u64 A = (u64)Bounds * RandomXorshift(Series);
    u64 B = (u32)A + (((u64)Bounds * RandomXorshift(Series)) >> 32);
    u32 Result = (A >> 32) + (B >> 32);
    return(Result);
}

#define RUINENGLASS_RANDOM_H
#endif
