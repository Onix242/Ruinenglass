#if !defined(RUINENGLASS_MATH_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Michael Chow $
   $Notice: $
   ======================================================================== */

inline v2u
V2U(u32 X, u32 Y)
{
    v2u Result = {X, Y};

    return(Result);
}

inline v2s
V2S(s32 X, s32 Y)
{
    v2s Result = {X, Y};

    return(Result);
}

// NOTE(chowie): I want to say, Min = "Some Value" for both.
inline v2s
V2S(s32 Value)
{
    v2s Result = {Value, Value};

    return(Result);
}

// NOTE(chowie): V2 from ints saves on casting all the time
inline v2
V2i(s32 X, s32 Y)
{
    v2 Result = {(r32)X, (r32)Y};

    return(Result);
}

inline v2
V2i(u32 X, u32 Y)
{
    v2 Result = {(r32)X, (r32)Y};

    return(Result);
}

inline v2
V2(r32 X, r32 Y)
{
    v2 Result = {X, Y};

    return(Result);
}

// NOTE(chowie): I want to say, Min = "Some Value" for both.
inline v2
V2(r32 Value)
{
    v2 Result = {Value, Value};

    return(Result);
}

inline v3u
V3U(u32 X, u32 Y, u32 Z)
{
    v3u Result = {X, Y, Z};

    return(Result);
}

inline v3s
V3S(s32 X, s32 Y, s32 Z)
{
    v3s Result = {X, Y, Z};

    return(Result);
}

inline v3
V3(r32 X, r32 Y, r32 Z)
{
    v3 Result = {X, Y, Z};

    return(Result);
}

inline v3
V3(v2 XY, r32 Z)
{
    v3 Result = {XY.x, XY.y, Z};

    return(Result);
}

inline v4
V4(r32 X, r32 Y, r32 Z, r32 W)
{
    v4 Result = {X, Y, Z, W};

    return(Result);
}

inline v4
V4(v3 XYZ, r32 W)
{
    v4 Result = {XYZ, W};

    return(Result);
}

// NOTE(chowie): Use SafeRatio where you are less concerned about
// performance and more concerned about robustness
inline r32
SafeRatioN(r32 Numerator, r32 Divisor, r32 N)
{
    r32 Result = N;

    if(Divisor != 0.0f)
    {
        Result = Numerator / Divisor;
    }

    return(Result);
}

inline r32
SafeRatio0(r32 Numerator, r32 Divisor)
{
    r32 Result = SafeRatioN(Numerator, Divisor, 0.0f);

    return(Result);
}

inline r32
SafeRatio1(r32 Numerator, r32 Divisor)
{
    r32 Result = SafeRatioN(Numerator, Divisor, 1.0f);

    return(Result);
}

//
// NOTE(chowie): Scalar operations
//

// NOTE(chowie): Squaring functions are nice for whole expressions
inline r32
Square(r32 A)
{
#define SQUARE(A) ((A)*(A))
    r32 Result = SQUARE(A);

    return(Result);
}

inline r32
Cube(r32 A)
{
#define CUBE(A) ((A)*(A)*(A))
    r32 Result = CUBE(A);

    return(Result);
}

// RESOURCE(fabian): https://fgiesen.wordpress.com/2012/08/15/linear-interpolation-past-present-and-future/
// NOTE(chowie): Can be also represented as "A + T*(B - A)",
// or "(1.0f - t)*A + t*B"
inline r32
Lerp(r32 A, r32 t, r32 B)
{
    r32 Result = Fma(t, B - A, A);

    return(Result);
}

inline r32
Clamp(r32 Min, r32 Value, r32 Max)
{
    r32 Result = Value;

    if(Result < Min)
    {
        Result = Min;
    }
    else if(Result > Max)
    {
        Result = Max;
    }

    return(Result);
}

// RESOURCE: https://developer.download.nvidia.com/cg/saturate.html
// NOTE(chowie): This is Saturate
inline r32
Clamp01(r32 Value)
{
    r32 Result = Clamp(0.0f, Value, 1.0f);

    return(Result);
}

// NOTE(chowie): A barycentricafier
inline r32
Clamp01MapToRange(r32 Min, r32 t, r32 Max)
{
    r32 Result = 0.0f;

    r32 Range = Max - Min;
    if(Range != 0.0f)
    {
        // STUDY(chowie): Normalises t bases around min, produce between 0 to
        // range, divide by range changes to 0 - 1
        Result = Clamp01((t - Min) / Range);
    }

    return(Result);
}

// RESOURCE: https://handmade.network/p/64/geometer/blog/p/3048-1_year_of_geometer_-_lessons_learnt
inline r32
SmoothStep(r32 t)
{
    r32 Result = Square(t) - (2.0f*Cube(t)); /* 3t^2 - 2t^3 */

    return(Result);
}

// RESOURCE: https://realtimecollisiondetection.net/blog/?p=95
// RESOURCE: https://www.fundza.com/rman_shaders/smoothstep/
// TODO(chowie): Test smooth-step and pulse for alpha tests?
inline r32
SmoothStep(r32 Min, r32 t, r32 Max)
{
    r32 Result = Clamp01MapToRange(Min, t, Max);
    Result = Square(Result) * (3.0f - 2.0f*Result);

    return(Result);
}

inline r32
Step(r32 t, r32 Value)
{
    r32 Result = (r32)(Value >= t);
    return(Result);
}

inline r32
SquarePulse(r32 A, r32 B, r32 Value)
{
    r32 Result = Step(A, Value) - Step(B, Value);
    return(Result);
}

inline r32
TrianglePulse(v2 A, r32 t, v2 B)
{
    r32 Result = Clamp01(A.x*t + B.x) - Clamp01(A.y*t + B.y);
    return(Result);
}

//
// NOTE(chowie): v2u / v2s operations
//

internal v2u
operator+(v2u A, v2u B)
{
    v2u Result =
    {
        A.x + B.x,
        A.y + B.y,
    };

    return(Result);
}

internal v2s
operator+(v2s A, v2s B)
{
    v2s Result =
    {
        A.x + B.x,
        A.y + B.y,
    };

    return(Result);
}

inline v2s
operator*(s32 A, v2s B)
{
    v2s Result =
    {
        A * B.x,
        A * B.y,
    };

    return(Result);
}

internal v2s
operator-(v2s A, v2s B)
{
    v2s Result = 
    {
        A.x - B.x,
        A.y - B.y,
    };

    return(Result);
}

inline v2s &
operator+=(v2s &A, v2s B)
{
    A = A + B;

    return(A);
}

inline v2s
Hadamard(v2s A, v2s B)
{
    v2s Result = {A.x*B.x, A.y*B.y};

    return(Result);
}

internal b32x
AreEqual(v2s A, v2s B)
{
    b32x Result = ((A.x == B.x) &&
                   (A.y == B.y));
    return Result;
}

internal b32x
AreEqual(v2u A, v2u B)
{
    b32x Result = ((A.x == B.x) &&
                   (A.y == B.y));

    return(Result);
}

//
// NOTE(chowie): v2 operations
//

inline v2
Perp(v2 A)
{
    v2 Result = {-A.y, A.x};

    return(Result);
}

// NOTE(chowie): A single multiplier
inline v2
operator*(r32 A, v2 B)
{
    v2 Result =
    {
        A * B.x,
        A * B.y,
    };

    return(Result);
}

// NOTE(chowie): 5*Vector or Vector*5 is valid syntax
inline v2
operator*(v2 B, r32 A)
{
    v2 Result = A*B;

    return(Result);
}

// NOTE(chowie): Calls operator*
inline v2 &
operator*=(v2 &B, r32 A)
{
    B = A * B;

    return(B);
}

// NOTE(chowie): B = -A is valid syntax
inline v2
operator-(v2 A)
{
    v2 Result =
    {
        -A.x,
        -A.y,
    };

    return(Result);
}

// NOTE(chowie): A + B is valid syntax
inline v2
operator+(v2 A, v2 B)
{
    v2 Result =
    {
        A.x + B.x,
        A.y + B.y,
    };

    return(Result);
}

// NOTE(chowie): Calls the + operator on itself
inline v2 &
operator+=(v2 &A, v2 B)
{
    A = A + B;

    return(A);
}

// NOTE(chowie): A - B is valid syntax
inline v2
operator-(v2 A, v2 B)
{
    v2 Result =
    {
        A.x - B.x,
        A.y - B.y,
    };

    return(Result);
}

// NOTE(chowie): Calls the - operator on itself
inline v2 &
operator-=(v2 &A, v2 B)
{
    A = A - B;

    return(A);
}

// STUDY(chowie): Difference between hadamard and inner product is
// that the terms are not summed
inline v2
Hadamard(v2 A, v2 B)
{
    v2 Result = {A.x*B.x, A.y*B.y};

    return(Result);
}

// NOTE(chowie): Also called Dot Product
// RESOURCE: https://hero.handmade.network/forums/code-discussion/t/7858-help_with_collision_detection_with_rotated_rectangles
/* The dot product of two vectors has the resulting scalar tell you if
   the two vectors point are in the same general direction: if the
   value is positive they are pointing in the same general direction
   if the value is negative they are pointing in opposite general
   direction if the value is 0 they are perpendicular.
*/
inline r32
Inner(v2 A, v2 B)
{
    r32 Result = A.x*B.x + A.y*B.y;

    return(Result);
}

// NOTE(chowie): Also called Magnitude
inline r32
LengthSq(v2 A)
{
    r32 Result = Inner(A, A);
    return(Result);
}

// STUDY(chowie): Sqroot is undefined negative numbers. LengthSq
// cannot produce a negative number, i.e Negative*Negative is positive
inline r32
Length(v2 A)
{
    r32 Result = SquareRoot(LengthSq(A));
    return(Result);
}

inline v2
Clamp01(v2 Value)
{
    v2 Result =
    {
        Clamp01(Value.x),
        Clamp01(Value.y),
    };

    return(Result);
}

inline v2
Lerp(v2 A, r32 t, v2 B)
{
    v2 Result =
    {
        Fma(t, B.x - A.x, A.x),
        Fma(t, B.y - A.y, A.y),
    };

    return(Result);
}

// NOTE(chowie): For radial
inline v2
Arm2(r32 Angle)
{
    v2 Result = {Cos(Angle), Sin(Angle)};

    return(Result);
}

//
// NOTE(chowie): v3u / v3s operations
//

internal v3u
operator+(v3u A, v3u B)
{
    v3u Result =
    {
        A.x + B.x,
        A.y + B.y,
        A.z + B.z,
    };

    return(Result);
}

internal b32x
AreEqual(v3u A, v3u B)
{
    b32x Result = ((A.x == B.x) &&
                   (A.y == B.y) &&
                   (A.z == B.z));

    return(Result);
}

internal v3s
operator+(v3s A, v3s B)
{
    v3s Result =
    {
        A.x + B.x,
        A.y + B.y,
        A.z + B.z,
    };

    return(Result);
}

internal b32x
AreEqual(v3s A, v3s B)
{
    b32x Result = ((A.x == B.x) &&
                   (A.y == B.y) &&
                   (A.z == B.z));

    return(Result);
}

//
// NOTE(chowie): v3 operations
//

// NOTE(chowie): A single multiplier
inline v3
operator*(r32 A, v3 B)
{
    v3 Result =
    {
        A * B.x,
        A * B.y,
        A * B.z,
    };

    return(Result);
}

// NOTE(chowie): 5*Vector or Vector*5 is valid syntax
inline v3
operator*(v3 B, r32 A)
{
    v3 Result = A * B;

    return(Result);
}

// NOTE(chowie): Calls operator*
inline v3 &
operator*=(v3 &B, r32 A)
{
    B = A * B;

    return(B);
}

// NOTE(chowie): B = -A is valid syntax
inline v3
operator-(v3 A)
{
    v3 Result =
    {
        -A.x,
        -A.y,
        -A.z,
    };

    return(Result);
}

// NOTE(chowie): A + B is valid syntax
inline v3
operator+(v3 A, v3 B)
{
    v3 Result =
    {
        A.x + B.x,
        A.y + B.y,
        A.z + B.z,
    };

    return(Result);
}

// NOTE(chowie): Calls the + operator on itself
inline v3 &
operator+=(v3 &A, v3 B)
{
    A = A + B;

    return(A);
}

// NOTE(chowie): A - B is valid syntax
inline v3
operator-(v3 A, v3 B)
{
    v3 Result =
    {
        A.x - B.x,
        A.y - B.y,
        A.z - B.z,
    };

    return(Result);
}

// NOTE(chowie): Calls the - operator on itself
inline v3 &
operator-=(v3 &A, v3 B)
{
    A = A - B;

    return(A);
}

// STUDY(chowie): Difference between hadamard and inner product is
// that the terms are not summed
inline v3
Hadamard(v3 A, v3 B)
{
    v3 Result = {A.x*B.x, A.y*B.y, A.z*B.z};

    return(Result);
}

inline r32
Inner(v3 A, v3 B)
{
    r32 Result = A.x*B.x + A.y*B.y + A.z*B.z;

    return(Result);
}

inline v3
Cross(v3 A, v3 B)
{
    v3 Result =
    {
        A.y*B.z - A.z*B.y,
        A.z*B.x - A.x*B.z,
        A.x*B.y - A.y*B.x,
    };

    return(Result);
}

inline r32
LengthSq(v3 A)
{
    r32 Result = Inner(A, A);
    return(Result);
}

// TODO(chowie): Am I able to remove this now that I have InvSquareRoot?
// STUDY(chowie): Sqrt is undefined negative numbers. LengthSq
// cannot produce a negative number, i.e Negative*Negative is positive
inline r32
Length(v3 A)
{
    r32 Result = SquareRoot(LengthSq(A));
    return(Result);
}

// RESOURCE(fabien): https://fgiesen.wordpress.com/2010/10/21/finish-your-derivations-please/
// TODO(chowie): I don't want to be using angles, right?
// RESOURCE(owl): https://fastcpp.blogspot.com/2012/02/calculating-length-of-3d-vector-using.html
// TODO(chowie): SSE?
inline v3
Normalise(v3 A)
{
    r32 LenSq = LengthSq(A);
    v3 Result = A * InvSquareRoot(LenSq);

    return(Result);
}

// NOTE(chowie): Normalise by 0
inline v3
NOZ(v3 A)
{
    v3 Result = {};

    r32 LenSq = LengthSq(A);
    if(LenSq > Square(0.0001f))
    {
        Result = A * InvSquareRoot(LenSq);
    }

    return(Result);
}

// RESOURCE(juckett): https://www.ryanjuckett.com/biarc-interpolation/
// TODO: Try biarc interpolation?
// TODO(chowie): Not quite sure how useful this function is - for asserts mainly
// NOTE(chowie): Vector length is within epsilon of 1
inline b32x
IsNormalisedEps(v3 A, r32 Epsilon)
{
    r32 Magnitude = LengthSq(A);
    b32x Result = (Magnitude >= Square(1.0f - Epsilon) &&
                  Magnitude <= Square(1.0f + Epsilon));
    return(Result);
}

// TODO(chowie): For bases? Camera projections?
// RESOURCE(sam): https://web.archive.org/web/20210725181210/https://lolengine.net/blog/2013/9
// NOTE(chowie): "Given a non-zero vector v in 3D space, find a
// non-zero vector w orthogonal to v (i.e. such that v.w = 0)."
// = Always works if the input is non-zero.
// = Doesn’t require the input to be normalised.
// = Doesn’t normalise the output.
inline v3
GeneratePerp(v3 A)
{
    v3 Result = (AbsoluteValue(A.x) > AbsoluteValue(A.z))
        ? V3(-A.y, A.x, 0.0) : V3(0.0, -A.z, A.y);
    return(Result);
}

// RESOURCE(gaul): https://web.archive.org/web/20200414122444/https://www.randygaul.net/2014/11/
// RESOURCE(gaul): https://www.reddit.com/r/gamedev/comments/2ljvlz/collection_of_numerically_robust_parallel_vectors/
// TODO(chowie): Double Check this!
inline b32x
IsPerp(v3 A, v3 B, r32 Epsilon)
{
    b32x Result = false;
    if(AbsoluteValue(Inner(A, B)) < Epsilon)
    {
        Result = true;
    }

    return(Result);
}

// TODO(chowie): Double Check this!
// NOTE(chowie): Pros; No non-linearity issues, fast with 2
// sqrts. Cons; Epsilons with v3 of wildly different components, bias
// to scale to the shorter or longer version - shorter?
inline b32x
IsParallel(v3 A, v3 B, r32 Epsilon)
{
    b32x Result;

    A = Normalise(A);
    B = Normalise(B);
    Result = ((1.0f - AbsoluteValue(Inner(A, B))) < Epsilon);

    return(Result);
}

inline v3
Clamp01(v3 Value)
{
    v3 Result =
    {
        Clamp01(Value.x),
        Clamp01(Value.y),
        Clamp01(Value.z),
    };

    return(Result);
}

inline v3
Lerp(v3 A, r32 t, v3 B)
{
    v3 Result =
    {
        Fma(t, B.x - A.x, A.x),
        Fma(t, B.y - A.y, A.y),
        Fma(t, B.z - A.z, A.z),
    };

    return(Result);
}

// RESOURCE: https://box2d.org/posts/2014/02/computing-a-basis/
// RESOURCE: https://web.archive.org/web/20190120215637/http://www.randygaul.net/2015/10/27/compute-basis-with-simd/
// TODO(chowie): Find out if this can be a replacement for entities? If so, SIMD this?
inline void
GetBasis(v3 A, v3 *B, v3 *C)
{
    // Suppose vector a has all equal components and is a unit vector:
    // a = (s, s, s)
    // Then 3*s*s = 1, s = sqrt(1/3) = 0.57735. This means that at
    // least one component of a unit vector must be greater or equal
    // to 0.57735.

#define SqrtThird 0.57735f
    if(AbsoluteValue(A.x) >= SqrtThird)
    {
        *B = V3(A.y, -A.x, 0.0f);
    }
    else
    {
        *B = V3(0.0f, A.z, -A.y);
     }

    *B = Normalise(*B);
    *C = Cross(A, *B);
}

//
// NOTE(chowie): v4 operations
//

// NOTE(chowie): A single multiplier
inline v4
operator*(r32 A, v4 B)
{
    v4 Result =
    {
        A * B.x,
        A * B.y,
        A * B.z,
        A * B.w,
    };

    return(Result);
}

// NOTE(chowie): 5*Vector or Vector*5 is valid syntax
inline v4
operator*(v4 B, r32 A)
{
    v4 Result = A*B;

    return(Result);
}

// NOTE(chowie): Calls operator*
inline v4 &
operator*=(v4 &B, r32 A)
{
    B = A * B;

    return(B);
}

// NOTE(chowie): B = -A is valid syntax
inline v4
operator-(v4 A)
{
    v4 Result =
    {
        -A.x,
        -A.y,
        -A.z,
        -A.w,
    };

    return(Result);
}

// NOTE(chowie): A + B is valid syntax
inline v4
operator+(v4 A, v4 B)
{
    v4 Result =
    {
        A.x + B.x,
        A.y + B.y,
        A.z + B.z,
        A.w + B.w,
    };

    return(Result);
}

// NOTE(chowie): Calls the + operator on itself
inline v4 &
operator+=(v4 &A, v4 B)
{
    A = A + B;

    return(A);
}

// NOTE(chowie): A - B is valid syntax
inline v4
operator-(v4 A, v4 B)
{
    v4 Result =
    {
        A.x - B.x,
        A.y - B.y,
        A.z - B.z,
        A.w - B.w,
    };

    return(Result);
}

// NOTE(chowie): Calls the - operator on itself
inline v4 &
operator-=(v4 &A, v4 B)
{
    A = A - B;

    return(A);
}

// STUDY(chowie): Difference between hadamard and inner product is
// that the terms are not summed
inline v4
Hadamard(v4 A, v4 B)
{
    v4 Result = {A.x*B.x, A.y*B.y, A.z*B.z, A.w*B.w};

    return(Result);
}

inline r32
Inner(v4 A, v4 B)
{
    r32 Result = A.x*B.x + A.y*B.y + A.z*B.z + A.w*B.w;

    return(Result);
}

inline r32
LengthSq(v4 A)
{
    r32 Result = Inner(A, A);
    return(Result);
}

// STUDY(chowie): Sqroot is undefined negative numbers. LengthSq
// cannot produce a negative number, i.e Negative*Negative is positive
inline r32
Length(v4 A)
{
    r32 Result = SquareRoot(LengthSq(A));
    return(Result);
}

inline v4
Clamp01(v4 Value)
{
    v4 Result =
    {
        Clamp01(Value.x),
        Clamp01(Value.y),
        Clamp01(Value.z),
        Clamp01(Value.w),
    };

    return(Result);
}

inline v4
Lerp(v4 A, r32 t, v4 B)
{
    v4 Result =
    {
        Fma(t, B.x - A.x, A.x),
        Fma(t, B.y - A.y, A.y),
        Fma(t, B.z - A.z, A.z),
        Fma(t, B.w - A.w, A.w),
    };

    return(Result);
}

//
// NOTE(chowie): Rectangle2
//

// NOTE(chowie): Only to initialise unions
inline rectangle2
InvertedInfinityRectangle2(void)
{
    rectangle2 Result
    {
        V2(R32Maximum),
        V2(-R32Maximum),
    };

    return(Result);
}

inline rectangle2
Union(rectangle2 A, rectangle2 B)
{
    rectangle2 Result =
    {
        (A.Min.x < B.Min.x) ? A.Min.x : B.Min.x,
        (A.Min.y < B.Min.y) ? A.Min.y : B.Min.y,
        (A.Max.x > B.Max.x) ? A.Max.x : B.Max.x,
        (A.Max.y > B.Max.y) ? A.Max.y : B.Max.y,
    };

    return(Result);
};

inline v2
GetMinCorner(rectangle2 Rect)
{
    v2 Result = Rect.Min;
    return(Result);
}

inline v2
GetMaxCorner(rectangle2 Rect)
{
    v2 Result = Rect.Max;
    return(Result);
}

inline v2
GetDim(rectangle2 Rect)
{
    v2 Result = Rect.Max - Rect.Min;
    return(Result);
}

inline v2
GetCenter(rectangle2 Rect)
{
    v2 Result = 0.5f*(Rect.Min + Rect.Max);
    return(Result);
}

inline rectangle2
RectMinMax(v2 Min, v2 Max)
{
    rectangle2 Result =
    {
        Min,
        Max,
    };

    return(Result);
}

// NOTE(chowie): Min is the corner
inline rectangle2
RectMinDim(v2 Min, v2 Dim)
{
    rectangle2 Result =
    {
        Min,
        Min + Dim,
    };

    return(Result);
}

// NOTE(chowie): Minkowski Sum
inline rectangle2
AddRadiusTo(rectangle2 A, v2 Radius)
{
    rectangle2 Result =
    {
        A.Min - Radius,
        A.Max + Radius,
    };

    return(Result);
}

inline rectangle2
Offset(rectangle2 A, v2 Offset)
{
    rectangle2 Result =
    {
        A.Min + Offset,
        A.Max + Offset,
    };

    return(Result);
}

// NOTE(chowie): Dim stores half-width and half-height
inline rectangle2
RectCenterHalfDim(v2 Center, v2 HalfDim)
{
    rectangle2 Result =
    {
        Center - HalfDim,
        Center + HalfDim,
    };

    return(Result);
}

inline rectangle2
RectCenterDim(v2 Center, v2 Dim)
{
    rectangle2 Result = RectCenterHalfDim(Center, 0.5f*Dim);

    return(Result);
}

inline b32x
IsInRectangle(rectangle2 Rectangle, v2 Test)
{
    b32x Result = ((Test.x >= Rectangle.Min.x) &&
                   (Test.y >= Rectangle.Min.y) &&
                   (Test.x < Rectangle.Max.x) &&
                   (Test.y < Rectangle.Max.y));

    return(Result);
}

// TODO(chowie): One of the dimensions of GetBarycentric could be collapsed,
// where there is no size, that would produce a nonsense value
inline v2
GetBarycentric(rectangle2 A, v2 P)
{
    v2 Result =
    {
        SafeRatio0(P.x - A.Min.x, A.Max.x - A.Min.x),
        SafeRatio0(P.y - A.Min.y, A.Max.y - A.Min.y),
    };

    return(Result);
}

//
// NOTE(chowie): Rectangle3
//

inline v3
GetMinCorner(rectangle3 Rect)
{
    v3 Result = Rect.Min;
    return(Result);
}

inline v3
GetMaxCorner(rectangle3 Rect)
{
    v3 Result = Rect.Max;
    return(Result);
}

inline v3
GetDim(rectangle3 Rect)
{
    v3 Result = Rect.Max - Rect.Min;
    return(Result);
}

inline v3
GetCenter(rectangle3 Rect)
{
    v3 Result = 0.5f*(Rect.Max + Rect.Max);
    return(Result);
}

inline rectangle3
RectMinMax(v3 Min, v3 Max)
{
    rectangle3 Result =
    {
        Min,
        Max,
    };

    return(Result);
}

// NOTE(chowie): Min is the corner
inline rectangle3
RectMinDim(v3 Min, v3 Dim)
{
    rectangle3 Result =
    {
        Min,
        Min + Dim,
    };

    return(Result);
}

inline rectangle3
AddRadiusTo(rectangle3 A, v3 Radius)
{
    rectangle3 Result =
    {
        A.Min - Radius,
        A.Max + Radius,
    };

    return(Result);
}

inline rectangle3
Offset(rectangle3 A, v3 Offset)
{
    rectangle3 Result =
    {
        A.Min + Offset,
        A.Max + Offset,
    };

    return(Result);
}

// NOTE(chowie): Dim stores half-width and half-height
inline rectangle3
RectCenterHalfDim(v3 Center, v3 HalfDim)
{
    rectangle3 Result =
    {
        Center - HalfDim,
        Center + HalfDim
    };

    return(Result);
}

inline rectangle3
RectCenterDim(v3 Center, v3 Dim)
{
    rectangle3 Result = RectCenterHalfDim(Center, 0.5f*Dim);

    return(Result);
}

// RESOURCE: https://fgiesen.wordpress.com/2011/10/16/checking-for-interval-overlap/
// TODO(chowie): Interval Overlap?
inline b32x
IsInRectangle(rectangle3 Rectangle, v3 Test)
{
    b32x Result = ((Test.x >= Rectangle.Min.x) &&
                   (Test.y >= Rectangle.Min.y) &&
                   (Test.z >= Rectangle.Min.z) &&
                   (Test.x < Rectangle.Max.x) &&
                   (Test.y < Rectangle.Max.y) &&
                   (Test.z < Rectangle.Max.z));

    return(Result);
}

inline b32x
RectanglesIntersect(rectangle3 A, rectangle3 B)
{
    b32x Result = !((B.Max.x <= A.Min.x) ||
                    (B.Min.x >= A.Max.x) ||
                    (B.Max.y <= A.Min.y) ||
                    (B.Min.y >= A.Max.y) ||
                    (B.Max.z <= A.Min.z) ||
                    (B.Min.z >= A.Max.z));

    return(Result);
}

inline v3
GetBarycentric(rectangle3 A, v3 P)
{
    v3 Result =
    {
        SafeRatio0(P.x - A.Min.x, A.Max.x - A.Min.x),
        SafeRatio0(P.y - A.Min.y, A.Max.y - A.Min.y),
        SafeRatio0(P.z - A.Min.z, A.Max.z - A.Min.z),
    };

    return(Result);
}

inline rectangle2
ToRectangleXY(rectangle3 A)
{
    rectangle2 Result =
    {
        A.Min.xy,
        A.Max.xy,
    };

    return(Result);
}

//
//
//

// RESOURCE(paniq): https://gist.github.com/paniq/3f882c50f1790e323482
// TODO(chowie): Can this be refined with better abs-identities?
// TODO(chowie): Could this be the base of collision dectection?
union rectangle2i
{
    struct
    {
        v2s Min;
        v2s Max;
    };
    struct
    {
        v2s Min;
        v2s MaxN; // NOTE(chowie): N means Negated
    };
    v4 E[4];
};

inline rectangle2i
Min(rectangle2i A, rectangle2i B)
{
    rectangle2i Result =
    {
        Minimum(A.Min.x, B.Min.x),
        Minimum(A.MaxN.y, B.MaxN.y),
    };

    return(Result);
}

inline rectangle2i
Max(rectangle2i A, rectangle2i B)
{
    rectangle2i Result =
    {
        Maximum(A.Min.x, B.Min.x),
        Maximum(A.MaxN.y, B.MaxN.y),
    };

    return(Result);
}

// RESOURCE: https://fgiesen.wordpress.com/2013/01/14/min-max-under-negation-and-an-aabb-trick/
// TODO(chowie): Does this work? SIMD this!
// NOTE(chowie): Requires negative max!
inline rectangle2i
UnionN(rectangle2i A, rectangle2i B)
{
    rectangle2i Result;

    for(s32 Index = 0;
        Index < ArrayCount(Result.E);
        ++Index)
    {
        Result = Min(A, B);
    }

    return(Result);
};

// NOTE(chowie): Requires negative max!
inline rectangle2i
IntersectN(rectangle2i A, rectangle2i B)
{
    rectangle2i Result;
    
    for(s32 Index = 0;
        Index < ArrayCount(Result.E);
        ++Index)
    {
        Result = Max(A, B);
    }

    return(Result);
};

/*
inline rectangle2i
Union(rectangle2i A, rectangle2i B)
{
    rectangle2i Result =
    {
        (A.Min.x < B.Min.x) ? A.Min.x : B.Min.x,
        (A.Min.y < B.Min.y) ? A.Min.y : B.Min.y,
        (A.Max.x > B.Max.x) ? A.Max.x : B.Max.x,
        (A.Max.y > B.Max.y) ? A.Max.y : B.Max.y,
    };

    return(Result);
};

inline rectangle2i
Intersect(rectangle2i A, rectangle2i B)
{
    rectangle2i Result =
    {
        (A.Min.x < B.Min.x) ? B.Min.x : A.Min.x,
        (A.Min.y < B.Min.y) ? B.Min.y : A.Min.y,
        (A.Max.x > B.Max.x) ? B.Max.x : A.Max.x,
        (A.Max.y > B.Max.y) ? B.Max.y : A.Max.y,
    };

    return(Result);
};
*/

// RESOURCE: https://web.archive.org/web/20211023131624/https://lolengine.net/blog/2012/02/14/fewer-comparisons
// TODO(chowie): Less comparisons for collisions?
// NOTE(chowie): float a, b, c, d;
//               /* ... */
//               return (a > b) && (c > d);
//               The below equation might be branchless?
//               (a - b) + (c - d) > fabsf((a - b) - (c - d));

inline s32
GetClampedRectArea(rectangle2i A)
{
    s32 Width = (A.Max.x - A.Min.x);
    s32 Height = (A.Max.y - A.Min.y);
    s32 Result = 0;
    if((Width > 0) && (Height > 0))
    {
        Result = Width * Height;
    }

    return(Result);
}

inline b32x
HasArea(rectangle2i A)
{
    b32x Result = ((A.Min.x < A.Max.x) && (A.Min.y < A.Max.y));

    return(Result);
}

// NOTE(chowie): Only to initialise unions
inline rectangle2i
InvertedInfinityRectangle2i(void)
{
    rectangle2i Result
    {
        V2S(INT_MAX),
        V2S(-INT_MAX),
    };

    return(Result);
}

// RESOURCE: https://web.archive.org/web/20211023131624/https://lolengine.net/blog/2011/3/20/understanding-fast-float-integer-conversions
// TODO(chowie): Conversion for colours?
inline v4
SRGB255ToLinear1(v4 C)
{
#define Inv255 (1.0f / 255.0f)
    v4 Result =
    {
        Square(Inv255*C.r),
        Square(Inv255*C.g),
        Square(Inv255*C.b),
        Inv255*C.a,
    };

    // NOTE(chowie): Alpha is not a brightness value, but a value tells us how
    // much we want to blend (as a percentage). Only ever blend in
    // that linear space anyway, assume inputs are in linearly encoded
    // alpha (but who knows really depending on the program).
    // NOTE(chowie): Alpha channels are not converted to sRGB, is linear.

    return(Result);
}

inline v4
Linear1ToSRGB255(v4 C)
{
#define One255 255.0f
    v4 Result =
    {
        One255*SquareRoot(C.r),
        One255*SquareRoot(C.g),
        One255*SquareRoot(C.b),
        One255*C.a,
    };

    return(Result);
}

// TODO(chowie): Use this! Computing the binary logarithm is
// equivalent to knowing the position of the highest order set
// bit. For instance, log2(0x1) is 0 and log2(0x100) is 8.
// RESOURCE: https://web.archive.org/web/20211023131624/https://lolengine.net/blog/2012/4/3/beyond-de-bruijn
// RESOURCE: https://web.archive.org/web/20210724051712/https://lolengine.net/attachment/blog/2012/4/3/beyond-de-bruijn/debruijn.cpp
// NOTE(chowie): Beyond De Bruijn: fast binary logarithm of a 10-bit number
global s32 MagicTable[16] = 
{
    0, 1, 2, 8, -1, 3, 5, 9, 9, 7, 4, -1, 6, -1, -1, -1,
};
inline s32
FastLog2(u32 Value)
{
    Value |= Value >> 1;
    Value |= Value >> 2;
    Value |= Value >> 4;

    return(MagicTable[(u32)(Value * 0x5a1a1a2u) >> 28]);
}

// RESOURCE(sam): https://web.archive.org/web/20211023131624/https://lolengine.net/blog/2011/12/14/understanding-motion-in-games
// TODO(chowie): Implement Verlet equation of motions instead of Euler
//  Accel = V3(0, 0, -9.81f);
//  v3 OldVel = Vel;
//  Vel = Vel + Accel * dtForFrame;
//  Pos = Pos + (OldVel + Vel) * 0.5f * dtForFrame;

// TODO(chowie): Try to use this!
// RESOURCE: https://gist.github.com/d7samurai/f98cb2aa30a6d73e62a65a376d24c6da
// NOTE(chowie): Storing argb color in various compact forms, either
// as uint32 or packed into an __m128i, where I had to multply alpha
// values from nested containers and it was beneficial to do it
// in-place, as bytes, without extracting to float and repacking

// normalized byte multiplication:
// range [0, 255] maps to [0.0, 1.0]
// examples:
// 0xff * 0xff = 0xff   (255 * 255 = 255)
// 0xff * 0x7f = 0x7f   (255 * 127 = 127)
// 0x7f * 0x7f = 0x3f   (127 * 127 =  63)
// 0x01 * 0xff = 0x01   (  1 * 255 =   1)
// 0x01 * 0x7f = 0x00   (  1 * 127 =   0) 
inline u8
D7samNormalisedMul(u8 A, u8 B)
{
    return (((u16)A + 1) * B) >> 8;
}

//
// NOTE(chowie): Hashing Functions
//

// TODO(chowie): Better Hash Functions!

// RESOURCE(orlp): https://stackoverflow.com/questions/101439/the-most-efficient-way-to-implement-an-integer-based-power-function-powint-int
// TODO(chowie): iPow? https://gist.github.com/orlp/3551590
// NOTE(by mauro): Using Szudnik Pairing.
// * Seed would always be the same based on location, and collisions would only occur as you got very far away from the origin
// * You could fit two 16-bit integers into a single 32-bit integer with no collisions.
// RESOURCE(mauro): https://dmauro.com/post/77011214305/a-hashing-function-for-x-y-z-coordinates
inline u32
MauroHash(v3u Value)
{
    u32 Max = Maximum3(Value.x, Value.y, Value.z);
    u32 Result = Pow(Max, 3) + (2 * Max * Value.z) + Value.z;
    if(Max == Value.z)
    {
        Result += Pow(Maximum(Value.x, Value.y), 2);
    }

    if(Value.y >= Value.x)
    {
        Result += (Value.x + Value.y);
    }
    else
    {
        Result += Value.y;
    }

    return(Result);
}

inline s32
MauroHash(v3s Value)
{
    s32 NegX = (Value.x >= 0) ? (2 * Value.x) : (-2 * Value.x - 1);
    s32 NegY = (Value.y >= 0) ? (2 * Value.y) : (-2 * Value.y - 1);
    s32 NegZ = (Value.z >= 0) ? (2 * Value.z) : (-2 * Value.z - 1);

    s32 Max = Maximum3(NegX, NegY, NegZ);
    s32 Result = Pow(Max, 3) + (2 * Max * NegZ) + NegZ;
    if(Max == NegZ)
    {
        Result += Pow(Maximum(NegX, NegY), 2);
    }

    if(NegY >= NegX)
    {
        Result += (NegX + NegY);
    }
    else
    {
        Result += NegY;
    }

    return(Result);
}

// TODO(chowie): Spatial Hash Table? For particles?
// RESOURCE(cincotti): https://carmencincotti.com/2022-10-31/spatial-hash-maps-part-one/#how-to-populate-a-dense-hash-table
// NOTE(chowie): Originally intended for s32, with an Abs. Why not
// just type cast to u32?
inline u32
MullerHash(v3u Value)
{
    u32 Result = (Value.x * 92837111) ^ (Value.y * 689287499) ^ (Value.z * 283923481);

    return(Result);
}

//
// NOTE(chowie): Compression
//

//
// RESOURCE(fabian): https://fgiesen.wordpress.com/2009/12/13/decoding-morton-codes/
//

// TODO(chowie): Try to use this for tile/world storage? (Must be in u32)
// NOTE(by fabian): "Insert" a 0 bit after each of the 16 low bits of x
inline u32
Part1By1(u32 Value)
{
    Value &= 0x0000ffff;                          // Value = ---- ---- ---- ---- fedc ba98 7654 3210
    Value = (Value ^ (Value <<  8)) & 0x00ff00ff; // Value = ---- ---- fedc ba98 ---- ---- 7654 3210
    Value = (Value ^ (Value <<  4)) & 0x0f0f0f0f; // Value = ---- fedc ---- ba98 ---- 7654 ---- 3210
    Value = (Value ^ (Value <<  2)) & 0x33333333; // Value = --fe --dc --ba --98 --76 --54 --32 --10
    Value = (Value ^ (Value <<  1)) & 0x55555555; // Value = -f-e -d-c -b-a -9-8 -7-6 -5-4 -3-2 -1-0
    return(Value);
}

// NOTE(by fabian): "Insert" two 0 bits after each of the 10 low bits of Value
inline u32
Part1By2(u32 Value)
{
    Value &= 0x000003ff;                          // Value = ---- ---- ---- ---- ---- --98 7654 3210
    Value = (Value ^ (Value << 16)) & 0xff0000ff; // Value = ---- --98 ---- ---- ---- ---- 7654 3210
    Value = (Value ^ (Value <<  8)) & 0x0300f00f; // Value = ---- --98 ---- ---- 7654 ---- ---- 3210
    Value = (Value ^ (Value <<  4)) & 0x030c30c3; // Value = ---- --98 ---- 76-- --54 ---- 32-- --10
    Value = (Value ^ (Value <<  2)) & 0x09249249; // Value = ---- 9--8 --7- -6-- 5--4 --3- -2-- 1--0
    return(Value);
}

inline u32
EncodeMortonV2U(v2u Value)
{
    u32 Result = (Part1By1(Value.y) << 1) + Part1By1(Value.x);

    return(Result);
}

inline u32
EncodeMortonV2U(v3u Value)
{
    u32 Result = (Part1By2(Value.z) << 2) + (Part1By2(Value.y) << 1) + Part1By2(Value.x);
    return(Result);
}

// NOTE(by fabian): Inverse of Part1By1 - "delete" all odd-indexed bits
inline u32
Compact1By1(u32 Value)
{
    Value &= 0x55555555;                          // Value = -f-e -d-c -b-a -9-8 -7-6 -5-4 -3-2 -1-0
    Value = (Value ^ (Value >>  1)) & 0x33333333; // Value = --fe --dc --ba --98 --76 --54 --32 --10
    Value = (Value ^ (Value >>  2)) & 0x0f0f0f0f; // Value = ---- fedc ---- ba98 ---- 7654 ---- 3210
    Value = (Value ^ (Value >>  4)) & 0x00ff00ff; // Value = ---- ---- fedc ba98 ---- ---- 7654 3210
    Value = (Value ^ (Value >>  8)) & 0x0000ffff; // Value = ---- ---- ---- ---- fedc ba98 7654 3210
    return(Value);
}

// NOTE(by fabian): Inverse of Part1By2 - "delete" all bits not at positions divisible by 3
inline u32
Compact1By2(u32 Value)
{
    Value &= 0x09249249;                          // Value = ---- 9--8 --7- -6-- 5--4 --3- -2-- 1--0
    Value = (Value ^ (Value >>  2)) & 0x030c30c3; // Value = ---- --98 ---- 76-- --54 ---- 32-- --10
    Value = (Value ^ (Value >>  4)) & 0x0300f00f; // Value = ---- --98 ---- ---- 7654 ---- ---- 3210
    Value = (Value ^ (Value >>  8)) & 0xff0000ff; // Value = ---- --98 ---- ---- ---- ---- 7654 3210
    Value = (Value ^ (Value >> 16)) & 0x000003ff; // Value = ---- ---- ---- ---- ---- --98 7654 3210
    return(Value);
}

inline v2u
DecodeMorton2(u32 Value)
{
    v2u Result =
    {
        Compact1By1(Value >> 0),
        Compact1By1(Value >> 1),
    };

    return(Result);
}

inline v3u
DecodeMorton3(u32 Value)
{
    v3u Result =
    {
        Compact1By2(Value >> 0),
        Compact1By2(Value >> 1),
        Compact1By2(Value >> 2),
    };

    return(Result);
}

//
// NOTE(chowie): Sorting
//

#define Prefetch64 64
#define Prefetch128 128
#define Prefetch0(Value, HistIndex) _mm_prefetch((char *)(Value + HistIndex + Prefetch64), 0)
#define Prefetch1(Value, HistIndex) _mm_prefetch((char *)(Value + HistIndex + Prefetch128), 0)

#define PrefetchHist0_(Value) (Value & 0x7FF)
#define PrefetchHist1_(Value) (Value >> 11 & 0x7FF)
#define PrefetchHist2_(Value) (Value >> 22)

// RESOURCE(chowie): https://fastcpp.blogspot.com/2011/03/changing-sign-of-float-values-using-sse.html
// TODO(chowie): Convert to BitscanForward & Reverse? Or SSE?
inline u32
FloatFlip(u32 R32)
{
    u32 Mask = -(s32)(R32 >> 31) | 0x80000000;
    u32 Result = R32 ^ Mask;
    return(Result);
}

inline void
FloatFlipX(u32 *R32)
{
    u32 Mask = -(s32)(*R32 >> 31) | 0x80000000;
    *R32 ^= Mask;
}

inline u32
InvFloatFlip(u32 R32)
{
    u32 Mask = ((R32 >> 31) - 1) | 0x80000000;
    u32 Result = R32 ^ Mask;
    return(Result);
}

internal void
HerfRadixSort(u32 Count, r32 *First, r32 *Temp)
{
    u32 *Source = (u32 *)First;
    u32 *Dest = (u32 *)Temp;

    // NOTE(chowie): 3 Histograms on stack, by bytes
#define HistogramMax 2048
    u32 Hist0[HistogramMax * 3] = {};
    u32 *Hist1 = Hist0 + HistogramMax;
    u32 *Hist2 = Hist1 + HistogramMax;

    // NOTE(chowie): 1. Parallel Histogram Pass
    for(u32 HistIndex = 0;
        HistIndex < Count;
        ++HistIndex)
    {
        Prefetch0(Source, HistIndex);

        u32 Flip = FloatFlip(Source[HistIndex]);

        ++Hist0[PrefetchHist0_(Flip)];
        ++Hist1[PrefetchHist1_(Flip)];
        ++Hist2[PrefetchHist2_(Flip)];
    }

    // NOTE(chowie): 2. Sum histograms' entries, records number of values preceeding itself.
    {
        u32 SumHist0 = 0;
        u32 SumHist1 = 0;
        u32 SumHist2 = 0;
        u32 Total;

        for(u32 HistIndex = 0;
            HistIndex < HistogramMax;
            ++HistIndex)
        {
            Total = Hist0[HistIndex] + SumHist0;
            Hist0[HistIndex] = SumHist0 - 1;
            SumHist0 = Total;

            Total = Hist1[HistIndex] + SumHist1;
            Hist1[HistIndex] = SumHist1 - 1;
            SumHist1 = Total;

            Total = Hist2[HistIndex] + SumHist2;
            Hist2[HistIndex] = SumHist2 - 1;
            SumHist2 = Total;
        }
    }

    // NOTE(chowie): Byte 0, Flip value, read/write histogram, write out flipped
    for(u32 HistIndex = 0;
        HistIndex < HistogramMax;
        ++HistIndex)
    {
        u32 Flip = Source[HistIndex];
        FloatFlipX(&Flip);
        u32 Pos = PrefetchHist0_(Flip);

        Prefetch1(Source, HistIndex);
        Dest[++Hist0[Pos]] = Flip;
    }

    // NOTE(chowie): Byte 1, Read-write histogram, copy dest -> source
    for(u32 HistIndex = 0;
        HistIndex < HistogramMax;
        ++HistIndex)
    {
        u32 DestIndex = Dest[HistIndex];
        u32 Pos = PrefetchHist1_(DestIndex);

        Prefetch1(Dest, HistIndex);
        Source[++Hist1[Pos]] = DestIndex;
    }

    // NOTE(chowie): Byte 2, Read-write histogram, copy & flip out source -> dest
    for(u32 HistIndex = 0;
        HistIndex < HistogramMax;
        ++HistIndex)
    {
        u32 SourceIndex = Source[HistIndex];
        u32 Pos = PrefetchHist2_(SourceIndex);

        Prefetch1(Source, HistIndex);
        Dest[++Hist2[Pos]] = InvFloatFlip(SourceIndex);
    }
}

#define RUINENGLASS_MATH_H
#endif
