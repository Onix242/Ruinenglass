#if !defined(RUINENGLASS_RANDOM_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Michael Chow $
   $Notice: $
   ======================================================================== */

// STUDY(chowie): RNG used to use a LUT (Lookup Table), LUT are less
// favourable for RNG because it's a lot harder to fully SIMD.

// RESOURCE(): https://www.flipcode.com/archives/Random_Unit_Vectors.shtml
// TODO(chowie): Lot more useful than you would think to test
// reliability, give something a new direction, slerp between random
// orientations

// RESOURCE(o'neill): https://www.pcg-random.org/download.html
struct pcg32_random_series
{
    u64 State; // RNG state. All values are possible.
    u64 Inc; // Controls which RNG sequence (stream) is selected. Must *always* be odd.
};

internal u32
PCG32(pcg32_random_series *Series)
{
    u64 OldState = Series->State;
    Series->State = OldState*6364136223846793005ULL + Series->Inc;
    u32 Xorshifted = (u32)(((OldState >> 18u) ^ OldState) >> 27u);
    u32 Rot = OldState >> 59u;
    u32 Result = (Xorshifted >> Rot) | (Xorshifted << ((-(s32)Rot) & 31));
    return(Result);
}

internal void
RandomSeed(pcg32_random_series *Series, u64 InitState, u64 InitSeq)
{
    Series->State = 0U;
    Series->Inc = (InitSeq << 1u) | 1u;
    PCG32(Series);
    Series->State += InitState;
    PCG32(Series);
}

// NOTE(chowie) How to use PCG:
//    pcg32_random_series PCG;
//    RandomSeed(&PCG, 42u, 54u);
//    printf("Test RNG PCG: %u\n", PCG32(&PCG));
//    printf("Rand Bounds1: %u\n", RandomBounds(&PCG, 10));
//    printf("Rand Bounds2: %u\n", RandomBounds(&PCG, 10));

// RESOURCE(inigo quilez): https://iquilezles.org/articles/sfrand/
// STUDY(chowie): Passing in Series/Seed has benefits of
// multithreading, compared to C-standard rand(), one for each thread
// on the stack
struct xorshift32_random_series
{
    u32 Index;
};

// NOTE(chowie): Required in -O2. Otherwise, the series is uninitialised!
inline xorshift32_random_series
RandomSeed(u32 Value)
{
    xorshift32_random_series Series;
    Series.Index = (Value);

    return(Series);
}

internal u32
XorshiftStar32(xorshift32_random_series *Series)
{
    u32 Result = Series->Index;

    Result ^= Result << 13;
    Result ^= Result >> 17;
    Result ^= Result << 5;
    Result *= 0x9e02ad0d;

    return(Result);
}

// RESOURCE(graemephi): https://graemephi.github.io/posts/some-low-discrepancy-noise-functions/
// TODO(chowie): Use Variant has better statistical quality for the
// bottom-most 4 bits. Works for 2^16 (Similar to LCGs).
// TODO(chowie): Note how easy it is to convert to SIMD, not so much if also wanting to rotate too
// TODO(chowie): Noise?
// RESOURCE(wikipedia): https://en.wikipedia.org/wiki/Xorshift
// NOTE(chowie): The state must be initialized to non-zero
internal u32
Xorshift32(xorshift32_random_series *Series)
{
    u32 Result = Series->Index;

    Result ^= Result << 13;
    Result ^= Result >> 17;
    Result ^= Result << 5;

    return(Result);
}

internal u32
ReverseXorshift32(u32 XorHash)
{
    u32 Result = XorHash;

    Result ^= Result << 5;
    Result ^= Result << 10;
    Result ^= Result << 20;

    Result ^= Result >> 17;

    Result ^= Result << 13;
    Result ^= Result << 26;

    return(Result);
}

// RESOURCE: https://github.com/apple/swift/pull/39143
// NOTE(chowie): Lemire Uniform [0 to n). No divisions, branchless
internal u32
RandomBounds(pcg32_random_series *Series, u32 Bounds)
{
    u64 A = (u64)Bounds*PCG32(Series);
    u64 B = (u32)A + (((u64)Bounds*PCG32(Series)) >> 32);
    u32 Result = (A >> 32) + (B >> 32);
    return(Result);
}

// NOTE(chowie): [0, 1]
inline f32
RandomUnilateral(pcg32_random_series *Series)
{
    f32 Result = (f32)(PCG32(Series) >> 1) / (U32Max >> 1);
    return(Result);
}

// NOTE: Binormal (-1 to 1) 
inline f32
RandomBilateral(pcg32_random_series *Series)
{
    f32 Result = 2.0f*RandomUnilateral(Series) - 1.0f;
    return(Result);
}

inline f32
RandomBetween(pcg32_random_series *Series, v2 Range)
{
    f32 Result = Lerp(Range.Min, RandomUnilateral(Series), Range.Max);
    return(Result);
}

inline s32
RandomBetween(pcg32_random_series *Series, v2s Range)
{
    // NOTE(chowie): We want to be one over the Max, we want to include
    // the case where 1 can produce a 0 or 1 instead of always getting
    // the min.
    s32 Result = Range.Min + (s32)(PCG32(Series) % ((Range.Max + 1) - Range.Min));
    return(Result);
}

// RESOURCE(): Reversing hashes - https://www.youtube.com/watch?v=XDsYPXRCXAs
// x = (x * (x + 3 + 4 * a) + 4 * b + 2) % c
// is also pretty cool one I found, it looks like if c is a power of two,
// it loops through all even numbers, with odd numbers linking to the main
// loop so the total amount of unique numbers is (c + 1) / 2.
// A and B are integer constants (like 0 or 1) which change the way
// even numbers are linked together

// COULDDO(chowie): Might be useful to invert LCGs?

//
// NOTE(chowie): Initial Checking of multiples of two
//
// Pow2C = '4' is how much the next x will move and will always be a pair of numbers

// f(x, A, B, 4) Numbers that work, x = '2, 3, 6, 7, 10, 11, 14, 15'
// e.g. 2, 3 -> 6, 7 -> 10, 11 etc. A and B can be anything

// f(x, A, B, 8) Numbers that work, x = '3, 6, 11, 14, 19, 22'

// f(x, A, B, 16) Numbers that work, x = '3, 6, 19, 22'

// Pow2C = '16' is how much the next x will move and will always be a pair of numbers
// TODO(chowie): Check if number is greater!

// IMPORTANT(chowie): This is how you hack a pow2 modulo op
inline u32
LCGPossiblePhaseCancelling(u32 x, u32 A, u32 B, u32 Pow2C)
{
    u32 NextNumber = x;
    u32 LastNumber = A; // NOTE(chowie): Just changes the even/parity
    u32 NumberToFind = B; // NOTE(chowie): Just changes the even/parity
    u32 Combined = (x * (x + 3 + 4 * A) + 4 * B + 2);
    u32 Result = Combined % Pow2C;
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

// TODO(chowie): Use for Perlin noise!
// IMPORTANT(chowie): Length must be power-of-two
// NOTE(chowie): Mask = hashing domain < 2x size of array. Hash
// has 50% chance to flip the MostSignificantBit (hash to value
// 1/2 of lower half of domain). Takes < 2 iterations on average.
internal u32
Permute(xorshift32_random_series *Series, u32 Index, u32 Length) // TODO(chowie): Change to use PCG?
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

// TODO(chowie): Use this to make static starfields, the ones in the
// night sky!
// RESOURCE(): https://hero.handmade.network/forums/code-discussion/t/2799-samplehemisphere_is_not_uniform#13820
// RESOURCE(): https://hero.handmade.network/forums/code-discussion/t/419-periodic_functions
internal v3
SampleUniformlyHemisphere(pcg32_random_series *Series)
{
    v3 Result = {};

    f32 z = RandomBilateral(Series);
    f32 r = SquareRoot(1 - Square(z));
    f32 theta = RandomBilateral(Series)*Tau32;

    Result.x = r*Cos(theta);
    Result.y = r*Sin(theta);
    Result.z = z;

    return(Result);
}

// NOTE(from bromage): For light transport, add geometric cosine
// factor, sample unit disc, map to hemisphere sitting above
internal v3
CosineSampleNonuniformlyHemisphere(pcg32_random_series *Series)
{
    v3 Result = {};

    f32 r2 = RandomBilateral(Series);
    f32 r = SquareRoot(r2);
    f32 theta = RandomBilateral(Series)*Tau32;
    f32 z = SquareRoot(1 - r2);

    Result.x = r*Cos(theta);
    Result.y = r*Sin(theta);
    Result.z = z;

    return(Result);
}

// TODO(chowie): Random distribution of a triangle using barycentric
// with exponential distribution
// RESOURCE(): https://www.johndcook.com/blog/2025/09/11/random-inside-triangle/

#define RUINENGLASS_RANDOM_H
#endif
