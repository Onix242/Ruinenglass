#if !defined(RUINENGLASS_RANDOM_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Michael Chow $
   $Notice: $
   ======================================================================== */

// TODO(chowie): Connect this!

struct random_series
{
    u32 Index;
};

inline random_series
RandomSeed(u32 Value)
{
    random_series Series;

    Series.Index = (Value); // NOTE(chowie): Required in -O2. Otherwise, the series is unitialised!

    return(Series);
}

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
    Series->Index = Result; // TODO(chowie): Is this how you are properly meant to interpret this?

    return(Series->Index);
}

inline u32
RandomChoice(random_series *Series, u32 ChoiceCount)
{
    u32 Result = (RandomXorshift(Series) % ChoiceCount);

    return(Result);
}

// TODO(chowie): How to properly do a unilateral?
// NOTE: Normal (0 to 1)
inline r32
RandomUnilateral(random_series *Series)
{
    r32 Divisor = 1.0f / (r32)Series.Index;
    // NOTE: Allow the compiler to pre-invert the number (a coefficient multiplication)
    r32 Result = Divisor * (r32)RandomXorshift(Series);

    return(Result);
}

// NOTE: Binormal (-1 to 1) 
inline r32
RandomBilateral(random_series *Series)
{
    r32 Result = 2.0f*RandomUnilateral(Series) - 1.0f;

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

#define RUINENGLASS_RANDOM_H
#endif
