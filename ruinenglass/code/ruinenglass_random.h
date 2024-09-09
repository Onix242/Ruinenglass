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
    u32 Result = Series->Index;

    Result ^= Result << 13;
    Result ^= Result >> 17;
    Result ^= Result << 5;

    Series->Index = Result;

    return(Result);
}

// RESOURCE: https://github.com/apple/swift/pull/39143
// NOTE(chowie): Lemire Uniform (0 to n). No divisions, branchless
inline u32
RandomBounds(random_series *Series, u32 Bounds)
{
    u64 A = (u64)Bounds*RandomXorshift(Series);
    u64 B = (u32)A + (((u64)Bounds*RandomXorshift(Series)) >> 32);
    u32 Result = (A >> 32) + (B >> 32);
    return(Result);
}

/*
inline u32
RandomChoice(random_series *Series, u32 ChoiceCount)
{
    u32 Result = (RandomXorshift(Series) % ChoiceCount);
    return(Result);
}
*/

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

/*
  RESOURCE(andrew helmer): https://andrew-helmer.github.io/permute/
  NOTE(chowie): Andrew Kesler's Permute, randomly shuffles (0 to n)

  Works up to 2^27. For larger domains, call function twice.

  Linear access, O(k) time and O(1) memory, the best you could possibly
  do. Can arbitrarily query random indices in constant time, thus can
  be used in parallel across multiple threads or computers.

  1) The rendering case, which is what motivated Kensler. When you're
  iterating over lots of arrays simultaneously (10s of thousands or
  millions), or iterating over one array in a million different
  random orders. In other words, rather than fully iterating over one
  array, then the next array, etc. you want to get the i'th shuffled
  value of a million arrays, then get the i+1'th shuffled value of
  those arrays, etc.
  
  2) Where you want a "small" random sample (without replacement,
  i.e. no duplicates) from a huge array - think 10,000 unique.
  
  3) Compute something - like an average - over a large random sample
  from an even huger array, neither of which can fit in memory, and
  important that there are no duplicate values.

  Fisher-Yates - an implace shuffle, doesn't work well when you must
  store the shuffled array. Can use Floyd's or incremental Fisher-Yates
  for 2) for good randomness.

  Sample Usage - 10 random values from a vector
  uint32_t seed = uniform_rand_int(0, UINT_MAX);
  for(int i = 0;
      i < 10;
      i++)
  {
      int rand_idx = permute(i, array.size(), seed);
      ...  // Do something with array[rand_idx].
  }
*/

// TODO(chowie): Better max mask!
inline u32
FindMostSignificantBitFullMask(u32 Length)
{
    u32 Result = 0;
    if(Length != 1)
    {
        u32 FullMask = 1;
        Result = (FullMask >> FindMostSignificantBit(Length - 1).Index);
    }

    return(Result);
}

// STUDY(chowie): Hill-climbing invertible hash for power-of-two, bijective function
// NOTE(chowie): Invertible hash is usually with an odd number.
inline u32
InvertibleHash(u32 Seed, u32 Index, u32 Mask)
{
    u32 Result = Index;

    // NOTE(chowie): Fancier version of "Index ^= Seed; (Index * 3) & Mask;"
    Index ^= Seed;
    Index *= 0xe170893d;
    Index ^= Seed >> 16;
    Index ^= (Index & Mask) >> 4;
    Index ^= Seed >> 8;
    Index *= 0x0929eb3f;
    Index ^= Seed >> 23;
    Index ^= (Index & Mask) >> 1;
    Index *= 1 | Seed >> 27;
    Index *= 0x6935fa69;
    Index ^= (Index & Mask) >> 11;
    Index *= 0x74dcb303;
    Index ^= (Index & Mask) >> 2;
    Index *= 0x9e501cc3;
    Index ^= (Index & Mask) >> 2;
    Index *= 0xc860a3df;
    Index &= Mask;
    Index ^= Index >> 5;

    return(Result);
}

// IMPORTANT(chowie): Length must be power-of-two
// NOTE(chowie): Mask = hashing domain < 2x size of array. Hash
// has 50% chance to flip the MostSignificantBit (hash to value
// 1/2 of lower half of domain). Takes < 2 iterations on average.
internal u32
Permute(random_series *Series, u32 Index, u32 Length)
{
    u32 Seed = Series->Index;
    u32 Result = 0;

    u32 Mask = FindMostSignificantBitFullMask(Length);

    // STUDY(chowie): Cycle walking = repeat hash until < array length.
    // STUDY(chowie): "idx -> hash(idx) -> hash^2(idx) -> hash^3(idx)"
    // Inverse of hash of say "hash^3(idx)" becomes "hash^2(idx)".
    // Thus the loop is an invertible hash too, instead of mapping to
    // power-of-two sized domain, maps within domain of hash.
    do
    {
        Result = InvertibleHash(Seed, Index, Mask);
    } while(Index >= Length);

    // NOTE(chowie): Optional extra randomness
    Result = (Index + Seed) % Length;
    return(Result);
}

#define RUINENGLASS_RANDOM_H
#endif
