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

// RESOURCE(chowie): https://en.wikipedia.org/wiki/Xorshift
// NOTE(wikipedia): The state must be initialized to non-zero
internal u32
RandomXorshift(random_series *Series)
{
    // NOTE(wikipedia): Algorithm "xor" from p. 4 of Marsaglia, "Xorshift RNGs"
    u32 Result = Series->Index;

    Result ^= Result << 13;
    Result ^= Result >> 17;
    Result ^= Result << 5;
    Series->Index = Result; // TODO(chowie): Is this how you are properly meant to interpret this?

    return(Series->Index);
}

inline s32
RandomBetween(random_series *Series, s32 Min, s32 Max)
{
    // NOTE(chowie): We want to be one over the Max, we want to include
    // the case where 1 can produce a 0 or 1 instead of always getting
    // the min.
    s32 Result = Min + (s32)(RandomXorshift(Series) %((Max + 1) - Min));

    return(Result);
}

#define RUINENGLASS_RANDOM_H
#endif
