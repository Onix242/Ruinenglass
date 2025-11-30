#if !defined(RUINENGLASS_MATH_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Michael Chow $
   $Notice: $
   ======================================================================== */

#define QUADRATIC(a) ((a)*(a))
#define CUBIC(a) ((a)*(a)*(a))
#define QUARTIC(a) ((a)*(a)*(a)*(a)) // TODO(chowie): Use?
#define QUINTIC(a) ((a)*(a)*(a)*(a)*(a))
#define Epsilon32 0.0001f

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
    v2 Result = {(f32)X, (f32)Y};
    return(Result);
}

inline v2
V2i(u32 X, u32 Y)
{
    v2 Result = {(f32)X, (f32)Y};
    return(Result);
}

inline v2
V2i(v2s A)
{
    v2 Result = {(f32)A.x, (f32)A.y};
    return(Result);
}

inline v2
V2i(v2u A)
{
    v2 Result = {(f32)A.x, (f32)A.y};
    return(Result);
}

inline v3
V3i(v3s A)
{
    v3 Result = {(f32)A.x, (f32)A.y, (f32)A.z};
    return(Result);
}

inline v3
V3i(v3u A)
{
    v3 Result = {(f32)A.x, (f32)A.y, (f32)A.z};
    return(Result);
}

inline v2
V2(f32 X, f32 Y)
{
    v2 Result = {X, Y};
    return(Result);
}

// NOTE(chowie): I want to say, Min = "Some Value" for both.
inline v2
V2(f32 Value)
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
V3(f32 X, f32 Y, f32 Z)
{
    v3 Result = {X, Y, Z};
    return(Result);
}

inline v3
V3(f32 Value)
{
    v3 Result = {Value, Value, Value};
    return(Result);
}

inline v3
V3(v2 XY, f32 Z)
{
    v3 Result = {XY.x, XY.y, Z};
    return(Result);
}

inline v4
V4(f32 X, f32 Y, f32 Z, f32 W)
{
    v4 Result = {X, Y, Z, W};
    return(Result);
}

inline v4
V4(v3 XYZ, f32 W)
{
    v4 Result = {XYZ, W};
    return(Result);
}

inline v4u
V4U(u32 X, u32 Y, u32 Z, u32 W)
{
    v4u Result = {X, Y, Z, W};
    return(Result);
}

inline v4
V4(f32 Value)
{
    v4 Result = {Value, Value, Value, Value};
    return(Result);
}

// NOTE(chowie): Use SafeRatio where you are less concerned about
// performance and more concerned about robustness. Guard from dividing by 0
inline f32
SafeRatioN(f32 Numerator, f32 Divisor, f32 N)
{
    f32 Result = N;
    if(Divisor != 0.0f)
    {
        Result = Numerator / Divisor;
    }

    return(Result);
}

inline f32
SafeRatio0(f32 Numerator, f32 Divisor)
{
    f32 Result = SafeRatioN(Numerator, Divisor, 0.0f);
    return(Result);
}

inline f32
SafeRatio1(f32 Numerator, f32 Divisor)
{
    f32 Result = SafeRatioN(Numerator, Divisor, 1.0f);
    return(Result);
}

inline f64
SafeRatioN(f64 Numerator, f64 Divisor, f64 N)
{
    f64 Result = N;
    if(Divisor != 0.0f)
    {
        Result = Numerator / Divisor;
    }

    return(Result);
}

inline f64
SafeRatio0(f64 Numerator, f64 Divisor)
{
    f64 Result = SafeRatioN(Numerator, Divisor, 0.0); // NOTE: Trick to take away 'f' from '0.0f' to allow the call!
    return(Result);
}

//
// NOTE(chowie): Scalar operations
//

// NOTE(chowie): Squaring functions are nice for whole expressions
inline f32
Square(f32 A)
{
    f32 Result = QUADRATIC(A);
    return(Result);
}

inline v3
Square(v3 A)
{
    v3 Result = {QUADRATIC(A.r), QUADRATIC(A.g), QUADRATIC(A.b)};
    return(Result);
}

inline f32
Cube(f32 A)
{
    f32 Result = CUBIC(A);
    return(Result);
}

inline f32
Quin(f32 A)
{
    f32 Result = QUINTIC(A);
    return(Result);
}

// RESOURCE(john d cook): Sin approx (0 to Pi32) - https://www.johndcook.com/blog/2024/08/31/sine-approx/
// RESOURCE(johndcook): https://www.johndcook.com/blog/2024/09/07/aryabhata/
// TODO(chowie): Blog also contains Cos approx (0 to Pi32), what are ways to use it?
// NOTE(chowie): Parabolas 0.5f, used with lerp! Oner on caller to
// pass -0.25f*Tau32 to 0.25f*Tau32. For rotation e.g. sword swings
inline f32
Sin01(f32 t)
{
    f32 Range = (Pi32*t);
    // NOTE(chowie): Sin approx (0 to Pi32) originates from Aryabhata I, around 500 AD.
    f32 Result = ((16*Range)*(Pi32 - Range)) / (5*Square(Pi32) - 4*Range*(Pi32 - Range));
    return(Result);
}

// RESOURCE(endesga): https://www.shadertoy.com/view/M3jGWV
// TODO(chowie): Proper write of Tau32 Sin!

// RESOURCE(): https://blog.demofox.org/2014/11/04/four-ways-to-calculate-sine-without-trig/
// TODO(chowie): Sine underscore with lerp!

// RESOURCE(): https://www.desmos.com/calculator/cefigtqgbw
// TODO(chowie): Mod to triangle wave.
// NOTE(chowie): As 0 -> 1, at 0.5f to be at 1.
inline f32
Triangle01(f32 t)
{
    f32 Result = 2.0f*t;
    if(Result > 1.0f)
    {
        Result = 2.0f - Result;
    }

    return(Result);
}

// RESOURCE(longtran): https://handmade.network/forums/t/8776-best_way_to_generate_lerp_function
// RESOURCE(fabian): https://fgiesen.wordpress.com/2012/08/15/linear-interpolation-past-present-and-future/
// NOTE(chowie): Can be also represented as "A + T*(B - A)" or "(1.0f - t)*A + t*B"
inline f32
Lerp(f32 A, f32 t, f32 B)
{
    __m128 LerpT = _mm_set_ss(t);
    __m128 LerpA = _mm_set_ss(A);

    f32 Result = _mm_cvtss_f32(_mm_fmadd_ss(LerpT, _mm_set_ss(B), _mm_fnmsub_ss(LerpT, LerpA, LerpA)));
    return(Result);
}

// RESOURCE(freya): https://www.gamedev.net/articles/programming/general-and-gameplay-programming/inverse-lerp-a-super-useful-yet-often-overlooked-function-r5230/
// NOTE(chowie): Returns t based on value between A and B without
// needing to normalise. E.g. explosion or sound based on distance,
// hue-shifting water depth
inline f32
InvLerp(f32 A, f32 Value, f32 B)
{
    f32 Result = (Value - A)/(B - A);
    return(Result);
}

inline f32
LerpRemap(v2 A, f32 Value, v2 B)
{
    f32 t = InvLerp(A.Min, Value, A.Max);
    f32 Result = Lerp(B.Min, t, B.Max);
    return(Result);
}

// RESOURCE(): https://theorangeduck.com/page/scalar-velocity
// NOTE(chowie): For scaling between two values. Eerp requires mult,
// div, pow. Compared to lerp (for positions), it requires add, sub,
// mult. Slerp (for rotations) requires angles.
inline f32
Eerp(f32 A, f32 Value, f32 B)
{
    f32 Result = Exp(Lerp(Log(A), Value, Log(B)));
    return(Result);
}

inline f32
Clamp(f32 Min, f32 Value, f32 Max)
{
    f32 Result = Value;

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
inline f32
Clamp01(f32 Value)
{
    f32 Result = Clamp(0.0f, Value, 1.0f);
    return(Result);
}

// NOTE(chowie): A barycentricafier
inline f32
Clamp01MapToRange(f32 Min, f32 t, f32 Max)
{
    f32 Result = 0.0f;

    f32 Range = Max - Min;
    if(Range != 0.0f)
    {
        // STUDY(chowie): Normalises t bases around min, produce between 0 to
        // range, divide by range changes to 0 - 1
        Result = Clamp01((t - Min) / Range);
    }

    return(Result);
}

inline f32
ClampAboveZero(f32 Value)
{
    f32 Result = (Value < 0) ? 0.0f : Value;
    return(Result);
}

// STUDY(chowie): Inverval inclusion, 1D rect if point in between two bounds
inline b32x
IsInRange(f32 Min, f32 Value, f32 Max)
{
    b32x Result = ((Min <= Value) &&
                   (Value <= Max));
    return(Result);
}

// NOTE(chowie): It could be rewritten as "Square(Result) * (3.0f - 2.0f*Result)"
// RESOURCE: https://handmade.network/p/64/geometer/blog/p/3048-1_year_of_geometer_-_lessons_learnt
// NOTE(chowie): Smooth curve for blending (lerp)
inline f32
SmoothStep(f32 t)
{
    f32 Result = DifferenceOfProducts(3.0f, Square(t), 2.0f, Cube(t)); /* 3t^2 - 2t^3 */
    return(Result);
}

// RESOURCE(inigo quilez): https://iquilezles.org/articles/smoothstepintegral/
// NOTE(chowie): Also called EaseInOut = Cubic curve
// RESOURCE: https://realtimecollisiondetection.net/blog/?p=95
// RESOURCE: https://www.fundza.com/rman_shaders/smoothstep/
// TODO(chowie): Test smooth-step and pulse for alpha tests?
// NOTE(chowie): Smooth curve for blending (lerp)
inline f32
SmoothStep(f32 Min, f32 t, f32 Max)
{
    f32 Clamp = Clamp01MapToRange(Min, t, Max);
    f32 Result = SmoothStep(Clamp);
    return(Result);
}

// RESOURCE(): https://iradicator.com/fast-inverse-smoothstep/
// TODO(chowie): Find which one matches me!
// NOTE(chowie): Hard edge for blending (lerp)
inline f32
Step(f32 t, f32 Value)
{
    f32 Result = (f32)(Value >= t);
    return(Result);
}

inline f32
SquarePulse(f32 A, f32 B, f32 Value)
{
    f32 Result = Step(A, Value) - Step(B, Value);
    return(Result);
}

// NOTE(chowie): Converted from "Clamp01(A.x*t + B.x) - Clamp01(A.y*t + B.y)"
inline f32
TrianglePulse(v2 A, f32 t, v2 B)
{
    f32 Result = Clamp01(Fma(A.x, t, B.x)) - Clamp01(Fma(A.y, t, B.y));
    return(Result);
}

// RESOURCE(inigo quilez): https://iquilezles.org/articles/functions/
// TODO(chowie): Implement these remapping functions! Can be used for animations too

//
// NOTE(chowie): v2u / v2s operations
//

inline v2u
operator+(v2u A, v2u B)
{
    v2u Result = {A.x + B.x, A.y + B.y};
    return(Result);
}

inline v2s
operator+(v2s A, v2s B)
{
    v2s Result = {A.x + B.x, A.y + B.y};
    return(Result);
}

inline v2s
operator*(s32 A, v2s B)
{
    v2s Result = {A*B.x, A*B.y};
    return(Result);
}

inline v2s
operator-(v2s A, v2s B)
{
    v2s Result = {A.x - B.x, A.y - B.y};
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

inline v2u
operator*(u32 A, v2u B)
{
    v2u Result = {A*B.x, A*B.y};
    return(Result);
}

inline v2u
operator-(v2u A, v2u B)
{
    v2u Result = {A.x - B.x, A.y - B.y};
    return(Result);
}

inline v2u &
operator+=(v2u &A, v2u B)
{
    A = A + B;
    return(A);
}

inline v2u
Hadamard(v2u A, v2u B)
{
    v2u Result = {A.x*B.x, A.y*B.y};
    return(Result);
}

inline b32x
AreEqual(v2s A, v2s B)
{
    b32x Result = ((A.x == B.x) &&
                   (A.y == B.y));
    return Result;
}

inline b32x
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

// RESOURCE(chowie): https://endesga.xyz/?page=vector
inline v2
Invert(v2 A)
{
    v2 Result = {-A.x, -A.y};
    return(Result);
}

// NOTE(chowie): A single multiplier
inline v2
operator*(f32 A, v2 B)
{
    v2 Result = {A*B.x, A*B.y};
    return(Result);
}

// NOTE(chowie): 5*Vector or Vector*5 is valid syntax
inline v2
operator*(v2 B, f32 A)
{
    v2 Result = A*B;
    return(Result);
}

// NOTE(chowie): This is hadamard
inline v2
operator*(v2 A, v2 B)
{
    v2 Result = {A.x*B.x, A.y*B.y};
    return(Result);
}

// NOTE(chowie): Calls operator*
inline v2 &
operator*=(v2 &B, f32 A)
{
    B = A*B;
    return(B);
}

inline v2
operator/(v2 B, f32 A)
{
    v2 Result = (1.0f/A)*B;
    return(Result);
}

inline v2
operator/(f32 B, v2 A)
{
    v2 Result = {B / A.x, B / A.y};
    return(Result);
}

inline v2
operator/(v2 A, v2 B)
{
    v2 Result = {A.x/B.x, A.y/B.y};
    return(Result);
}

inline v2 &
operator/=(v2 &B, f32 A)
{
    B = B / A;
    return(B);
}

// NOTE(chowie): B = -A is valid syntax
inline v2
operator-(v2 A)
{
    v2 Result = {-A.x, -A.y};
    return(Result);
}

// NOTE(chowie): A + B is valid syntax
inline v2
operator+(v2 A, v2 B)
{
    v2 Result = {A.x + B.x, A.y + B.y};
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
    v2 Result = {A.x - B.x, A.y - B.y};
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
   value is positive they are pointing in the same general direction,
   if the value is negative they are pointing in opposite general
   direction, if the value is 0 they are perpendicular.

   If two vectors are parallel, 2d or 3d. Dot product will equal to
   their length, "atb = |a||b|cos0" > "atb = |a||b|" >
   "ax*bx + ay*by = sqrt(ax^2+ay^2)*sqrt(bx^2+by^2)"
*/
// A.x*B.x + A.y*B.y
inline f32
Inner(v2 A, v2 B)
{
    f32 Result = SumOfProducts(A.x, B.x, A.y, B.y);
    return(Result);
}

// NOTE(chowie): Also called Magnitude
inline f32
LengthSq(v2 A)
{
    f32 Result = Inner(A, A);
    return(Result);
}

// STUDY(chowie): Sqrt is undefined negative numbers. LengthSq
// cannot produce a negative number, i.e Negative*Negative is positive
// NOTE(chowie): Also called "norm"
inline f32
Length(v2 A)
{
    f32 Result = SquareRoot(LengthSq(A));
    return(Result);
}

inline v2
NOZ(v2 A)
{
    v2 Result = {};

    f32 LenSq = LengthSq(A);
    if(LenSq > Square(Epsilon32))
    {
        Result = A*InvSquareRoot(LenSq);
    }

    return(Result);
}

inline v2
Clamp01(v2 Value)
{
    v2 Result = {Clamp01(Value.x), Clamp01(Value.y)};
    return(Result);
}

inline v2
Lerp(v2 A, f32 t, v2 B)
{
    v2 Result = {Lerp(A.x, t, B.x), Lerp(A.y, t, B.y)};
    return(Result);
}

// RESOURCE(HmH): On Differential Geometry (gradient, divergence and
// curl) & Geometric Algebra (Dot and Cross) being old school
// https://guide.handmadehero.org/code/day376/#2703

// RESOURCE(stijn oomes): https://stijnoomes.com/laws-of-rational-trigonometry/
// RESOURCE(alexander bogomolny): https://www.cut-the-knot.org/pythagoras/RationalTrig/CutTheKnot.shtml
// RESOURCE(): https://news.ycombinator.com/item?id=34299550

//
// NOTE(chowie): Rational Trig v2 operations
//

inline v2
operator*(m2x2 A, v2 P)
{
    v2 Result =
    {
        Inner(A.RowA, P), // A.E[0][0]*P.x + A.E[0][1]*P.y;
        Inner(A.RowB, P), // A.E[1][0]*P.x + A.E[1][1]*P.y;
    };

    return(Result);
}

// RESOURCE(): Identity matrix plays a role - https://blog.demofox.org/2020/02/09/a-fun-2d-rotation-matrix-derivation/
// RESOURCE(ratchetfreak): https://hero.handmade.network/forums/code-discussion/t/1018-2d_rotation_help
// TODO(chowie): General rotation for other shapes other than circles?
inline m2x2
V2ToM2x2Rotation(v2 P)
{
    m2x2 Result =
    {
        {{P.x, -P.y},
         {P.y, P.x}}
    };

    return(Result);
}

// COULDDO(chowie): Good for pentagon/hexagon tiling for SFX?
// RESOURCE(petrik clarberg): Fast equal-area mapping of (hemi)sphere
// using SIMD by 2008, mentioned the sin approx I used below!
// TODO(chowie): Pass starting angle. Average the triangle count too
// to not look super detailed when rescaling?
// NOTE(chowie): Works best for (n >= 5). Better accuracy / less
// underdraw as n increases. If n is high enough, circle overdraws.
// NOTE(chowie): Pythag for cos is more accurate than small-angle approx
// NOTE(chowie): Custom magic number Pow comparable to std:pow. Better
// than nieve; accuracy close to sinf (n = 4 just short to close diamond; sinf no problem)
// & similar no. instructions Nieve: 5x mult vs Magic Pow: 1x FMA, 1x sub, 1x cast
// RESOURCE(basesandframes): https://basesandframes.wordpress.com/2020/04/04/even-faster-math-functions/
// NOTE(chowie): Approx had indistinguishable results to -x^3/6 +x^5/120,
// improved by using mult rather than division.
// TODO(chowie): See when using sin, use this for the best accuracy!
// Pow 7 seems to be less accurate compared to stopping at Pow 5?
// -(Cube(Sector)*0.1666666635050979363f) + (Pow(Sector, 5)*0.0083332964926239037349f) - (Pow(Sector, 7)*0.00019777591579068317932f);
internal v2
RotationByCircleSector(f32 n, f32 Circumference)
{
    // RESOURCE(john d cook): https://www.johndcook.com/blog/2010/07/27/sine-approximation-for-small-x/
    f32 Sector = (Circumference / n);
    f32 Error = -(Cube(Sector)*0.16667131080141501286f) + (Pow(Sector, 5)*0.008380604124044387360f);

    v2 Result;
    Result.y = Sector + Error;
    Result.x = SquareRoot(1.0f - Square(Result.y));

    return(Result);
}

// RESOURCE(arnon): https://github.com/HardCoreCodin/Rational-Ray-Casting/blob/master/raycasting-c/src/main.c
internal v2
RotationByGradient(f32 m)
{
    // NOTE(by arnon): Project a point on a unit circle from a
    // position on a vertical line of "x = 1" towards the origin
    f32 mSq = Square(m);
    f32 Factor = 1.0f / (1.0f + mSq);

    v2 Result;
    Result.x = (1.0f - mSq)*Factor;
    Result.y = (2.0f*m)*Factor;

    return(Result);
}

// TODO(chowie): Use for a radial menu?
inline m2x2
M2x2RotationByTris(f32 n, f32 Circumference)
{
    v2 Rot = RotationByCircleSector(n, Circumference);
    m2x2 Result = V2ToM2x2Rotation(Rot);

    return(Result);
}

/*
internal v2
NormalisedRotationByCircleSector(f32 n)
{
    // IMPORTANT(chowie): An approximation where there's many
    // segments, so sin can be dropped. Might leave small gaps.
    f32 s3 = (Tau32 / n);
    //f32 s3 = (Sin(Tau32 / n)); // NOTE(chowie): Change to this for more accuracy

    // NOTE(chowie): Cross Law
    f32 Gradient = s3 / (1.0f - SquareRoot(1.0f - Square(s3)));
    //f32 Gradient = SquareRoot(s3 / (2 - SquareRoot(4*(1 - s3)) - s3));

    // NOTE(by arnon): Project a point on a unit circle from a
    // position on a vertical line of "x = 1" towards the origin
    // RESOURCE(NJ Wildberger): Rational Param of Circle - https://www.youtube.com/watch?v=Ui8OvmzDn7o
    f32 mSq = Square(Gradient);
    f32 Factor = 1.0f / (1.0f + mSq);

    v2 Result = {};
    Result.x = (mSq - 1.0f)*Factor; // NOTE(chowie): Flipped to +tive since gradient is > 1 for angles less than 45 deg
    Result.y = (2.0f*Gradient)*Factor;

    return(Result);
}
*/

// RESOURCE(inigo iquillez): https://iquilezles.org/articles/noacos/
// TODO(chowie): Aligning one vector to another -> to rotate one thing around another
inline m3x3
M3x3RotationByAlign(v3 Source, v3 Dest)
{
}

inline v2
RationalDirection(v2 From, v2 To)
{
    v2 Result = To - From;
    return(Result);
}

//
// NOTE(chowie): v3u / v3s operations
//

inline v3u
operator+(v3u A, v3u B)
{
    v3u Result = {A.x + B.x, A.y + B.y, A.z + B.z};
    return(Result);
}

inline b32x
AreEqual(v3u A, v3u B)
{
    b32x Result = ((A.x == B.x) &&
                   (A.y == B.y) &&
                   (A.z == B.z));
    return(Result);
}

inline v3s
operator+(v3s A, v3s B)
{
    v3s Result = {A.x + B.x, A.y + B.y, A.z + B.z};
    return(Result);
}

// NOTE(chowie): This is hadamard
inline v3s
operator*(v3s A, v3s B)
{
    v3s Result = {A.x*B.x, A.y*B.y, A.z*B.z};
    return(Result);
}

inline b32x
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

// TODO(chowie): For stepping in a grid via a ray intersection test
// NOTE(chowie): Outputs 1 +tive, -1 -tive
inline v3
SignOfDir(v3 A)
{
    v3 Result = {SignOf(A.x), SignOf(A.y), SignOf(A.z)};
    return(Result);
}

// RESOURCE(ken whatmough): https://math.stackexchange.com/questions/137362/how-to-find-perpendicular-vector-to-another-vector/4112622#4112622
// NOTE(by ken whatmough): Sqrt free, trig free, abs free, branchless!
// "This solution has the added bonus that the magnitude of the result
// is within a factor of Sqrt(2) of the magnitude of the input."
inline v3
Perp(v3 A)
{
    v3 Result =
    {
        CopySign(A.z, A.x),
        CopySign(A.z, A.y),
        -CopySign(A.x, A.z) - CopySign(A.y, A.z)
    };

    return(Result);
}

// TODO(chowie): Compare vs the above - seems superior! For bases? Camera projections?
// RESOURCE(sam): https://web.archive.org/web/20210725181210/https://lolengine.net/blog/2013/9
// NOTE(chowie): "Given a non-zero vector v in 3D space, find a
// non-zero vector w orthogonal to v (i.e. such that v.w = 0)."
// = Always works if the input is non-zero.
// = Doesn’t require the input to be normalised.
// = Doesn’t normalise the output.
/*
inline v3
GeneratePerp(v3 A)
{
    v3 Result = (AbsoluteValue(A.x) > AbsoluteValue(A.z))
        ? V3(-A.y, A.x, 0.0) : V3(0.0, -A.z, A.y);
    return(Result);
}
*/

inline v3
Invert(v3 A)
{
    v3 Result = {-A.x, -A.y, -A.z};
    return(Result);
}

// NOTE(chowie): A single multiplier
inline v3
operator*(f32 A, v3 B)
{
    v3 Result = {A*B.x, A*B.y, A*B.z};
    return(Result);
}

// NOTE(chowie): 5*Vector or Vector*5 is valid syntax
inline v3
operator*(v3 B, f32 A)
{
    v3 Result = A*B;
    return(Result);
}

// NOTE(chowie): This is hadamard
inline v3
operator*(v3 A, v3 B)
{
    v3 Result = {A.x*B.x, A.y*B.y, A.z*B.z};
    return(Result);
}

// NOTE(chowie): Calls operator*
inline v3 &
operator*=(v3 &B, f32 A)
{
    B = A*B;
    return(B);
}

inline v3
operator/(v3 B, f32 A)
{
    v3 Result = (1.0f/A)*B;
    return(Result);
}

inline v3
operator/(f32 B, v3 A)
{
    v3 Result = {B / A.x, B / A.y, B / A.z};
    return(Result);
}

inline v3
operator/(v3 A, v3 B)
{
    v3 Result = {A.x/B.x, A.y/B.y, A.z/B.z};
    return(Result);
}

inline v3 &
operator/=(v3 &B, f32 A)
{
    B = B / A;
    return(B);
}

// NOTE(chowie): B = -A is valid syntax
inline v3
operator-(v3 A)
{
    v3 Result = {-A.x, -A.y, -A.z};
    return(Result);
}

// NOTE(chowie): A + B is valid syntax
inline v3
operator+(v3 A, v3 B)
{
    v3 Result = {A.x + B.x, A.y + B.y, A.z + B.z};
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
    v3 Result = {A.x - B.x, A.y - B.y, A.z - B.z};
    return(Result);
}

// NOTE(chowie): Calls the - operator on itself
inline v3 &
operator-=(v3 &A, v3 B)
{
    A = A - B;
    return(A);
}

// NOTE(chowie): Difference between hadamard and inner product is
// that the terms are not summed
inline v3
Hadamard(v3 A, v3 B)
{
    v3 Result = {A.x*B.x, A.y*B.y, A.z*B.z};
    return(Result);
}

// TODO(chowie): Is there a better way? Fma(A.x, B.x, Fma(A.y, B.y, A.z*B.z))?
// A.x*B.x + A.y*B.y + A.z*B.z
inline f32
Inner(v3 A, v3 B)
{
    f32 Result = Fma(A.z, B.z, SumOfProducts(A.x, B.x, A.y, B.y));
    return(Result);
}

inline v3
Cross(v3 A, v3 B)
{
    v3 Result =
    {
        DifferenceOfProducts(A.y, B.z, A.z, B.y), // A.y*B.z - A.z*B.y,
        DifferenceOfProducts(A.z, B.x, A.x, B.z), // A.z*B.x - A.x*B.z,
        DifferenceOfProducts(A.x, B.y, A.y, B.x), // A.x*B.y - A.y*B.x,
    };

    return(Result);
}

inline f32
LengthSq(v3 A)
{
    f32 Result = Inner(A, A);
    return(Result);
}

// STUDY(chowie): Sqrt is undefined negative numbers. LengthSq
// cannot produce a negative number, i.e Negative*Negative is positive
// NOTE(chowie): Also called "norm"
inline f32
Length(v3 A)
{
    f32 Result = SquareRoot(LengthSq(A));
    return(Result);
}

// RESOURCE(owl): https://fastcpp.blogspot.com/2012/02/calculating-length-of-3d-vector-using.html
// TODO(chowie): Convert to SSE4?
inline v3
Normalise(v3 A)
{
    f32 LenSq = LengthSq(A);
    v3 Result = A*InvSquareRoot(LenSq);
    return(Result);
}

// STUDY: E.g. asks if moving in this direction is roughly the same
// direction as the head, Cos0. This will be scaled by the distance;
// it would get higher as it gets further, but we would like the
// opposite. 1 if directly pointed in the same direction, -1 if
// opposite. 0 if perp.
// NOTE(chowie): Normalise _or_ 0. Automatic epsilon check, becomes a
// zero vector. For Inner Product
// NOTE(chowie): Use this if you don't know how long the line could
// be, and if degerate!
inline v3
NOZ(v3 A)
{
    v3 Result = {};

    f32 LenSq = LengthSq(A);
    if(LenSq > Square(Epsilon32))
    {
        Result = A*InvSquareRoot(LenSq);
    }

    return(Result);
}

// NOTE(chowie): Normalise _or_ Unit Vector.
// TODO(chowie): Piecewise multiplication/hadamard for scaling?
inline v3
NOU(v3 A)
{
    v3 Result = V3(1, 1, 1);

    f32 LenSq = LengthSq(A);
    if(AbsoluteValue(LenSq - 1.0f) > Square(Epsilon32))
    {
        Result = A*InvSquareRoot(LenSq);
    }

    return(Result);
}

// RESOURCE(juckett): https://www.ryanjuckett.com/biarc-interpolation/
// TODO: Try biarc interpolation?
// TODO(chowie): Not quite sure how useful this function is - for asserts mainly
// NOTE(chowie): Vector length is within epsilon of 1
inline b32x
IsNormalisedEps(v3 A, f32 Epsilon)
{
    f32 Magnitude = LengthSq(A);
    b32x Result = (Magnitude >= Square(1.0f - Epsilon) &&
                  Magnitude <= Square(1.0f + Epsilon));
    return(Result);
}

// RESOURCE(gaul): https://web.archive.org/web/20200414122444/https://www.randygaul.net/2014/11/
// RESOURCE(gaul): https://www.reddit.com/r/gamedev/comments/2ljvlz/collection_of_numerically_robust_parallel_vectors/
// TODO(chowie): Double Check this!
inline b32x
IsPerp(v3 A, v3 B, f32 Epsilon)
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
IsParallel(v3 A, v3 B, f32 Epsilon)
{
    A = Normalise(A);
    B = Normalise(B);
    b32x Result = ((1.0f - AbsoluteValue(Inner(A, B))) < Epsilon);

    return(Result);
}

inline v3
Floor(v3 Value)
{
    v3 Result = {Floor(Value.x), Floor(Value.y), Floor(Value.z)};
    return(Result);
}

inline v3
Round(v3 Value)
{
    v3 Result = {Round(Value.x), Round(Value.y), Round(Value.z)};
    return(Result);
}

inline v3
Clamp01(v3 Value)
{
    v3 Result = {Clamp01(Value.x), Clamp01(Value.y), Clamp01(Value.z)};
    return(Result);
}

inline v3
Lerp(v3 A, f32 t, v3 B)
{
    v3 Result = {Lerp(A.x, t, B.x), Lerp(A.y, t, B.y), Lerp(A.z, t, B.z)};
    return(Result);
}

/*
  RESOURCE(jblow): http://number-none.com/product/Understanding%20Slerp,%20Then%20Not%20Using%20It/

                      | Commutative | Constant Velocity | Torque-Minimal |
  Quaternion slerp    |     No!     |       Yes         |      Yes       |
  Quaternion nlerp    |     Yes     |       No!         |      No!       |
  Log-Quaternion lerp |     Yes     |       Yes         |      Yes       |
*/

// RESOURCE(maggio): https://keithmaggio.wordpress.com/2011/02/15/math-magician-lerp-slerp-and-nlerp/
// STUDY(chowie): For animation
// TODO(chowie): V2?
inline v3
NLerp(v3 A, f32 t, v3 B)
{
    v3 Result = Normalise(Lerp(A, t, B));
    return(Result);
}

// RESOURCE(casey): https://hero.handmade.network/forums/code-discussion/t/1089-quaternion_interpolation
// NOTE(chowie): The reason why you wouldn't use slerp, isn't that
// it's slow. But it cannot handle more than two quarternions.
// RESOURCE(demofox): https://blog.demofox.org/2016/02/19/normalized-vector-interpolation-tldr/
// TODO(chowie): No trig slerp!
/*
inline v3
SLerp(v3 A, f32 t, v3 B)
{
    f32 Dot = Inner(A, B);
    Dot = Clamp(-1.0f, Dot, 1.0f);
    f32 Theta = acosf(Dot)*t;
    v3 Relative = Normalise(B - A*Dot);
    v3 Result = ((A*Cos(Theta)) + (Relative*Sin(Theta)));
    return(Result);
}
*/

struct get_basis_result
{
    v3 BasisB;
    v3 BasisC;
};
// RESOURCE: https://box2d.org/posts/2014/02/computing-a-basis/
// RESOURCE: https://web.archive.org/web/20190120215637/http://www.randygaul.net/2015/10/27/compute-basis-with-simd/
// TODO(chowie): Find out if this can be a replacement for entities? If so, SIMD this?
inline get_basis_result
GetBasis(v3 A, v3 B, v3 C)
{
    get_basis_result Result = {};

    // NOTE(): Suppose vector a has all equal components and is a unit vector:
    // a = (s, s, s)
    // Then 3*s*s = 1, s = sqrt(1/3) = 0.57735. This means that at
    // least one component of a unit vector must be greater or equal
    // to 0.57735.
#define SqrtThird 0.57735f
    if(AbsoluteValue(A.x) >= SqrtThird)
    {
        B = V3(A.y, -A.x, 0.0f);
    }
    else
    {
        B = V3(0.0f, A.z, -A.y);
    }

    Result.BasisB = Normalise(B);
    Result.BasisC = Cross(A, B);

    return(Result);
}

// RESOURCE(inigo quilez): https://iquilezles.org/articles/dontflip/
// TODO(chowie): Use for collision sliding on walls and collision bounces
inline v3
Reflect(v3 A, v3 B)
{
    f32 Coeff = Inner(A, B);
    v3 Result = (Coeff > 0.0f) ? A : (A - 2.0f*B*Coeff);
    return(Result);
}

// NOTE(chowie): Snaps to the half-plane
inline v3
Clip(v3 A, v3 B)
{
    f32 Coeff = Inner(A, B);
    v3 Result = (Coeff > 0.0f) ? A :
        ((A - B*Coeff)*InvSquareRoot(1.0f - Square(Coeff) / Inner(A, A)));
    return(Result);
}

// NOTE(chowie): Doesn't preserve length, must normalise after clipping
inline v3
ClipNoLength(v3 A, v3 B)
{
    f32 Coeff = Inner(A, B);
    v3 Result = (Coeff > 0.0f) ? A : (A - B*Coeff);
    return(Result);
}

//
// NOTE(chowie): v4 operations
//

inline v4u
operator+(v4u A, v4u B)
{
    v4u Result = {A.x + B.x, A.y + B.y, A.z + B.z, A.w + B.w};
    return(Result);
}

// NOTE(chowie): This is hadamard
inline v4u
operator*(v4u A, v4u B)
{
    v4u Result = {A.x*B.x, A.y*B.y, A.z*B.z, A.w*B.w};
    return(Result);
}

inline b32x
AreEqual(v4u A, v4u B)
{
    b32x Result = ((A.x == B.x) &&
                   (A.y == B.y) &&
                   (A.z == B.z) &&
                   (A.w == B.w));
    return(Result);
}

// NOTE(chowie): A single multiplier
inline v4u
operator*(u32 A, v4u B)
{
    v4u Result = {A*B.x, A*B.y, A*B.z, A*B.w};
    return(Result);
}

// NOTE(chowie): 5*Vector or Vector*5 is valid syntax
inline v4u
operator*(v4u B, u32 A)
{
    v4u Result = A*B;
    return(Result);
}

// NOTE(chowie): Calls operator*
inline v4u &
operator*=(v4u &B, u32 A)
{
    B = A*B;
    return(B);
}

// NOTE(chowie): A single multiplier
inline v4
operator*(f32 A, v4 B)
{
    v4 Result = {A*B.x, A*B.y, A*B.z, A*B.w};
    return(Result);
}

// NOTE(chowie): 5*Vector or Vector*5 is valid syntax
inline v4
operator*(v4 B, f32 A)
{
    v4 Result = A*B;
    return(Result);
}

// NOTE(chowie): Calls operator*
inline v4 &
operator*=(v4 &B, f32 A)
{
    B = A*B;
    return(B);
}

// NOTE(chowie): B = -A is valid syntax
inline v4
operator-(v4 A)
{
    v4 Result = {-A.x, -A.y, -A.z, -A.w};
    return(Result);
}

// NOTE(chowie): A + B is valid syntax
inline v4
operator+(v4 A, v4 B)
{
    v4 Result = {A.x + B.x, A.y + B.y, A.z + B.z, A.w + B.w};
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
    v4 Result = {A.x - B.x, A.y - B.y, A.z - B.z, A.w - B.w};
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

// A.x*B.x + A.y*B.y + A.z*B.z + A.w*B.w
inline f32
Inner(v4 A, v4 B)
{
    f32 Result = SumOfProducts(A.x, B.x, A.y, A.y) + SumOfProducts(A.z, B.z, A.w, B.w);
    return(Result);
}

inline f32
LengthSq(v4 A)
{
    f32 Result = Inner(A, A);
    return(Result);
}

// STUDY(chowie): Sqroot is undefined negative numbers. LengthSq
// cannot produce a negative number, i.e Negative*Negative is positive
inline f32
Length(v4 A)
{
    f32 Result = SquareRoot(LengthSq(A));
    return(Result);
}

// RESOURCE(owl): https://fastcpp.blogspot.com/2012/02/calculating-length-of-3d-vector-using.html
// TODO(chowie): Convert to SSE4?
inline v4
Normalise(v4 A)
{
    f32 LenSq = LengthSq(A);
    v4 Result = A*InvSquareRoot(LenSq);
    return(Result);
}

inline v4
Clamp01(v4 Value)
{
    v4 Result = {Clamp01(Value.x), Clamp01(Value.y), Clamp01(Value.z), Clamp01(Value.w)};
    return(Result);
}

inline v4
Lerp(v4 A, f32 t, v4 B)
{
    v4 Result = {Lerp(A.x, t, B.x), Lerp(A.y, t, B.y), Lerp(A.z, t, B.z), Lerp(A.w, t, B.w)};
    return(Result);
}

// RESOURCE(Ameye): https://ameye.dev/notes/rendering-outlines/
// TODO(chowie): Figure out which outlines might be the best!
inline f32
FresnelSphericalOutline(v4 NormalisedNormal, v4 NormalisedView, f32 Power)
{
    f32 Result = Pow((1.0f - Clamp01(Inner(NormalisedNormal, NormalisedView))), Power);
    return(Result);
}

//
// NOTE(chowie): Rotors
//

// RESOURCE(): https://marctenbosch.com/quaternions/
// NOTE(chowie): Small benefits of rotors over quaternions:
// 1) Rotors work in n dimensions, helpful for transitioning from 3D
// to 2D rotations with less effort (only need bivector, x^y)
// 2) Geometric Algebra is conceptually easier to understand for the
// average person
// 3) More physics-based applications care about spinors
// 4) Slerp simplification to Exp and Log, don't need angles

// RESOURCE(endesga): https://endesga.xyz/?page=main
// STUDY(chowie): Hangle is just the scalar! NOTE(chowie): Sometimes
// people for a FPS camera tie the Euclid angle is convenient avoiding
// converting between quaternion and Euclid.

// RESOURCE(endesga): https://endesga.xyz/?page=rotor
// TODO(chowie): Compare the rotor implementation

// RESOURCE(): Nothing here yet - https://math.stackexchange.com/questions/4809683/how-to-derive-the-log-and-exp-functions-of-a-geometric-algebra-rotor-for-spheric

// RESOURCE(kromenaker): http://clarkkromenaker.com/post/gengine-09-quaternions/

// NOTE(chowie): Generic abstraction of a cross product
// NOTE(chowie): Also called outer product. Rotations can be thought of the as an "offset" in the axis
//    |                         | offset in x*x-axis, and y*y-axis
//  \ | /                       ^
// __\|/___  -- rotates --> ____|-->___
//    |                         |
//    |                         |
inline v3
Wedge(v3 A, v3 B)
{
    v3 Result =
    {
        DifferenceOfProducts(A.x, B.y, A.y, B.x), // A.x*B.y - A.y*B.x,
        DifferenceOfProducts(A.x, B.z, A.z, B.x), // A.x*B.z - A.z*B.x,
        DifferenceOfProducts(A.y, B.z, A.z, B.y), // A.y*B.z - A.z*B.y,
    };

    return(Result);
}

// NOTE(chowie): Generic abstraction of a quarternion
// NOTE(from marc): Construct the rotor that rotates one unit vector
// to another uses the usual trick to get the half angle
inline v4
FromToRotor(v3 From, v3 To)
{
    v4 Result;
    Result.s = 1.0f + Inner(To, From); // NOTE(chowie): Halfway normalise
    Result.bxyyzzx = Wedge(To, From); // NOTE(from marc): The left side of the products have b a, not a b, so flip
    Result = Normalise(Result);
    return(Result);
}

// TODO(chowie): IMPORTANT(chowie): Why did everyone else's rotors not
// have a dedicated yaw, roll, pitch func than endesga's & Casey's????
// Could the bivector that represents 2 of the axis cover that
// already? See "Axis-angle representation for rotors" on
// jacquesheunis?

// NOTE(chowie): Eq works regardless of the coord handedness!
// NOTE(from marc): Angle+plane, plane must be normalized
// NOTE(from marc): Angle must be in radians
inline v4
RotorPlane(v3 Plane, f32 AngleRadians)
{
    v4 Result;
    f32 SinAngle = Sin(AngleRadians / 2.0f);
    Result.s = Cos(AngleRadians / 2.0f);
    Result.bxyyzzx = -SinAngle*Plane; // NOTE(chowie): The left side of the products have b a, not a b
    return(Result);
}

// NOTE(chowie): Geometric product -> produces twice angle, -tive direction
inline v4
Geo(v3 A, v3 B)
{
    v4 Result = {Wedge(A, B), Inner(A, B)};
    return(Result);
}

// RESOURCE(): https://hero.handmade.network/forums/code-discussion/t/2305-dot_product_and_cross_product_as_quaternions
// RESOURCE(): https://and-what-happened.blogspot.com/2013/03/basic-linear-algebra-for-3d-graphics.html?view=sidebar
// STUDY(chowie): Rotor/Quaternion can be made by cross + inner/dot product
// TODO(chowie): Is this the correct order? I'm assuming A = P, B = Q
// TODO(chowie): Marc Ten Bosch suggests this can be optimised
// TODO(chowie): Replace with geometric product?
// NOTE(chowie): Rotor product
inline v4
operator*(v4 A, v4 B)
{
    v4 Result =
        {
            A.bxyyzzx*B.s + A.s*B.bxyyzzx + Cross(A.bxyyzzx, B.bxyyzzx), // TODO(chowie): SumOfProd?
            A.s*B.s - Inner(A, B),
        };

    return(Result);
}

// NOTE(chowie): Calls operator*
inline v4 &
operator*=(v4 &B, v4 A)
{
    B = A*B;
    return(B);
}

// NOTE(chowie): Vector rotation > multiply by quaternion > multiply
// inverse = sandwich product
// TODO(chowie): Is this the correct order? I'm assuming A = P, B = X
// TODO(chowie): Marc Ten Bosch suggests this can be Optimised
// STUDY(chowie): For an orbiting camera's "azimuth", think Wii sports orbiting camera for a stadium
inline v3
RotateRotor(v4 A, v3 B)
{
    // NOTE(chowie): Q = A B
    v3 Q =
        {
            A.s*B.x + B.y*A.bxy + B.z*A.byz,
            A.s*B.y - B.x*A.bxy + B.z*A.bzx,
            A.s*B.z - B.x*A.byz - B.y*A.bzx,
        };

    // NOTE(chowie): Trivector part of result is always zero!
    f32 Tri = B.x*A.bzx - B.y*A.byz + B.z*A.bxy;

    // NOTE(chowie): R = Q A*
    v3 Result =
        {
            A.s*Q.x + Q.y*A.bxy + Q.z*A.byz + Tri*A.bzx,
            A.s*Q.y - Q.x*A.bxy - Tri*A.byz + Q.z*A.bzx,
            A.s*Q.z + Tri*A.bxy + Q.x*A.byz - Q.y*A.bzx,
        };

    return(Result);
}

// NOTE(chowie): Same as quarternion conjugate/invert
// NOTE(chowie): Negating only the axis via inversion causes the angle
// to negate = calculates the opposite direction. Looks like 2D perp to me!
inline v4
ReverseRotor(v4 A)
{
    v4 Result = {-A.bxyyzzx, A.s};
    return(Result);
}

// TODO(chowie): The identity quaternion, [(0, 0, 0), 1] is achieved
// by the inverse. Conjugate/LengthSquared = inverse. A unit length
// quaternion is equal to its conjugate!

// NOTE(chowie): Sandwich Product
inline v4
RotateRotorByAnother(v4 A, v4 B)
{
    v4 Result = A*B*ReverseRotor(A);
    return(Result);
}

// STUDY(chowie): HmH 366 2:07:40 - Camera defined by X,Y,Z and P. You
// would defined by Quaternion and P, generate X,Y,Z axis by
// quaternion. Not so difficult because you can turn a quaternion into
// a matrix, thus you can extract the axis from that!
// TODO(chowie): Marc Ten Bosch suggests this can be optimised
inline m3x3
RotorToMatrix(v4 A)
{
    m3x3 Result =
        {
            RotateRotor(A, V3(1, 0, 0)), // STUDY(chowie): Isn't this the identity matrix?
            RotateRotor(A, V3(0, 1, 0)),
            RotateRotor(A, V3(0, 0, 1)),
        };

    return(Result);
}

inline v4
Nlerp(v4 A, f32 t, v4 B)
{
    v4 Result = Normalise(Lerp(A, t, B));
    return(Result);
}

inline v4
Negate(v4 A)
{
    v4 Result = {-A.x, -A.y, -A.z, -A.w};
    return(Result);
}

// RESOURCE(): https://jacquesheunis.com/post/rotors/
inline v4
Slerp(v4 From, f32 t, v4 To)
{
    v4 Result = {};

    f32 Dot = Inner(From, To);
    if(Dot < 0.0f)
    {
        To = Negate(To);
        Dot = -Dot;
    }

    // Avoid numerical stability issues with trig functions when
    // the angle between `From` and `To` is close To zero.
    // Also assumes `From` and `To` both have magnitude 1.
    if(Dot > 0.99995f)
    {
        Result = Nlerp(From, t, To);
    }
    else
    {
        // Assume that `From` and `To` both have magnitude 1
        // (IE they are the product of two unit vecTors)
        // then cos(Theta) = Dot(From, To)
        f32 CosTheta = Dot;

        f32 Theta = acosf(CosTheta);
        f32 FromFactor = sinf((1.0f - t)*Theta)/sinf(Theta);
        f32 ToFactor = sinf(t*Theta)/sinf(Theta);

        Result.bxy = SumOfProducts(FromFactor, From.bxy, ToFactor, To.bxy);
        Result.byz = SumOfProducts(FromFactor, From.byz, ToFactor, To.byz);
        Result.bzx = SumOfProducts(FromFactor, From.bzx, ToFactor, To.bzx);
        Result.s = SumOfProducts(FromFactor, From.s, ToFactor, To.s);
    }

    return(Result);
}

// RESOURCE(): https://www.euclideanspace.com/maths/algebra/clifford/d3/index.htm
// RESOURCE(): https://www.reddit.com/r/gamedev/comments/np27gy/rotations_using_quaternions/
// RESOURCE(): https://imadr.me/rotations-with-quaternions/
inline v4
RotorLog(v4 A)
{
    v3 Normal = Normalise(A.bxyyzzx);
    f32 Magnitude = LengthSq(A);
    f32 Angle = acosf(A.s / Magnitude);

    v4 Result = V4(Normal.x*Angle, Normal.y*Angle, Normal.z*Angle, Log(Magnitude));

    return(Result);
}

inline v4
RotorExp(v4 A)
{
    v3 Normal = Normalise(A.bxyyzzx);
    f32 Magnitude = LengthSq(A.bxyyzzx);
    f32 SinAngle = sinf(Magnitude);
    f32 ExpW = Exp(A.s);

    v4 Result = V4(Normal.x*SinAngle*ExpW, Normal.y*SinAngle*ExpW, Normal.z*SinAngle*ExpW, cosf(Magnitude)*ExpW);

    return(Result);
}

inline v4
RotorSlerp(v4 A, f32 t, v4 B)
{
    // RESOURCE(): https://martin.ankerl.com/2007/02/11/optimized-exponential-functions-for-java/
    // STUDY(chowie): Optimisations Exp(B*Log(A)) = Pow(A, B)
    v4 Delta = ReverseRotor(A)*B;
    v4 Result = A*RotorExp(t*RotorLog(Delta));
    return(Result);
}

// TODO(chowie): Is this wrong? I think these only affects
// bivectors, not scalar. See RotorReverse from Marc Ten Bosch.
inline v4
Log(v4 A)
{
    v4 Result = {};
    Result.bxy = Log(A.bxy);
    Result.byz = Log(A.byz);
    Result.bzx = Log(A.bzx);
    Result.s = A.s;

    return(Result);
}

inline v4
Exp(v4 A)
{
    v4 Result = {};
    Result.bxy = Exp(A.bxy);
    Result.byz = Exp(A.byz);
    Result.bzx = Exp(A.bzx);
    Result.s = A.s;

    return(Result);
}

// TODO(chowie): Really no angles?
// RESOURCE(): https://rastergraphics.wordpress.com/2022/04/12/geometric-algebra-rotor-average-in-closed-form/
inline v4
RotorAverage(v4 A, f32 t, v4 B)
{
    // RESOURCE(): https://martin.ankerl.com/2007/02/11/optimized-exponential-functions-for-java/
    // STUDY(chowie): Optimisations Exp(B*Log(A)) = Pow(A, B)
    v4 Delta = ReverseRotor(A)*B;
    v4 Result = A*Exp(t*Log(Delta));
    return(Result);
}

/*
// RESOURCE(quilez): https://www.shadertoy.com/view/3s33zj
// RESOURCE(weiler): https://github.com/graphitemaster/normals_revisited/
// TODO(chowie): Check this works with rotors
/*
inline m3x3
Adjugate(m4x4 A)
{
    m3x3 Result =
        {
            Cross(A.E[1].xyz, A.E[2].xyz),
            Cross(A.E[2].xyz, A.E[0].xyz),
            Cross(A.E[0].xyz, A.E[1].xyz),
        };
    return(Result);
}
*/

// RESOURCE(endesga): https://www.shadertoy.com/view/M3jBDW
// IMPORTANT(chowie): TODO(chowie): Projection!

//
// NOTE(chowie): Matrix
//

// STUDY(chowie): This is the instructive version, not optimised!
// Compiler should be smart to unroll constants in loops
inline m4x4
operator*(m4x4 A, m4x4 B)
{
    m4x4 Result = {};

    // NOTE(chowie): Rows of A
    for(s32 r = 0;
        r <= 3;
        ++r)
    {
        // NOTE(chowie): Columns of B
        for(s32 c = 0;
            c <= 3;
            ++c)
        {
            // NOTE(chowie): Columns of A, Rows of B
            for(s32 i = 0; // COULDDO(chowie): Pull out A.E[r][i], i = 1 to be optimal
                i <= 3;
                ++i)
            {
                // STUDY(chowie): A is free to have as many col and B as many
                // rows, notice how r(ow) c(ol) are dependent
                // respectively. A.E[r][i] r only exists in
                // "A". B.E[i][c] only exists in "B". i appears in both
                Result.E[r][c] += A.E[r][i]*B.E[i][c];
            }
        }
    }

    return(Result);
}

// IMPORTANT(chowie): STUDY(chowie): This is the instructive version, not optimised!
inline v4
Transform(m4x4 A, v4 P) // STUDY(chowie): Can specify .w if necessary
{
    v4 Result
    {
        P.x*A.E[0][0] + P.y*A.E[0][1] + P.z*A.E[0][2] + P.w*A.E[0][3],
        P.x*A.E[1][0] + P.y*A.E[1][1] + P.z*A.E[1][2] + P.w*A.E[1][3],
        P.x*A.E[2][0] + P.y*A.E[2][1] + P.z*A.E[2][2] + P.w*A.E[2][3],
        P.x*A.E[3][0] + P.y*A.E[3][1] + P.z*A.E[3][2] + P.w*A.E[3][3],
    };

    return(Result);
}

inline v3
operator*(m4x4 A, v3 P)
{
    v3 Result = Transform(A, V4(P, 1.0f)).xyz;
    return(Result);
}

inline v4
operator*(m4x4 A, v4 P)
{
    v4 Result = Transform(A, P);
    return(Result);
}

// IMPORTANT(chowie): STUDY(chowie): This is the instructive version, not optimised!
inline m4x4
Identity(void)
{
    m4x4 Result =
    {
        {{1, 0, 0, 0},
         {0, 1, 0, 0},
         {0, 0, 1, 0},
         {0, 0, 0, 1}}
    };

    return(Result);
}

// IMPORTANT(chowie): STUDY(chowie): This is the instructive version, not optimised!
inline m4x4
XRotation(f32 Angle)
{
    f32 c = Cos(Angle);
    f32 s = Sin(Angle);

    m4x4 Result =
    {
        {{1, 0, 0, 0},
         {0, c,-s, 0},
         {0, s, c, 0},
         {0, 0, 0, 1}}
    };

    return(Result);
}

// IMPORTANT(chowie): STUDY(chowie): This is the instructive version, not optimised!
inline m4x4
YRotation(f32 Angle)
{
    f32 c = Cos(Angle);
    f32 s = Sin(Angle);

    m4x4 Result =
    {
        {{ c, 0, s, 0},
         { 0, 1, 0, 0},
         {-s, 0, c, 0},
         { 0, 0, 0, 1}}
    };

    return(Result);
}

// IMPORTANT(chowie): STUDY(chowie): This is the instructive version, not optimised!
inline m4x4
ZRotation(f32 Angle)
{
    f32 c = Cos(Angle);
    f32 s = Sin(Angle);

    m4x4 Result =
    {
        {{c,-s, 0, 0},
         {s, c, 0, 0},
         {0, 0, 1, 0},
         {0, 0, 0, 1}}
    };

    return(Result);
}

// NOTE(chowie): A transpose op for m4x4 may not be that useful. Since OpenGL
// should handle it. Maybe for smaller matrices or other reasons?
inline m4x4
Transpose(m4x4 A)
{
    m4x4 Result = {};

    for(int j = 0;
        j <= 3;
        ++j)
    {
        for(int i = 0;
            i <= 3;
            ++i)
        {
            Result.E[j][i] = A.E[i][j];
        }
    }

    return(Result);
}

// RESOURCE(): https://unspecified.wordpress.com/2012/06/21/calculating-the-gluperspective-matrix-and-other-opengl-matrix-maths/
// NOTE(chowie): Outputs -1 to 1, not in pixel coords
inline m4x4_inv
PerspectiveProj(f32 AspectWidthOverHeight, f32 FocalLength,
                f32 NearClipPlane, f32 FarClipPlane)
{
    f32 a = 1.0f;
    f32 b = AspectWidthOverHeight; // STUDY(chowie): HmH 363, puts Aspect in {2,2} of the matrix Y-correction instead of {1, 1} X-correction (X requires inversion 1/aspect, but Y doesn't need to)
    f32 c = FocalLength; // STUDY(chowie): People take the Z directly (as -1) and not modify by the focal length

    f32 n = NearClipPlane; // NOTE(chowie): Near clip plane dist = OpenGL map to -1
    f32 f = FarClipPlane; // NOTE(chowie): Far clip plane dist = OpenGL map to 1

    // NOTE(chowie): These are perspective corrected terms, when you divide by -z
    f32 d = (n+f)/(n-f);
    f32 e = (2*f*n) / (n-f);

    m4x4_inv Result =
    {
        // NOTE(chowie): Forward
        {{{a*c,   0,  0, 0},
          {  0, b*c,  0, 0},
          {  0,   0,  d, e},
          {  0,   0, -1, 0}}}, // NOTE(chowie): -1 to get negated, right-handed coord independent

        // NOTE(chowie): Inverse
        {{{1/(a*c),       0,   0,   0},
          {      0, 1/(b*c),   0,   0},
          {      0,       0,   0,  -1},
          {      0,       0, 1/e, d/e}}},
    };

#if RUINENGLASS_SLOW
    m4x4 I = Result.Inverse*Result.Forward;
    v4 Test0 = Result.Forward*V4(0, 0, -n, 1);
    Test0.xyz /= Test0.w;
    v4 Test1 = Result.Forward*V4(0, 0, -f, 1);
    Test1.xyz /= Test1.w;
#endif

    return(Result);
}

// NOTE(chowie): Outputs -1 to 1, not in pixel coords
// TODO(chowie): Pass in the near and far clip plane?
inline m4x4_inv
OrthographicProj(f32 AspectWidthOverHeight,
                 f32 NearClipPlane, f32 FarClipPlane)
{
    f32 a = 1.0f;
    f32 b = AspectWidthOverHeight; // STUDY(chowie): Should take care of width + height, the caller of SetCameraTransform only needs width!

    // STUDY(chowie): Since everything can be proj on a sheet with no
    // foreshortening in orthographic, you can draw in front like behind.
    f32 n = NearClipPlane; // NOTE(chowie): Near clip plane dist = OpenGL map to -1
    f32 f = FarClipPlane; // NOTE(chowie): Far clip plane dist = OpenGL map to 1

    f32 d = 2.0f / (n - f); // NOTE(chowie): Near clip plane
    f32 e = (n + f) / (n - f); // NOTE(chowie): Far clip plane

    m4x4_inv Result =
    {
        {{{a, 0, 0, 0},
          {0, b, 0, 0},
          {0, 0, d, e},
          {0, 0, 0, 1}}},

        {{{1/a,   0,   0, 0},
          {  0, 1/b,   0, 0},
          {  0,   0, 1/d, -e/d},
          {  0,   0,   0, 1}}},
    };

#if RUINENGLASS_SLOW
    m4x4 I = Result.Inverse*Result.Forward;
    v3 Test0 = Result.Forward*V3(0, 0, -n);
    v3 Test1 = Result.Forward*V3(0, 0, -f);
#endif

    return(Result);
}

internal m4x4
Columns3x3(v3 X, v3 Y, v3 Z)
{
    m4x4 Result =
    {
        {{X.x, Y.x, Z.x, 0},
         {X.y, Y.y, Z.y, 0},
         {X.z, Y.z, Z.z, 0},
         {  0,   0,   0, 1}
        },
    };

    return(Result);
}

internal m4x4
Rows3x3(v3 X, v3 Y, v3 Z)
{
    m4x4 Result =
    {
        {{X.x, X.y, X.z, 0},
         {Y.x, Y.y, Y.z, 0},
         {Z.x, Z.y, Z.z, 0},
         {  0,   0,   0, 1}
        },
    };

    return(Result);
}

internal m4x4
Translate(m4x4 A, v3 T)
{
    m4x4 Result = A;
    Result.E[0][3] += T.x;
    Result.E[1][3] += T.y;
    Result.E[2][3] += T.z;

    return(Result);
}

internal v3
GetColumn(m4x4 A, u32 Column)
{
    v3 Result = {A.E[0][Column], A.E[1][Column], A.E[2][Column]};
    return(Result);
}

internal v3
GetRow(m4x4 A, u32 Row)
{
    v3 Result = {A.E[Row][0], A.E[Row][1], A.E[Row][2]};
    return(Result);
}

internal m4x4_inv
CameraTransform(v3 X, v3 Y, v3 Z, v3 P)
{
    m4x4_inv Result;

    m4x4 A = Rows3x3(X, Y, Z);
    v3 AP = -(A*P);
    A = Translate(A, AP); // TODO(chowie): Math would notate as "-Result*P", but needs an operator overload
    Result.Forward = A;

    v3 InvX = X/LengthSq(X);
    v3 InvY = Y/LengthSq(Y);
    v3 InvZ = Z/LengthSq(Z);
    v3 InvP = V3(AP.x*InvX.x + AP.y*InvY.x + AP.z*InvZ.x,
                 AP.x*InvX.y + AP.y*InvY.y + AP.z*InvZ.y,
                 AP.x*InvX.z + AP.y*InvY.z + AP.z*InvZ.z);

    // STUDY(chowie): Not the most efficient way to build!
    m4x4 B = Columns3x3(InvX, InvY, InvZ);
    B = Translate(B, -InvP);
    Result.Inverse = B;

#if RUINENGLASS_SLOW
    m4x4 I = Result.Inverse*Result.Forward;
#endif

    return(Result);
}

//
// NOTE(chowie): Rect2
//

// NOTE(chowie): Only use to initialise unions
inline rect2
InvertedInfinityRect2(void)
{
    rect2 Result = {V2(F32Max), V2(-F32Max)};
    return(Result);
}

// RESOURCE(fabian): https://fgiesen.wordpress.com/2013/01/14/min-max-under-negation-and-an-aabb-trick/
inline rect2
Union(rect2 A, rect2 B)
{
    A.MaxN = V2(-A.MaxN.x, -A.MaxN.y);
    B.MaxN = V2(-B.MaxN.x, -B.MaxN.y);

    rect2 Result = {};
    for(s32 Corner = 0;
        Corner < ArrayCount(Result.E);
        ++Corner)
    {
        Result.E[Corner] = Minimum(A.E[Corner], B.E[Corner]);
    }

    Result.MaxN = V2(-Result.MaxN.x, -Result.MaxN.y);
    return(Result);
}

inline rect2
Intersect(rect2 A, rect2 B)
{
    A.MaxN = V2(-A.MaxN.x, -A.MaxN.y);
    B.MaxN = V2(-B.MaxN.x, -B.MaxN.y);

    rect2 Result = {};
    for(s32 Corner = 0;
        Corner < ArrayCount(Result.E);
        ++Corner)
    {
        Result.E[Corner] = Maximum(A.E[Corner], B.E[Corner]);
    }

    Result.MaxN = V2(-Result.MaxN.x, -Result.MaxN.y);
    return(Result);
}

inline v2
GetMin(rect2 Rect)
{
    v2 Result = Rect.Min;
    return(Result);
}

inline v2
GetMax(rect2 Rect)
{
    v2 Result = Rect.Max;
    return(Result);
}

// STUDY(chowie): Can refer as GetDim(...).x
inline v2
GetDim(rect2 Rect)
{
    v2 Result = Rect.Max - Rect.Min;
    return(Result);
}

inline v2
GetCenter(rect2 Rect)
{
    v2 Result = 0.5f*(Rect.Min + Rect.Max);
    return(Result);
}

inline rect2
RectMinMax(v2 Min, v2 Max)
{
    rect2 Result = {Min, Max};
    return(Result);
}

// NOTE(chowie): Min is the corner
inline rect2
RectMinDim(v2 Min, v2 Dim)
{
    rect2 Result = {Min, Min + Dim};
    return(Result);
}

// NOTE(chowie): Minkowski Sum
inline rect2
AddRadiusTo(rect2 A, v2 Radius)
{
    rect2 Result = {A.Min - Radius, A.Max + Radius};
    return(Result);
}

inline rect2
SubtractRadiusTo(rect2 A, v2 Radius)
{
    rect2 Result = {A.Min + Radius, A.Max - Radius};
    return(Result);
}

inline rect2
Offset(rect2 A, v2 Offset)
{
    rect2 Result = {A.Min + Offset, A.Max + Offset};
    return(Result);
}

// NOTE(chowie): Dim stores half-width and half-height
inline rect2
RectCenterHalfDim(v2 Center, v2 HalfDim)
{
    rect2 Result = {Center - HalfDim, Center + HalfDim};
    return(Result);
}

inline rect2
RectCenterDim(v2 Center, v2 Dim)
{
    rect2 Result = RectCenterHalfDim(Center, 0.5f*Dim);
    return(Result);
}

// STUDY: More like is "PointInRect"
inline b32x
IsInRect(rect2 Rect, v2 Test)
{
    b32x Result = ((Test.x >= Rect.Min.x) &&
                   (Test.y >= Rect.Min.y) &&
                   (Test.x < Rect.Max.x) &&
                   (Test.y < Rect.Max.y));
    return(Result);
}

inline b32x
RectsIntersect(rect2 A, rect2 B)
{
    b32x Result = !((B.Max.x <= A.Min.x) ||
                    (B.Min.x >= A.Max.x) ||
                    (B.Max.y <= A.Min.y) ||
                    (B.Min.y >= A.Max.y));
    return(Result);
}

// TODO(chowie): One of the dimensions of GetBarycentric could be collapsed,
// where there is no size, that would produce a nonsense value
inline v2
GetBarycentric(rect2 A, v2 P)
{
    v2 Result = 
    {
        SafeRatio0(P.x - A.Min.x, A.Max.x - A.Min.x),
        SafeRatio0(P.y - A.Min.y, A.Max.y - A.Min.y),
    };

    return(Result);
}

inline f32
GetArea(rect2 A)
{
    v2 Dim = GetDim(A);
    f32 Result = Dim.x*Dim.y;
    return(Result);
}

//
// NOTE(chowie): Rect2i
//

// NOTE: Convention is not add the final row with +1. If the max and
// min are coincident, the rect is empty, not a 1px long rect.
inline s32
GetWidth(rect2i A)
{
    s32 Result = A.Max.x - A.Min.x;
    return(Result);
}

inline s32
GetHeight(rect2i A)
{
    s32 Result = A.Max.y - A.Min.y;
    return(Result);
}

inline rect2i
Offset(rect2i A, v2s Offset)
{
    rect2i Result = {A.Min + Offset, A.Max + Offset};
    return(Result);
}

inline rect2i
RectMinMax(v2s Min, v2s Max)
{
    rect2i Result = {Min, Max};
    return(Result);
}

// NOTE(chowie): Min is the corner
inline rect2i
RectMinDim(v2s Min, v2s Dim)
{
    rect2i Result = {Min, Min + Dim};
    return(Result);
}

// TODO(chowie): Do you actually need this anymore?
// NOTE(chowie): Only to initialise unions
inline rect2i
InvertedInfinityRect2i(void)
{
    rect2i Result = {V2S(S32Max), V2S(S32Min)};
    return(Result);
}

inline b32x
HasArea(rect2i A)
{
    b32x Result = ((A.Min.x < A.Max.x) && (A.Min.y < A.Max.y));
    return(Result);
}

// RESOURCE: https://web.archive.org/web/20211023131624/https://lolengine.net/blog/2012/02/14/fewer-comparisons
// TODO(chowie): Less comparisons for collisions?
// NOTE(chowie): float a, b, c, d;
//               /* ... */
//               return (a > b) && (c > d);
//               The below equation might be branchless?
//               (a - b) + (c - d) > fabsf((a - b) - (c - d));
inline s32
GetClampedRectArea(rect2i A)
{
    s32 Width = (A.Max.x - A.Min.x);
    s32 Height = (A.Max.y - A.Min.y);
    s32 Result = 0;
    if((Width > 0) && (Height > 0))
    {
        Result = Width*Height;
    }

    return(Result);
}

// RESOURCE(fabian): https://fgiesen.wordpress.com/2013/01/14/min-max-under-negation-and-an-aabb-trick/
// TODO(chowie): Does the loop actually work? SIMD this!
// RESOURCE(paniq): https://gist.github.com/paniq/3f882c50f1790e323482
// TODO(chowie): Can this be refined with better abs-identities?
/*
    rect2i Result = 
    {
        Minimum(A.Min.x, B.Min.x),
        Minimum(A.Min.y, B.Min.y),
        -Minimum(-A.MaxN.x, -B.MaxN.x),
        -Minimum(-A.MaxN.y, -B.MaxN.y),
    };
*/
// TODO(chowie): SIMD
inline rect2i
Union(rect2i A, rect2i B)
{
    A.MaxN = V2S(-A.MaxN.x, -A.MaxN.y);
    B.MaxN = V2S(-B.MaxN.x, -B.MaxN.y);

    rect2i Result = {};
    for(s32 Corner = 0;
        Corner < ArrayCount(Result.E);
        ++Corner)
    {
        Result.E[Corner] = Minimum(A.E[Corner], B.E[Corner]);
    }

    Result.MaxN = V2S(-Result.MaxN.x, -Result.MaxN.y);
    return(Result);
}

inline rect2i
Intersect(rect2i A, rect2i B)
{
    A.MaxN = V2S(-A.MaxN.x, -A.MaxN.y);
    B.MaxN = V2S(-B.MaxN.x, -B.MaxN.y);

    rect2i Result = {};
    for(s32 Corner = 0;
        Corner < ArrayCount(Result.E);
        ++Corner)
    {
        Result.E[Corner] = Maximum(A.E[Corner], B.E[Corner]);
    }

    Result.MaxN = V2S(-Result.MaxN.x, -Result.MaxN.y);
    return(Result);
}

/*
inline rect2
union_bounds(rect2 *boxes, int N) // N >= 1
{
    rect2 r = boxes[0];
    for (int i=1; i < N; i++) {
        r.Min.x = Minimum(r.Min.x, boxes[i].Min.x);
        r.Min.y = Minimum(r.Min.y, boxes[i].Min.y);
        r.Max.x = Maximum(r.Max.x, boxes[i].Max.x);
        r.Max.y = Maximum(r.Max.y, boxes[i].Max.y);
    }
    return r;
}

inline rect2i
UnionRects(rect2i *Rects, s32 Amount) // NOTE(chowie): At least one other
{
    rect2i Result = Rects[0];
    for(s32 Index = 1;
        Index < Amount;
        ++Index)
    {
        Result.E[Index] = Minimum(Result.E[Index], Rects.E[Index]);
    }

    return(Result);
};
*/

#if 1
inline rect2i
UnionRects(rect2i *Rects, s32 Amount) // NOTE(chowie): Amount >= 1
{
    rect2i Result = Rects[0];
    for(s32 RectNum = 1;
        RectNum < Amount;
        ++RectNum)
    {
        Result.Min.x = Minimum(Result.Min.x, Rects[RectNum].Min.x);
        Result.Min.y = Minimum(Result.Min.y, Rects[RectNum].Min.y);
        Result.MaxN.x = -Minimum(-Result.MaxN.x, -Rects[RectNum].MaxN.x);
        Result.MaxN.y = -Minimum(-Result.MaxN.y, -Rects[RectNum].MaxN.y);
    }

    return(Result);
}
#else
// IMPORTANT(chowie): TODO(chowie): This can be _SIMD_
// TODO(chowie): Test!
inline rect2i
UnionRects(rect2i *Rects, s32 Amount) // NOTE(chowie): Amount >= 1
{
    rect2i Result = Rects[0];
    Result.MaxN = V2S(-Result.MaxN.x, -Result.MaxN.y);
    for(s32 RectNum = 1;
        RectNum < Amount;
        ++RectNum)
    {
        Rects[RectNum].MaxN = V2S(-Rects[RectNum].MaxN.x, -Rects[RectNum].MaxN.y);

        for(s32 Corner = 0;
            Corner < ArrayCount(Result.E);
            ++Corner)
        {
            Result.E[Corner] = Minimum(Result.E[Corner], Rects[RectNum].E[Corner]);
        }

        Result.MaxN = V2S(-Result.MaxN.x, -Result.MaxN.y);
    }

    return(Result);
}
#endif

// TODO(chowie): Reposition clipping region in OpenGL viewport instead!
internal rect2i
AspectRatioFit(v2u Render, v2u Window)
{
    rect2i Result = {};

    v2 OptimalWindow =
    {
        (f32)(Window.Height * (Render.Width / Render.Height)),
        (f32)(Window.Width  * (Render.Height / Render.Width)),
    };

    Result.Min.x = 0;
    Result.Max.x = RoundF32ToS32(OptimalWindow.Width);
    Result.Min.y = 0;
    Result.Max.y = RoundF32ToS32(OptimalWindow.Height);

#if 0
    if((Render.Width > 0) &&
       (Render.Height > 0) &&
       (Window.Width > 0) &&
       (Window.Height > 0))
    {
        if(OptimalWindow.Width > (f32)Window.Width)
        {
            // NOTE(chowie): Width-constrained display - top and bottom black bars
            Result.Min.x = 0;
            Result.Max.x = Window.Width;

            f32 Empty = Window.Height - OptimalWindow.Height;
            s32 HalfEmpty = RoundF32ToS32(0.5f*Empty); // NOTE(chowie): Rounding to not be on a bogus boundary
            s32 UseHeight = RoundF32ToS32(OptimalWindow.Height);

            Result.Min.y = HalfEmpty;
            Result.Max.y = Result.Min.y + UseHeight;
        }
        else
        {
            // NOTE(chowie): Height-constrained display - left and right black bars
            Result.Min.y = 0;
            Result.Max.y = Window.Height;

            f32 Empty = Window.Width - OptimalWindow.Width;
            s32 HalfEmpty = RoundF32ToS32(0.5f*Empty); // NOTE(chowie): Rounding to not be on a bogus boundary
            s32 UseWidth  = RoundF32ToS32(OptimalWindow.Width);

            Result.Min.x = HalfEmpty;
            Result.Max.x = Result.Min.x + UseWidth;
        }
    }
#endif

    return(Result);
}

//
// NOTE(chowie): Rect3
//

inline v3
GetMin(rect3 Rect)
{
    v3 Result = Rect.Min;
    return(Result);
}

inline v3
GetMax(rect3 Rect)
{
    v3 Result = Rect.Max;
    return(Result);
}

inline v3
GetDim(rect3 Rect)
{
    v3 Result = Rect.Max - Rect.Min;
    return(Result);
}

inline v3
GetCenter(rect3 Rect)
{
    v3 Result = 0.5f*(Rect.Min + Rect.Max);
    return(Result);
}

inline rect3
RectMinMax(v3 Min, v3 Max)
{
    rect3 Result = {Min, Max};
    return(Result);
}

// NOTE(chowie): Min is the corner
inline rect3
RectMinDim(v3 Min, v3 Dim)
{
    rect3 Result = {Min, Min + Dim};
    return(Result);
}

inline rect3
AddRadiusTo(rect3 A, v3 Radius)
{
    rect3 Result = {A.Min - Radius, A.Max + Radius};
    return(Result);
}

inline rect3
SubtractRadiusTo(rect3 A, v3 Radius)
{
    rect3 Result = {A.Min + Radius, A.Max - Radius};
    return(Result);
}

inline rect3
Offset(rect3 A, v3 Offset)
{
    rect3 Result = {A.Min + Offset, A.Max + Offset};
    return(Result);
}

inline b32x
HasArea(rect3 A)
{
    b32x Result = ((A.Min.x < A.Max.x) && (A.Min.y < A.Max.y) && (A.Min.z < A.Max.z));
    return(Result);
}

// NOTE(chowie): Dim stores half-width and half-height
inline rect3
RectCenterHalfDim(v3 Center, v3 HalfDim)
{
    rect3 Result = {Center - HalfDim, Center + HalfDim};
    return(Result);
}

inline rect3
RectCenterDim(v3 Center, v3 Dim)
{
    rect3 Result = RectCenterHalfDim(Center, 0.5f*Dim);
    return(Result);
}

// RESOURCE: https://fgiesen.wordpress.com/2011/10/16/checking-for-interval-overlap/
// TODO(chowie): Interval Overlap?
inline b32x
IsInRect(rect3 Rect, v3 Test)
{
    b32x Result = ((Test.x >= Rect.Min.x) &&
                   (Test.y >= Rect.Min.y) &&
                   (Test.z >= Rect.Min.z) &&
                   (Test.x < Rect.Max.x) &&
                   (Test.y < Rect.Max.y) &&
                   (Test.z < Rect.Max.z));
    return(Result);
}

inline b32x
RectsIntersect(rect3 A, rect3 B)
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
GetBarycentric(rect3 A, v3 P)
{
    v3 Result =
    {
        SafeRatio0(P.x - A.Min.x, A.Max.x - A.Min.x),
        SafeRatio0(P.y - A.Min.y, A.Max.y - A.Min.y),
        SafeRatio0(P.z - A.Min.z, A.Max.z - A.Min.z),
    };

    return(Result);
}

inline rect2
ToRectXY(rect3 A)
{
    rect2 Result = {A.Min.xy, A.Max.xy};
    return(Result);
}

//
// Colour
//

// RESOURCE(): https://www.leadwerks.com/community/blogs/entry/2844-srgb-and-linear-color-spaces/
// TODO(chowie): Check if I need more heavy-duty pow() -> test with stdlib
inline v3
Pow(v3 A, v3 B)
{
    v3 Result = {Pow(A.r, B.r), Pow(A.g, B.g), Pow(A.b, B.b)};
    return(Result);
}

#define sRGB_GAMMA 2.2f
#define sRGB_INVGAMMA (1.0f / sRGB_GAMMA)

// STUDY(chowie): Convert to linear colours e.g. SRGB255ToLinear1(V4(0.0f, 0.6f, 0.3f, 1))
// Approximation of un-srgb to srgb input
// TODO(chowie): Use graphics hardware or in shader!
inline v3
sRGBToLinear(v3 Colour)
{
    v3 Result = Pow(Colour, V3(sRGB_GAMMA));
    return(Result);
}

inline v3
LinearTosRGB(v3 Colour)
{
    v3 Result = Pow(Colour, V3(sRGB_INVGAMMA));
    return(Result);
}

inline v4
sRGBToLinear(v4 Colour)
{
    v4 Result = {Pow(Colour.rgb, V3(sRGB_GAMMA)), Colour.a};
    return(Result);
}

inline v4
LinearTosRGB(v4 Colour)
{
    v4 Result = {Pow(Colour.rgb, V3(sRGB_INVGAMMA)), Colour.a};
    return(Result);
}

// RESOURCE: https://web.archive.org/web/20211023131624/https://lolengine.net/blog/2011/3/20/understanding-fast-float-integer-conversions
// TODO(chowie): Conversion for colours?
// NOTE(chowie): Alpha is not a brightness value, but a value tells us how
// much we want to blend (as a percentage). Only ever blend in
// that linear space anyway, assume inputs are in linearly encoded
// alpha (but who knows really depending on the program).
// NOTE(chowie): Alpha channels are not converted to sRGB, is linear.

#define sRGB_255 255.0f
#define sRGB_INV255 (1.0f / sRGB_255)

inline v4
sRGB255ToLinear1(v4 Colour)
{
    v4 Result = sRGBToLinear(sRGB_INV255*Colour);
    return(Result);
}

inline v4
Linear1TosRGB255(v4 Colour)
{
    v4 Result = sRGB_255*LinearTosRGB(Colour);
    return(Result);
}

// #define One255 = 255.0f;
// #define Inv255 = 1.0f / One255;

// inline v4
// sRGB255ToLinear1(v4 C)
// {
//     v4 Result = {Square(Inv255*C.r), Square(Inv255*C.g), Square(Inv255*C.b), Inv255*C.a};
//     return(Result);
// }

// inline v4
// Linear1TosRGB255(v4 C)
// {
//     v4 Result = {One255*SquareRoot(C.r), One255*SquareRoot(C.g), One255*SquareRoot(C.b), One255*C.a};
//     return(Result);
// }

// TODO(chowie): To implement fading to modulating/changing fog
// colours, you would have to set up directly in the renderer. You
// would have to set up a separate texture unit or shader with
// procedural-ness and specify blend per pixel with graphics API
// instead of colour. Adding another layer of complexity; another
// source of screen space data. However, the storecolour code runs
// once at push time.
// STUDY(chowie): Module allows alpha blending & fog effects! Only
// works if fading to black. Then, you would need a concept of a fog
// value to _blend to_ - more flexible than our purposes.
inline v4
PremultipliedStoreColour(v4 Source)
{
    v4 Dest;
    Dest.a = Source.a;
    // STUDY(chowie): We can reverse the sense; inv modulation =
    // higher you go, more you blend in the colour. Can init to 0
    Dest.rgb = Dest.a*Source.rgb;

    return(Dest);
}

inline v4
BGRAUnpack4x8(u32 Packed)
{
    v4 Result = {(f32)((Packed >> 16) & 0xFF),
                 (f32)((Packed >> 8) & 0xFF),
                 (f32)((Packed >> 0) & 0xFF),
                 (f32)((Packed >> 24) & 0xFF)};
    return(Result);
}

// NOTE(chowie): Requires multiply 255.0f with Unpacked outside function
inline u32
BGRAPack4x8(v4 Unpacked)
{
    u32 Result = ((RoundF32ToU32(Unpacked.a) << 24) |
                  (RoundF32ToU32(Unpacked.r) << 16) |
                  (RoundF32ToU32(Unpacked.g) << 8) |
                  (RoundF32ToU32(Unpacked.b) << 0));
    return(Result);
}

inline v4
RGBAUnpack4x8(u32 Packed)
{
    v4 Result = {(f32)((Packed >> 0) & 0xFF),
                 (f32)((Packed >> 8) & 0xFF),
                 (f32)((Packed >> 16) & 0xFF),
                 (f32)((Packed >> 24) & 0xFF)};
    return(Result);
}

// NOTE(chowie): Requires multiply 255.0f with Unpacked outside function
inline u32
RGBAPack4x8(v4 Unpacked)
{
    u32 Result = ((RoundF32ToU32(Unpacked.a) << 24) |
                  (RoundF32ToU32(Unpacked.b) << 16) |
                  (RoundF32ToU32(Unpacked.g) << 8) |
                  (RoundF32ToU32(Unpacked.r) << 0));
    return(Result);
}

//
//
//

// RESOURCE(): https://www.flipcode.com/archives/Little_Math_Trick.shtml
// STUDY(chowie): For loop accumulator e.g. raycasting to += rather
// than *= using log and exponential for faster calculations

// RESOURCE(sam): https://web.archive.org/web/20211023131624/https://lolengine.net/blog/2011/12/14/understanding-motion-in-games
// TODO(chowie): Implement Verlet equation of motions instead of Euler
//  Accel = V3(0, 0, -9.81f);
//  v3 OldVel = Vel;
//  Vel = Vel + Accel * dtForFrame;
//  Pos = Pos + (OldVel + Vel) * 0.5f * dt;

// TODO(chowie): Try to use this!
// RESOURCE: https://gist.github.com/d7samurai/f98cb2aa30a6d73e62a65a376d24c6da
// NOTE(from d7sam): Storing argb color in various compact forms, either
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
    return (((u16)A + 1)*B) >> 8;
}

//
// NOTE(chowie): Hashing Functions
// TODO(chowie): Better Hash Function
//

// RESOURCE(reed): https://www.reedbeta.com/blog/hash-functions-for-gpu-rendering/
// NOTE(chowie): Compared to Wang Hash, slightly better performance and much better statistical quality
inline u32
PCGHash(u32 Value)
{
    u32 State = Value*747796405u + 2891336453u;
    u32 Word = ((State >> ((State >> 28u) + 4u)) ^ State)*277803737u;
    u32 Result = (Word >> 22u) ^ Word;
    return(Result);
}

// NOTE(by mauro): Using Szudnik Pairing.
// * Seed would always be the same based on location, and collisions would only occur as you got very far away from the origin
// * You could fit two 16-bit integers into a single 32-bit integer with no collisions.
// RESOURCE(mauro): https://dmauro.com/post/77011214305/a-hashing-function-for-x-y-z-coordinates
inline u32
MauroHash(v3u Value)
{
    u32 Max = Maximum3(Value.x, Value.y, Value.z);
    u32 Result = CUBIC(Max) + (2*Max*Value.z) + Value.z;
    if(Max == Value.z)
    {
        Result += QUADRATIC(Maximum(Value.x, Value.y));
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
    s32 NegX = (Value.x >= 0) ? (2 * Value.x) : (-2*Value.x - 1);
    s32 NegY = (Value.y >= 0) ? (2 * Value.y) : (-2*Value.y - 1);
    s32 NegZ = (Value.z >= 0) ? (2 * Value.z) : (-2*Value.z - 1);

    s32 Max = Maximum3(NegX, NegY, NegZ);
    s32 Result = CUBIC(Max) + (2*Max*NegZ) + NegZ;
    if(Max == NegZ)
    {
        Result += QUADRATIC(Maximum(NegX, NegY));
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
// RESOURCE(): https://matthias-research.github.io/pages/tenMinutePhysics/index.html
// RESOURCE(cincotti): https://carmencincotti.com/2022-10-31/spatial-hash-maps-part-one/#how-to-populate-a-dense-hash-table
// NOTE(chowie): Originally intended for s32, with an Abs. Why not
// just type cast to u32?
inline u32
MullerHash(v3u Value)
{
    u32 Result = (Value.x*92837111) ^ (Value.y*689287499) ^ (Value.z*283923481);
    return(Result);
}

inline u32
MullerHash(v3s Value)
{
    s32 Result = (Value.x*92837111) ^ (Value.y*689287499) ^ (Value.z*283923481);
    Result = AbsoluteValue(Result);
    return(Result);
}

// RESOURCE(codejar): https://youtu.be/OjZ7vV62lJk?t=271
/*
inline u32
Hash(v4u Value)
{
    u32 Mult = 1103515245u;
    Value *= Mult;
    u32 Result = ((Value >> 2u) ^ (Value.yzwx >> 1u) ^ (Value.zwxy >> 3u) ^ (Value.wxyz >> 4) * Mult);
    return(Result);
}
*/

// RESOURCE(mason remaley): https://gamesbymason.com/2020/03/30/symmetric-matrices/
// f(x,y) = (x + (y)(y+1)/2), x = low/column, y = high/triangle
// RESOURCE(): https://en.wikipedia.org/wiki/Triangular_number
// Technically more correct to process the largest int with binary OR (, branchless), too niche for me thought
inline u32
TriangleNumber(v2u Value)
{
    u32 Result = (Value.x + ((Value.y)*(Value.y + 1))/2);
    return(Result);
}

// NOTE(chowie): Pairwise
// RESOURCE(mason remaley): https://gamesbymason.com/2020/03/30/symmetric-matrices/
internal u32
MapPairwiseToIndex(u16 A, u16 B)
{
    // TODO(chowie): I assume that if with swap is better than two if min/max
    // STUDY(chowie): This makes entity pairs AB vs BA pair agnostic
    v2u Pair = V2U((u32)B, (u32)A);
    if(A < B)
    {
        Swap(u32, Pair.x, Pair.y);
    }

    u32 Result = TriangleNumber(Pair);
    return(Result);
}

// NOTE(chowie): Non-pairwise, must be used with hierarchical trees
// RESOURCE(alex fink & jorg rhiemeier): https://listserv.brown.edu/archives/cgi-bin/wa?A2=ind0907B&L=CONLANG&P=R12478
// P.S. From 1999 amazingly! It's not even your main point????
// f(x,y) = 1 + 2(x + (x+y)(x+y+1)/2)
inline u32
TriangleNumber2D(v2u Value)
{
    v2u Offset = V2U(Value.x, Value.x + Value.y);
    u32 Result = 2*TriangleNumber(Offset) + 1;
    return(Result);
}

// RESOURCE(): https://theartincode.stanis.me/008-djb2/
inline u32
DJB2Hash(char *Scan)
{
    u32 MagicNumber = 5381;
    u32 Result = ((MagicNumber << 5) + MagicNumber) + *Scan;
    return(Result);
}

//
// NOTE(chowie): Compression
//

// NOTE(chowie): Compact implementation that balances processor cache usage against speed.
// RESOURCE(schelling): https://github.com/schellingb/ZIPValidateCRC/blob/main/ZIPValidateCRC.cpp
// RESOURCE(karl malbrain): http://www.geocities.ws/malbrain/
global u32 CRC32Table[16] = { 0, 0x1db71064, 0x3b6e20c8, 0x26d930ac, 0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c, 0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c, 0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c };
inline u32
CRC32(void *Data, umm Size)
{
    u32 Result = (u32)~(u32)0;
    for(u8 CRCIndex = 0, *Base = (u8 *)Data; // TODO(chowie): Does "*Base = u8 *Base"?
        Size--;
        )
    {
        CRCIndex = *Base++;
        Result = (Result >> 4) ^ CRC32Table[(Result & 0xF) ^ (CRCIndex & 0xF)];
        Result = (Result >> 4) ^ CRC32Table[(Result & 0xF) ^ (CRCIndex >> 4)];
    }
    return(~Result);
}

// TODO(chowie): Try to use this for tile/world storage? (Must be in u32)
//
// RESOURCE(fabian): https://fgiesen.wordpress.com/2009/12/13/decoding-morton-codes/
//

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
// TODO(chowie): Convert to BitscanForward & Reverse? Or SSE SIMD?
inline u32
FloatFlip(u32 F32)
{
    u32 Mask = -(s32)(F32 >> 31) | 0x80000000;
    u32 Result = F32 ^ Mask;
    return(Result);
}

inline void
FloatFlipX(u32 *F32)
{
    u32 Mask = -(s32)(*F32 >> 31) | 0x80000000;
    *F32 ^= Mask;
}

inline u32
InvFloatFlip(u32 F32)
{
    u32 Mask = ((F32 >> 31) - 1) | 0x80000000;
    u32 Result = F32 ^ Mask;
    return(Result);
}

internal void
HerfRadixSort(u32 Count, f32 *First, f32 *Temp)
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

//
// TODO(chowie): Particle Systems HmH 337/(first 20 min of)338
//

/*
  Follow this guide on how fast the code should be structured! 0.02%
  vs 0.50% in -Od is a sizable different based on what we had to
  change at the time! It's next to nothing in O2 mode. However, some
  notation will be much more clunky to write! NOTE(chowie): It should
  be pretty similar to what we had before, so no big changes needed!
  
#define MMSetExpr(Expr) _mm_set_ps(Expr, Expr, Expr, Expr)

inline __m128
operator+(__m128 A, __m128 B)
{
    __m128 Result = _mm_add_ps(A, B);
    return(Result);
}

inline __m128
operator*(f32 A, __m128 B)
{
    __m128 Result = _mm_mul_ps(_mm_set1_ps(A), B);
    return(Result);
}

#define MAX_PARTICLE_COUNT 4096
#define MAX_PARTICLE_COUNT_4 (MAX_PARTICLE_COUNT/4)
#define PARTICLE_SYSTEM_COUNT 64

struct particle_system_info
{
    entity_id ID;
    u32 FramesSinceTouched;
};
struct particle_system
{
    __m128 Px[MAX_PARTICLE_COUNT_4];
    __m128 Py[MAX_PARTICLE_COUNT_4];
    __m128 Pz[MAX_PARTICLE_COUNT_4];
    
    __m128 dPx[MAX_PARTICLE_COUNT_4];
    __m128 dPy[MAX_PARTICLE_COUNT_4];
    __m128 dPz[MAX_PARTICLE_COUNT_4];
    
    __m128 ddPx[MAX_PARTICLE_COUNT_4];
    __m128 ddPy[MAX_PARTICLE_COUNT_4];
    __m128 ddPz[MAX_PARTICLE_COUNT_4];
    
    __m128 Cr[MAX_PARTICLE_COUNT_4];
    __m128 Cg[MAX_PARTICLE_COUNT_4];
    __m128 Cb[MAX_PARTICLE_COUNT_4];
    __m128 Ca[MAX_PARTICLE_COUNT_4];

    __m128 dCr[MAX_PARTICLE_COUNT_4];
    __m128 dCg[MAX_PARTICLE_COUNT_4];
    __m128 dCb[MAX_PARTICLE_COUNT_4];
    __m128 dCa[MAX_PARTICLE_COUNT_4];
    
    u32 NextParticle4;
    
};

struct v3_4x
{
    __m128 x;
    __m128 y;
    __m128 z;
};

struct v4_4x
{
    __m128 r;
    __m128 g;
    __m128 b;
    __m128 a;
};

struct particle_v3
{
    __m128 x;
    __m128 Ignored0[MAX_PARTICLE_COUNT_4 - 1];
    
    __m128 y;
    __m128 Ignored1[MAX_PARTICLE_COUNT_4 - 1];
    
    __m128 z;
    __m128 Ignored2[MAX_PARTICLE_COUNT_4 - 1];
};

inline particle_v3 &
operator+=(particle_v3 &A, v3_4x B)
{
    A.x = _mm_add_ps(A.x, B.x);
    A.y = _mm_add_ps(A.y, B.y);
    A.z = _mm_add_ps(A.z, B.z);
    
    return(A);
}

inline v3_4x
operator*(f32 As, v3_4x B)
{
    v3_4x Result;
    
    __m128 A = _mm_set1_ps(As);
    Result.x = _mm_mul_ps(A, B.x);
    Result.y = _mm_mul_ps(A, B.y);
    Result.z = _mm_mul_ps(A, B.z);
    
    return(Result);
}

inline v3_4x
operator+(v3_4x A, v3_4x B)
{
    v3_4x Result;
    
    Result.x = _mm_add_ps(A.x, B.x);
    Result.y = _mm_add_ps(A.y, B.y);
    Result.z = _mm_add_ps(A.z, B.z);
    
    return(Result);
}

inline v3_4x
operator*(f32 As, particle_v3 B)
{
    v3_4x Result;
    
    __m128 A = _mm_set1_ps(As);
    Result.x = _mm_mul_ps(A, B.x);
    Result.y = _mm_mul_ps(A, B.y);
    Result.z = _mm_mul_ps(A, B.z);
    
    return(Result);
}

struct particle_v4
{
    __m128 r;
    __m128 Ignored0[MAX_PARTICLE_COUNT_4 - 1];
    
    __m128 g;
    __m128 Ignored1[MAX_PARTICLE_COUNT_4 - 1];
    
    __m128 b;
    __m128 Ignored2[MAX_PARTICLE_COUNT_4 - 1];
    
    __m128 a;
    __m128 Ignored3[MAX_PARTICLE_COUNT_4 - 1];
};

struct particle
{
    particle_v3 *P;
    particle_v3 *dP;
    particle_v3 *ddP;
    particle_v4 *C;
    particle_v4 *dC;
};

struct particle_cache
{
    particle_system_info SystemInfos[PARTICLE_SYSTEM_COUNT];
    particle_system Systems[PARTICLE_SYSTEM_COUNT];
};

struct particle_spec
{
    f32 SomethingSomethingSomething;
};
*/

#define RUINENGLASS_MATH_H
#endif
