#if !defined(RUINENGLASS_MATH_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Michael Chow $
   $Notice: $
   ======================================================================== */

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
    v2 Result;

    Result.x = X;
    Result.y = Y;

    return(Result);
}

inline v3
V3(r32 X, r32 Y, r32 Z)
{
    v3 Result;

    Result.x = X;
    Result.y = Y;
    Result.z = Z;

    return(Result);
}

inline v3
V3(v2 XY, r32 Z)
{
    v3 Result;

    Result.x = XY.x;
    Result.y = XY.y;
    Result.z = Z;

    return(Result);
}

inline v4
V4(r32 X, r32 Y, r32 Z, r32 W)
{
    v4 Result;

    Result.x = X;
    Result.y = Y;
    Result.z = Z;
    Result.w = W;

    return(Result);
}

inline v4
V4(v3 XYZ, r32 W)
{
    v4 Result;

    Result.xyz = XYZ;
    Result.w = W;

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
    r32 Result = A*A;

    return(Result);
}

inline r32
Cube(r32 A)
{
    r32 Result = A*A*A;

    return(Result);
}

inline r32
Lerp(r32 A, r32 t, r32 B)
{
    r32 Result = (1.0f - t)*A + t*B;

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

// NOTE(chowie): This is Saturate RESOURCE: https://developer.download.nvidia.com/cg/saturate.html
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
    r32 Result = Square(t) - (2*Cube(t)); /* 3t^2 - 2t^3 */

    return(Result);
}

// TODO(chowie): Test smooth-step and pulse for alpha tests? RESOURCE: https://realtimecollisiondetection.net/blog/?p=95
inline r32
SmoothStep(r32 Min, r32 t, r32 Max)
{
    r32 Result = Clamp01MapToRange(Min, t, Max);
    Result = Square(Result) * (3 - 2*Result);

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
    v2 Result;

    Result.x = A*B.x;
    Result.y = A*B.y;

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
    v2 Result;

    Result.x = -A.x;
    Result.y = -A.y;

    return(Result);
}

// NOTE(chowie): A + B is valid syntax
inline v2
operator+(v2 A, v2 B)
{
    v2 Result;

    Result.x = A.x + B.x;
    Result.y = A.y + B.y;

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
    v2 Result;

    Result.x = A.x - B.x;
    Result.y = A.y - B.y;

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
    v2 Result;

    Result.x = Clamp01(Value.x);
    Result.y = Clamp01(Value.y);

    return(Result);
}

inline v2
Lerp(v2 A, r32 t, v2 B)
{
    v2 Result = (1.0f - t)*A + t*B;

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
// NOTE(chowie): v3 operations
//

// NOTE(chowie): A single multiplier
inline v3
operator*(r32 A, v3 B)
{
    v3 Result;

    Result.x = A*B.x;
    Result.y = A*B.y;
    Result.z = A*B.z;

    return(Result);
}

// NOTE(chowie): 5*Vector or Vector*5 is valid syntax
inline v3
operator*(v3 B, r32 A)
{
    v3 Result = A*B;

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
    v3 Result;

    Result.x = -A.x;
    Result.y = -A.y;
    Result.z = -A.z;

    return(Result);
}

// NOTE(chowie): A + B is valid syntax
inline v3
operator+(v3 A, v3 B)
{
    v3 Result;

    Result.x = A.x + B.x;
    Result.y = A.y + B.y;
    Result.z = A.z + B.z;

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
    v3 Result;

    Result.x = A.x - B.x;
    Result.y = A.y - B.y;
    Result.z = A.z - B.z;

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
    v3 Result;

    Result.x = A.y*B.z - A.z*B.y;
    Result.y = A.z*B.x - A.x*B.z;
    Result.z = A.x*B.y - A.y*B.x;

    return(Result);
}

inline r32
LengthSq(v3 A)
{
    r32 Result = Inner(A, A);
    return(Result);
}

// STUDY(chowie): Sqroot is undefined negative numbers. LengthSq
// cannot produce a negative number, i.e Negative*Negative is positive
inline r32
Length(v3 A)
{
    r32 Result = SquareRoot(LengthSq(A));
    return(Result);
}

inline v3
Normalise(v3 A)
{
    v3 Result = A * (1.0f / Length(A));

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
        Result = A * (1.0f / SquareRoot(LenSq));
    }

    return(Result);
}

// TODO: Try biarc interpolation? RESOURCE: https://www.ryanjuckett.com/biarc-interpolation/
// TODO(chowie): Not quite sure how useful this function is - for asserts mainly
// NOTE(chowie): Vector length is within epsilon of 1
inline b32
IsNormalisedEps(v3 A, r32 Epsilon)
{
    r32 Magnitude = LengthSq(A);
    b32 Result = (Magnitude >= Square(1.0f - Epsilon) &&
                  Magnitude <= Square(1.0f + Epsilon));
    return(Result);
}

// RESOURCE: https://web.archive.org/web/20200414122444/https://www.randygaul.net/2014/11/
// RESOURCE: https://www.reddit.com/r/gamedev/comments/2ljvlz/collection_of_numerically_robust_parallel_vectors/
// TODO(chowie): Double Check this!
inline b32
IsPerp(v3 A, v3 B, r32 Epsilon)
{
    b32 Result = false;
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
inline b32
IsParallel(v3 A, v3 B, r32 Epsilon)
{
    b32 Result;

    A = NOZ(A);
    B = NOZ(B);
    Result = ((1.0f - AbsoluteValue(Inner(A, B))) < Epsilon);

    return(Result);
}

inline v3
Clamp01(v3 Value)
{
    v3 Result;

    Result.x = Clamp01(Value.x);
    Result.y = Clamp01(Value.y);
    Result.z = Clamp01(Value.z);

    return(Result);
}

inline v3
Lerp(v3 A, r32 t, v3 B)
{
    v3 Result = (1.0f - t)*A + t*B;

    return(Result);
}

// RESOURCE: https://box2d.org/posts/2014/02/computing-a-basis/
// TODO(chowie): Find out if this can be a replacement for entities? If so, SIMD this? RESOURCE: https://web.archive.org/web/20190120215637/http://www.randygaul.net/2015/10/27/compute-basis-with-simd/
inline void
GetBasis(v3 A, v3 B, v3 C)
{
    // Suppose vector a has all equal components and is a unit vector:
    // a = (s, s, s)
    // Then 3*s*s = 1, s = sqrt(1/3) = 0.57735. This means that at
    // least one component of a unit vector must be greater or equal
    // to 0.57735.

    if(AbsoluteValue(A.x) >= 0.57735)
    {
        B = V3(A.y, -A.x, 0.0f);
    }
    else
    {
        B = V3(0.0f, A.z, -A.y);
    }

    B = Normalise(B);
    C = Cross(A, B);
}

//
// NOTE(chowie): v4 operations
//

// NOTE(chowie): A single multiplier
inline v4
operator*(r32 A, v4 B)
{
    v4 Result;

    Result.x = A*B.x;
    Result.y = A*B.y;
    Result.z = A*B.z;
    Result.w = A*B.w;

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
    v4 Result;

    Result.x = -A.x;
    Result.y = -A.y;
    Result.z = -A.z;
    Result.w = -A.w;

    return(Result);
}

// NOTE(chowie): A + B is valid syntax
inline v4
operator+(v4 A, v4 B)
{
    v4 Result;

    Result.x = A.x + B.x;
    Result.y = A.y + B.y;
    Result.z = A.z + B.z;
    Result.w = A.w + B.w;

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
    v4 Result;

    Result.x = A.x - B.x;
    Result.y = A.y - B.y;
    Result.z = A.z - B.z;
    Result.w = A.w - B.w;

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
    v4 Result;

    Result.x = Clamp01(Value.x);
    Result.y = Clamp01(Value.y);
    Result.z = Clamp01(Value.z);
    Result.w = Clamp01(Value.w);

    return(Result);
}

inline v4
Lerp(v4 A, r32 t, v4 B)
{
    v4 Result = (1.0f - t)*A + t*B;

    return(Result);
}

//
// NOTE(chowie): Rectangle2
//

inline rectangle2
InvertedInfinityRectangle2(void)
{
    rectangle2 Result;

    Result.Min.x = Result.Min.y = R32Maximum;
    Result.Max.x = Result.Max.y = -R32Maximum;

    return(Result);
}

inline rectangle2
Union(rectangle2 A, rectangle2 B)
{
    rectangle2 Result;

    Result.Min.x = (A.Min.x < B.Min.x) ? A.Min.x : B.Min.x;
    Result.Min.y = (A.Min.y < B.Min.y) ? A.Min.y : B.Min.y;
    Result.Max.x = (A.Max.x > B.Max.x) ? A.Max.x : B.Max.x;
    Result.Max.y = (A.Max.y > B.Max.y) ? A.Max.y : B.Max.y;

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
    rectangle2 Result;

    Result.Min = Min;
    Result.Max = Max;

    return(Result);
}

// NOTE(chowie): Min is the corner
inline rectangle2
RectMinDim(v2 Min, v2 Dim)
{
    rectangle2 Result;

    Result.Min = Min;
    Result.Max = Min + Dim;

    return(Result);
}

// NOTE(chowie): Dim stores half-width and half-height
inline rectangle2
RectCenterHalfDim(v2 Center, v2 HalfDim)
{
    rectangle2 Result;

    Result.Min = Center - HalfDim;
    Result.Max = Center + HalfDim;

    return(Result);
}

// NOTE(chowie): Minkowski Sum
inline rectangle2
AddRadiusTo(rectangle2 A, v2 Radius)
{
    rectangle2 Result;

    Result.Min = A.Min - Radius;
    Result.Max = A.Max + Radius;

    return(Result);
}

inline rectangle2
Offset(rectangle2 A, v2 Offset)
{
    rectangle2 Result;

    Result.Min = A.Min + Offset;
    Result.Max = A.Max + Offset;

    return(Result);
}

inline rectangle2
RectCenterDim(v2 Center, v2 Dim)
{
    rectangle2 Result = RectCenterHalfDim(Center, 0.5f*Dim);

    return(Result);
}

inline b32
IsInRectangle(rectangle2 Rectangle, v2 Test)
{
    b32 Result = ((Test.x >= Rectangle.Min.x) &&
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
    v2 Result;

    Result.x = SafeRatio0(P.x - A.Min.x, A.Max.x - A.Min.x);
    Result.y = SafeRatio0(P.y - A.Min.y, A.Max.y - A.Min.y);

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
    rectangle3 Result;

    Result.Min = Min;
    Result.Max = Max;

    return(Result);
}

// NOTE(chowie): Min is the corner
inline rectangle3
RectMinDim(v3 Min, v3 Dim)
{
    rectangle3 Result;

    Result.Min = Min;
    Result.Max = Min + Dim;

    return(Result);
}

// NOTE(chowie): Dim stores half-width and half-height
inline rectangle3
RectCenterHalfDim(v3 Center, v3 HalfDim)
{
    rectangle3 Result;

    Result.Min = Center - HalfDim;
    Result.Max = Center + HalfDim;

    return(Result);
}

inline rectangle3
AddRadiusTo(rectangle3 A, v3 Radius)
{
    rectangle3 Result;

    Result.Min = A.Min - Radius;
    Result.Max = A.Max + Radius;

    return(Result);
}

inline rectangle3
Offset(rectangle3 A, v3 Offset)
{
    rectangle3 Result;

    Result.Min = A.Min + Offset;
    Result.Max = A.Max + Offset;

    return(Result);
}

inline rectangle3
RectCenterDim(v3 Center, v3 Dim)
{
    rectangle3 Result = RectCenterHalfDim(Center, 0.5f*Dim);

    return(Result);
}

inline b32
IsInRectangle(rectangle3 Rectangle, v3 Test)
{
    b32 Result = ((Test.x >= Rectangle.Min.x) &&
                     (Test.y >= Rectangle.Min.y) &&
                     (Test.z >= Rectangle.Min.z) &&
                     (Test.x < Rectangle.Max.x) &&
                     (Test.y < Rectangle.Max.y) &&
                     (Test.z < Rectangle.Max.z));

    return(Result);
}

inline b32
RectanglesIntersect(rectangle3 A, rectangle3 B)
{
    b32 Result = !((B.Max.x <= A.Min.x) ||
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
    v3 Result;

    Result.x = SafeRatio0(P.x - A.Min.x, A.Max.x - A.Min.x);
    Result.y = SafeRatio0(P.y - A.Min.y, A.Max.y - A.Min.y);
    Result.z = SafeRatio0(P.z - A.Min.z, A.Max.z - A.Min.z);

    return(Result);
}

inline rectangle2
ToRectangleXY(rectangle3 A)
{
    rectangle2 Result;

    Result.Min = A.Min.xy;
    Result.Max = A.Max.xy;

    return(Result);
}

//
//
//

struct rectangle2i
{
    s32 MinX, MinY;
    s32 MaxX, MaxY;
};

inline rectangle2i
Intersect(rectangle2i A, rectangle2i B)
{
    rectangle2i Result;

    Result.MinX = (A.MinX < B.MinX) ? B.MinX : A.MinX;
    Result.MinY = (A.MinY < B.MinY) ? B.MinY : A.MinY;
    Result.MaxX = (A.MaxX > B.MaxX) ? B.MaxX : A.MaxX;
    Result.MaxY = (A.MaxY > B.MaxY) ? B.MaxY : A.MaxY;

    return(Result);
};

inline rectangle2i
Union(rectangle2i A, rectangle2i B)
{
    rectangle2i Result;

    Result.MinX = (A.MinX < B.MinX) ? A.MinX : B.MinX;
    Result.MinY = (A.MinY < B.MinY) ? A.MinY : B.MinY;
    Result.MaxX = (A.MaxX > B.MaxX) ? A.MaxX : B.MaxX;
    Result.MaxY = (A.MaxY > B.MaxY) ? A.MaxY : B.MaxY;

    return(Result);
};

inline s32
GetClampedRectArea(rectangle2i A)
{
    s32 Width = (A.MaxX - A.MinX);
    s32 Height = (A.MaxY - A.MinY);
    s32 Result = 0;
    if((Width > 0) && (Height > 0))
    {
        Result = Width*Height;
    }

    return(Result);
}

inline b32
HasArea(rectangle2i A)
{
    b32 Result = ((A.MinX < A.MaxX) && (A.MinY < A.MaxY));

    return(Result);
}

inline rectangle2i
InvertedInfinityRectangle2i(void)
{
    rectangle2i Result;

    Result.MinX = Result.MinY = INT_MAX;
    Result.MaxX = Result.MaxY = -INT_MAX;

    return(Result);
}

inline v4
SRGB255ToLinear1(v4 C)
{
    v4 Result;

    r32 Inv255 = 1.0f / 255.0f;

    Result.r = Square(Inv255*C.r);
    Result.g = Square(Inv255*C.g);
    Result.b = Square(Inv255*C.b);
    // NOTE(chowie): Alpha is not a brightness value, but a value tells us how
    // much we want to blend (as a percentage). Only ever blend in
    // that linear space anyway, assume inputs are in linearly encoded
    // alpha (but who knows really depending on the program).
    // NOTE(chowie): Alpha channels are not converted to sRGB, is linear
    // NOTE(chowie): Alpha channel is not an sRGB in current system, not converting
    Result.a = Inv255*C.a;

    return(Result);
}

inline v4
Linear1ToSRGB255(v4 C)
{
    v4 Result;

    r32 One255 = 255.0f;

    Result.r = One255*SquareRoot(C.r);
    Result.g = One255*SquareRoot(C.g);
    Result.b = One255*SquareRoot(C.b);
    Result.a = One255*C.a;

    return(Result);
}

#define RUINENGLASS_MATH_H
#endif
