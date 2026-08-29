/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Michael Chow $
   $Notice: $
   ======================================================================== */

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <math.h>
#include <string.h>
#include <windows.h>
#include <intrin.h>

#if !defined(internal)
#define internal static
#endif
#define local_persist static
#define global static

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;
typedef int_least32_t b32x;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef intptr_t smm;
typedef uintptr_t umm;

typedef size_t memory_index;

typedef float f32;
typedef double f64;

#define SID(string) string
typedef u32 sid;

#define Kilobytes(Value) ((Value)*1024LL)
#define Megabytes(Value) (Kilobytes(Value)*1024LL)
#define Gigabytes(Value) (Megabytes(Value)*1024LL)
#define Terabytes(Value) (Gigabytes(Value)*1024LL)

#define Len(arr)(sizeof((arr)) /(sizeof((arr)[0])))
#define Min(A, B) ((A < B) ? (A) : (B))
#define Max(A, B) ((A > B) ? (A) : (B))

#define Assert(Expression) if(!(Expression)) {*(int *)0 = 0;}
#define InvalidCodePath Assert(!"InvalidCodePath")
#define InvalidDefaultCase default: {InvalidCodePath;} break

#define NotImplemented Assert(!"NotImplemented")

#define Swap(type, A, B) {type Temp = (A); (A) = (B); (B) = Temp;}

#define foreachN(type, Value, array) for(type Value = 0; Value < array; ++Value)
#define foreach(type, Value, array) for(type Value = 0; Value < Len(array); ++Value)

#define Pow2N(Value) (1 << (Value))
#define BitSet(Bit) Pow2N(Bit)

inline s32
SignOf(s32 Value)
{
    s32 Result = (Value >= 0) ? 1 : -1;
    return (Result);
}

inline b32x
IsEndOfLine(char C)
{
    b32x Result = ((C == '\n') ||
                   (C == '\r'));

    return(Result);
}

inline b32x
IsWhitespace(char C)
{
    b32x Result = ((C == ' ') ||
                   (C == '\t') ||
                   (C == '\v') ||
                   (C == '\f') ||
                   IsEndOfLine(C));

    return(Result);
}

inline u32
StringLen(char *String)
{
    u32 Count = 0;
    if(String)
    {
        while(*String++)
        {
            ++Count;
        }
    }

    return(Count);
}

// TODO(chowie): Not sure if I need this to squish strings
internal void
CatStrings(char *SourceA, umm SourceACount,
           char *SourceB, umm SourceBCount,
           char *Dest, umm DestCount)
{
    // TODO: Dest bound checking
    foreachN(u32, Index, SourceACount)
    {
        *Dest++ = *SourceA++;
    }
    foreachN(u32, Index, SourceBCount)
    {
        *Dest++ = *SourceB++;
    }

    *Dest++ = 0; // NOTE: Insertion of NULL terminator
}

inline umm
Clamp(umm Min, umm Value, umm Max)
{
    umm Result = Value;

    if(Result <= Min)
    {
        Result = Min;
    }
    else if(Result > Max)
    {
        Result = Max;
    }

    return(Result);
}

inline u32
AbsDifferenceClampAboveZero(u32 A, u32 B)
{
    __m128i NewA = _mm_set1_epi32((s32)A);
    __m128i NewB = _mm_set1_epi32((s32)B);
    __m128i AB = _mm_subs_epu8(NewA, NewB);
    __m128i BA = _mm_subs_epu8(NewB, NewA);

    __m128i Result = _mm_or_si128(AB, BA);
    return((u32)_mm_extract_epi8(Result, 0));
}

inline u32
ClearLeftMostSet(u32 value)
{
    Assert(value != 0);
    return(value & (value - 1));
}

struct bit_scan_result
{
    b32x Found;
    u32 Index;
};
inline bit_scan_result
FindLeastSignificantBit(u32 Value) // NOTE(chowie): ctz
{
    bit_scan_result Result = {};
    Result.Found =  _BitScanForward((unsigned long *)&Result.Index, Value);
    return(Result);
}

inline bit_scan_result
FindMostSignificantBit(u32 Value) // NOTE(chowie): clz
{
    bit_scan_result Result = {};
    Result.Found =  _BitScanReverse((unsigned long *)&Result.Index, Value);
    return(Result);
}

//
//
//

inline u32
RoundF32ToU32(f32 F32)
{
    u32 Result = (u32)_mm_cvtss_si32(_mm_set_ss(F32));
    return(Result);
}

inline s32
NumDigitsLog10(u32 Value)
{
    u32 Result = 0;
    if(Value < 10000000000)
    {
        Result = ((Value >= 1000000000) ? 10 :
                  (Value >= 100000000) ? 9 :
                  (Value >= 10000000) ? 8 :
                  (Value >= 1000000) ? 7 :
                  (Value >= 100000) ? 6 :
                  (Value >= 10000) ? 5 :
                  (Value >= 1000) ? 4 :
                  (Value >= 100) ? 3 :
                  (Value >= 10) ? 2 : 1);
    }
    else
    {
        Result = ((Value >= 1000000000000000000) ? 19 :
                  (Value >= 100000000000000000) ? 18 :
                  (Value >= 10000000000000000) ? 17 :
                  (Value >= 1000000000000000) ? 16 :
                  (Value >= 100000000000000) ? 15 :
                  (Value >= 10000000000000) ? 14 :
                  (Value >= 1000000000000) ? 13 :
                  (Value >= 100000000000) ? 12 : 11);
    }

    return(Result);
}

#define TEMP_BUFFER_SIZE 512
#define Base10 10
global f32 Bases[] = { 1, 10, 100, 1000, 10000, 100000, 1000000 };
// RESOURCE: https://gist.github.com/d7samurai/1d778693ba33bbd2b9d709b209cc0aba
// TODO(chowie): Convert to using arenas!
// TODO(chowie): This hideous functions is really convenient! Probably only use this for debugging only!
struct d7sam_concat
{
    d7sam_concat(char* Source) { operator()(Source); }
    d7sam_concat(char  Source) { operator()(Source); }
    d7sam_concat(s32 Value) { operator()(Value); }
    d7sam_concat(f32 Value, u32 Decimals = 2) { operator()(Value, Decimals); }

    u32 CharCount = 0;
    char TempBuffer[TEMP_BUFFER_SIZE];

    d7sam_concat &
    operator()(char* Source)
    {
        // TODO(chowie): How do I remove the null terminator?
        // NOTE(chowie): Include null terminator
        u32 Size = StringLen(Source) + 1;

        foreachN(u32, CharIndex, Size)
        {
            TempBuffer[CharCount++] = Source[CharIndex];
        }
        CharCount--;

        return(*this);
    }

    d7sam_concat &
    operator()(char Source)
    {
        TempBuffer[CharCount++] = Source;
        return(*this);
    }

    // TODO(chowie): Compare this vs HmH OpenGLParseNumber
    d7sam_concat &
    operator()(s32 Value)
    {
        b32x Negative = (Value < 0);
        if(Negative)
        {
            Value = -Value;
        }

        CharCount += NumDigitsLog10(Value) + Negative;
        s32 CharIndex = CharCount;

        // STUDY(chowie): Process digits backwards; must flip buffer!
        TempBuffer[CharIndex--] = 0; // STUDY(chowie): Null terminate at the end (even if buffer is full)
        do {
            TempBuffer[CharIndex--] = '0' + (Value % Base10); // STUDY(chowie): Value % Base -> 1st Digit
            Value /= Base10; // STUDY(chowie): Value / Base -> Moves one digit down e.g. 123 -> 12
        } while(Value);

        if(Negative)
        {
            TempBuffer[CharIndex] = '-';
        }

        return(*this);
    }

    d7sam_concat &
    operator()(f32 Value, u32 Decimals = 2)
    {
        b32x Negative = (Value < 0);
        if(Negative)
        {
            Value = -Value;
        }

        u32 CastValue = RoundF32ToU32(Bases[Decimals]*Value);
        u32 MaxDecimals = Max(NumDigitsLog10(CastValue), (s32)Decimals + 1);
        CharCount += MaxDecimals + Negative + (Decimals > 0);
        u32 CharIndex = CharCount;

        TempBuffer[CharIndex--] = 0;
        do {
            TempBuffer[CharIndex--] = '0' + (CastValue % Base10);
            CastValue /= Base10; // STUDY(chowie): For floats, dividing by 10 is destructive and precision may be loss because 10 isn't a power of 2!
            if(CharIndex == (CharCount - Decimals - 1))
            {
                TempBuffer[CharIndex--] = '.';
            }
        } while(CastValue || ((CharCount - CharIndex) <= (Decimals ? Decimals + 2 : 0)));

        if(Negative)
        {
            TempBuffer[CharIndex] = '-';
        }

        return(*this);
    }

    operator char* ()
    {
        return(TempBuffer);
    }
};

//
//
//

// RESOURCE(): https://gist.github.com/Doy-lee/0c4d23af4afc51ea8b03b60d0af9a1fb
// TODO(chowie): Try using error sinks!

// RESOURCE(): https://randygaul.github.io/api-design/2019/04/26/Error-Codes-and-Error-Handling.html
// E.g.
// error_t err = do_something(params);
// if (err.is_error()) {
//     handle_error(err.details);
// }

struct error_code
{
    b32x Code; // NOTE(chowie): 1 is success, 0 is failure
    char *Details; // NOTE(chowie): Don't localise
};

inline b32x
IsError(error_code Error)
{
    b32x Result = (Error.Code == false);
    return(Result);
}

inline error_code
ErrorMake(b32x Code, char *Details)
{
    error_code Result =
    {
        Code,
        Details,
    };

    return(Result);
}

inline error_code
ErrorFailure(char *Details)
{
    error_code Result = ErrorMake(false, Details);
    return(Result);
}

inline error_code
ErrorSuccess(void)
{
    error_code Result = ErrorMake(true, 0);
    return(Result);
}

//
//
//

// RESOURCE(): https://github.com/mojobojo/smaz/blob/master/smaz.c
// TODO(chowie): String compression

// RESOURCE(): https://handmade.network/forums/t/8696-implementing_ctype.h_functions_using_bit_operations.
inline b32x
IsLower(char c)
{
    b32x Result = (c >= 'a') && (c <= 'z');
    return(Result);
}

inline b32x
IsUpper(char c)
{
    b32x Result = (c >= 'A') && (c <= 'Z');
    return(Result);
}

inline b32x
IsDigit(char c)
{
    b32x Result = (c >= '0') && (c <= '9');
    return(Result);
}

inline b32x
IsAlpha(char c)
{
    b32x Result = ((c ^ 0x40) - 1) < 0x5B; // NOTE(chowie): 
    return(Result);
}

inline b32x
IsAlnum(char c)
{
    b32x Result = IsDigit(c) || IsAlpha(c);
    return(Result);
}

inline s32
ToDigit(char c)
{
    s32 Result = c - '0';
    return(Result);
}

inline char
ToChar(u32 i)
{
    char Result = (char)(i + '0');
    return(Result);
}

// RESOURCE(): http://0x80.pl/notesen/2016-01-06-swar-swap-case.html
internal void
ToUpper(char s)
{
    if(IsLower(s))
    {
        s ^= (1 << 5);
    }
}

internal void
ToUpper(char *s, umm n)
{
    foreachN(umm, j, n)
    {
        if(IsLower(s[j]))
        {
            s[j] ^= (1 << 5);
        }
    }
}

internal void
ToLower(char s)
{
    if(IsUpper(s))
    {
        s ^= (1 << 5);
    }
}

internal void
ToLower(char *s, umm n)
{
    foreachN(umm, j, n)
    {
        if(IsUpper(s[j]))
        {
            s[j] ^= (1 << 5);
        }
    }
}

// RESOURCE(): http://0x80.pl/notesen/2016-12-21-swar-digits-validate.html
// COULDDO(chowie): SSE?
internal b32x
IsAllDigits(char* String, umm Size)
{
    b32x Result = true;
    foreachN(umm, i, Size)
    {
        u8 c = (u8)(String[i] - '0');
        if(c > 9)
        {
            Result = false;
            break;
        }
    }

    return(Result);
}

// RESOURCE(): https://theartincode.stanis.me/008-djb2/
// RESOURCE(): https://cowboyprogramming.com/2007/01/04/practical-hash-ids/
// RESOURCE(): https://randygaul.github.io/data-structures/architecture/hash/2015/12/11/Preprocessed-Strings-for-Asset-IDs.html
// TODO(chowie): Try preprocessing step on strings to be made as hash

// NOTE(from randygaul): This allows to hash a prefix string, and then
// feed the result off to hash the suffix. Mick explains this in his
// article this idea of combining hashed values as a useful feature
// for supporting tools and assets. This can be implemented both at
// compile or run-time (though I haven't quite thought of a need to do
// this at compile-time yet)
//
// In other words, given a hash value from a string A (HASH(A)), and
// given another string B, then you can calculate HASH(A+B) by
// calculating HASH(B) using a starting value of HASH(A). This means
// you can calculate HASH(A+B) without ever knowing the actual
// contents of the string A. This is very useful if you have assets
// that have a series of different extensions or other appended
// strings (e.g.: MDL_ROBOT_01_ might have MDL_ROBOT_01_LEFT_ARM,
// MDL_ROBOTO_01_LEFT_LEG, etc.) You can quickly calculate the hash
// values of the string with the extensions without having to know the
// original string.
//
// COULDDO(chowie): Integer hashed strings when assets are "locked for
// release" and store important strings in a debug table.

#define DJB2HASH_MAGICNUMBER 5381

// COULDDO(chowie): Do I care enough to support case-sensitive hash? (Remove ToLower)
inline u32
Hash(u32 Prefix, char Suffix)
{
    ToLower(Suffix);
    u32 Result = ((Prefix << 5) + Prefix) + Suffix;
    return(Result);
}

inline u32
Hash(char Suffix)
{
    u32 Result = Hash(DJB2HASH_MAGICNUMBER, Suffix);
    return(Result);
}

// COULDDO(chowie): I don't think suffix or infix is even possible?

// IMPORTANT(chowie): Case-insensitive hash by default
inline u32
Hash(u32 Prefix, char *Suffix)
{
    u32 Result = Prefix;
    for(char *Scan = Suffix;
        *Scan;
        Scan++)
    {
        Result = Hash(Result, *Scan);
    }

    return(Result);
}

inline u32
Hash(char *Suffix)
{
    u32 Result = Hash(DJB2HASH_MAGICNUMBER, Suffix);
    return(Result);
}

// NOTE(chowie): To guard for collisions/debug table.
inline u32
CheckHash_(u32 HashID, char *Name)
{
    Assert(HashID == Hash(Name));
    return(HashID);
}

#ifdef RUINENGLASS_INTERNAL
#define CheckHash(HashID, Name) CheckHash_(HashID, Name)
#else
#define CheckHash(HashID, Name) HashID
#endif

struct buffer
{
    umm Size;
    u8 *Data;
};
typedef buffer string;

#define CONSTANT_STRING(String) {sizeof(String) - 1, (u8 *)(String)}

internal string
WrapZ(char *Z)
{
    string Result;

    Result.Size = StringLen(Z);
    Result.Data = (u8 *)Z;

    return(Result);
}

internal string
BundleString(char *Z, umm Size)
{
    string Result;

    Result.Size = Size;
    Result.Data = (u8 *)Z;

    return(Result);
}

// RESOURCE(): https://chrisgreendevelopmentblog.wordpress.com/2024/10/03/approaches-for-efficient-unique-symbols-in-c/
enum sym_type : u64
{
    NIL = 0,
    QUIT = 1,
    SAVE = 2,

    INVALID = ~0ull,
};
// TODO(chowie): Preprocessed strings and a generate .h file!
// e.g. constexpr Symbol_t MA_STRENGTH = Symbol_t( 0x55aa348978 );  //Hash function calculated by preprocessor

// COULDDO(chowie): Convert birdfont .ttf to default lowercase?

// IMPORTANT(chowie): API works under assumption of Radix40ToAscii[],
// convert between custom formats e.g. ConlangRadix40ToAscii[]
global string Radix40ToAscii        = CONSTANT_STRING("?0123456789_abcdefghijklmnopqrstuvwxyz"); // 38/40
global string ConlangRadix40ToAscii = CONSTANT_STRING("?(![{,%}]?)_abcdefghijklmnopqrstuvwxyz"); // 38/40

// IMPORTANT: TODO(chowie): Make sure the naming uses some software
// that gives you a template when you give the name. Or use software
// that allows a default naming convention?

// RESOURCE(): 'Fallout 4's' Modular Level Design 2016 - https://gdcvault.com/play/1023202/-Fall
// IMPORTANT TODO(chowie): Make sure CTRL + Mouse Wheel works to swap
// variants/kits instead of a menu! CTRL + Right Click to pull up category?
// IMPORTANT TODO(chowie): Helper markers (in editor) on objects to
// help click them easier without needing to turn the camera!
// COULDDO(chowie): I can't imagine scenes would be so complex that
// we'll need to build layers?
// NOTE(chowie): Rendering kits generally might be a pain
// IMPORTANT(chowie): When making kits!
// 1) Basekit, don't make a greybox kit, blank walls doesn't help
// 2) COULDDO(chowie): Make in pieces, a corner walls instead of made
// from 1 piece (easy to render/optimise), it could be a corner made
// via a wall, corner, ceiling, floor for granular/variation combo.
// However, object count explodes! Would rather swap by texture?
//
//                                W______________T
//                                /             /|                 /|
//                               /             / |                / |
//                              /             /  |               /  |
//            open             /             /   |              /   |
//        ______________     H/_____________/Y   |             /    |
//       /             /      |             |    |             |    | 
//      /             /       |             |    |             |    |
//     /             /        |             |    |     unround |    | round
//    /             /         |             |    |             |    |
//   /_____________/          |   ~         |    |M            |    |
//        closed              |             |   /              |   /
//                            |             |  /               |  /
//                            |             | /                | /
//                           X|_____________|/V                |/
//                                 AIOU
//
//                                        back
//                             _____________
//                            |             |
//                            |             |
//                            |             |
//                            |             |
//                            |    front    |
//                            |             |
//                            |             |
//                            |             |
//                            |_____________|
//

//
// 3D ENVIRONMENT ART MODULAR KIT RADIX40 ENCODING:
//
// - 12 char max
//
// _ _|_ _ _ _|_|_|_|_|_ _
// 0 1 2 3 4 5 6 7 8 9 10 11
//
// - byte 0-1   = Basekit Name e.g. Ut (Utility = kits made of other kits, e.g. arch walls with pipes), Pi (Pipe), In (Industrial), De (Deco = MultiLevel Interior/Exterior), Bu (Building), Tu (Steam Tunnel = Vertical), Cav (Cave), Pr (Prop/Set Piece), Gl (Glue = Doorframes, blending between different kits/tiles), He (Hero = Inflexible custom set pieces)
// - byte 2-5   = Subkit Name e.g. In Pr Basekit: Mach (Machine), Foli (Foliage), Gree (Greebles/Decals), Hole (Hole e.g. Wall with punched holes)
// - byte 6     = Piece Type e.g. 1 (1 way corridor), 2 (hallway turn/corner), 3 (junction), 4 (junction)
// - byte 7     = Spatial Type e.g. I (Indoor), O (Outdoor), S (Semi-enclosed/Threshold spaces e.g. porches, verandahs)
// - byte 8     = Tiling Direction e.g. 0 (None), V (Vertically), H (Horizontally), N, S, E, W (Cardinal Directions)
// - byte 9     = Animation Repligram Trigger e.g. 0 (None, Static/Decal), P (on proximity of player/mob entity), N (autonomously = waits for nature/time e.g. tree shrivelling), F (Fire), R (Rain), W (Water), O (Oxygen), D (Damage), I (Illness)
// - byte 10-11 = Variant Code e.g. a1, b1, c1 etc (instead of 00, 01, 02 etc), nicer to view for artist
//

//
// 2D/3D ENVIRONMENT STATIC TEXTURE ART (+ DESIGN ICONS AND ILLUSTRATION) RADIX40 ENCODING:
//
// - 12 char max
//
// _ _|_ _ _ _ _|_|_|_|_ _
// 0 1 2 3 4 5 6 7 8 9 10 11
//
// - byte 0-1   = Texture Category/Purpose Name e.g. Li (Lighting/Sky/Astral/Weather), Ar (Architectural), La (Landscape/Flora/Aquatic/Liquids), Np (NPCs/Fauna), Pr (Props/Functional/Interactable/Misc), Dv (Technical/Dev e.g. Collision), or Meta Placement e.g. Sp (Splash), Ma (Main Menu), Lo (Logo), UI (UI Icons), HU (HUD), Wo (In-game/World), Bi (In-game/World with Billboards), Co (Concept Art)
// - byte 2-6   = Texture Name (flexible range = start byte 3 then chop 4 bytes off end!)
// - byte 7     = Texture Map e.g. D (Diffuse), N (Normal), S (Specular), A (AO = Ambient Occlusion)
// - byte 8     = Shape/Decorative Type e.g. L (Large 1m block), S (Small 0.5m Block), F (FX 1m Block), X (Fletched/3-Cross Deco), T (Triangle Deco), B (Box Deco), Z (Z Deco), A (3-Axis-Aligned Deco), P (Panel/Plane/Decal/Design Icon/Illustration)
// - byte 9     = Face Direction e.g. 0 (None), T (Tiling Texture/Seamless), V (Vertical), H (Horizontal), N, S, E, W, U, D (Cardinal Directions), B (Billboard)
// - byte 10-11 = Variant Code e.g. a1, b1, c1 etc (instead of 00, 01, 02 etc), nicer to view for artist
//
// NOTE(chowie): No animation trigger for textures
// RESOURCE(): Sub-category ideas - https://ayemteezy.github.io/minecraft-storage-organization-cheatsheet/
//

//
// 2D/3D CHARACTER ART MODULAR KIT (+ ANIMATION FRAME/VIDEO CLIPS) RADIX40 ENCODING:
//
// - 12 char max
//
// _ _|_ _ _ _ _|_|_ _ _ _
// 0 1 2 3 4 5 6 7 8 9 10 11
//
// - byte 0-1   = Character NPC/Character Class/Prop Name e.g. Bi (Birds), El (Elephant), In (Insects), Pr (Prop/Set Piece e.g. Umbrella, Cigarette) or Animation Category e.g. L (Locomotion), C (Combat), E (Interact), N (Narrative), A (NPC/AI), X (Cinematic)
// - byte 2-6   = Character Body Part Category Name
// - byte 7     = Animation Repligram Trigger e.g. 0 (None, Static/Decal), T (on proximity, entity/mob trigger), N (autonomously = waits for nature/time e.g. tree shrivelling), F (Fire), R (Rain), W (Water), O (Oxygen), D (Damage), I (Illness)
// - byte 9-11  = Variant Code e.g. a01, b01, c01 etc, nicer to view for artist
//

//
// CONLANG CORPUS RADIX40 ENCODING:
//
// - 12 char max
//
// _|_ _|_ _ _ _ _ _ _ _ _
// 0 1 2 3 4 5 6 7 8 9 10 11
//
// - byte 0    = Conlang Type e.g. Base 10 Numbers, Eurolang, Asialang, Calclang etc
// - byte 1-2  = Parts of Speech (primary tag) e.g. verb, noun, place etc
// - byte 3-11 = Conlang Word
//

//
// FONT RADIX40 ENCODING:
//
// - 12 char max
//
// _ _|_ _ _ _ _|_|_|_|_ _
// 0 1 2 3 4 5 6 7 8 9 10 11
//
// - byte 0-1   = Font Category/Real-World Country Name e.g. cl (conlang), eu (Euro), as (Asia) etc.
// - byte 2-6   = Typeface Name (flexible range = start byte 3 then chop 4 bytes off end!)
// - byte 7     = Font Hierarchy e.g. H (Heading), B (Body), U (UI)
// - byte 8     = Font Flesh e.g. S (Serif), P (San-Serif/Plain), L (Slab Serif), H (Handwritten), M (Monospaced), C (Script)
// - byte 9     = Form Model e.g. D (Dynamic), R (Rational), G (Geometric)
// - byte 10-11 = Variant/Weight Code e.g. a1, b1, c1 etc (instead of 00, 01, 02 etc), nicer to view for artist
//
// NOTE(chowie): byte 8-9 is exists to know how to match the pairing
//

//
// SOUND BANK RADIX40 ENCODING:
//
// - 12 char max
//
// _ _|_ _ _ _ _|_|_|_|_ _
// 0 1 2 3 4 5 6 7 8 9 10 11
//
// - byte 0-1   = Sound Bank Category Name e.g. We (Weather), So (Soundscape), Mu (Music Score), Mo (Movement e.g. footsteps), Do (Doors), An (Animal), Am (Ambience), Sp (Speech), Sc (Script), UI (UI), Co (Collision), Ms (Misc.)
// - byte 2-6   = Sound Name (flexible range = start byte 3 then chop 4 bytes off end!)
// - byte 7     = Spatial Type e.g. 0 (None/Mono), I (Indoor), O (Outdoor), S (Semi-enclosed/Threshold spaces e.g. porches, verandahs)
// - byte 8     = Spatial Distance Type e.g. 0 (None/Mono), C (Close), R (Rear), X (Far), D (Distant), F (Front), B (Back)
// - byte 9     = Sound Trigger e.g. 0 (None, Immediate), T (on proximity, entity/mob trigger), N (animated autonomously = waits for nature/time e.g. tree shrivelling), F (Fire), R (Rain), W (Water), O (Oxygen), D (Damage), I (Illness)
// - byte 10-11 = Variant Code e.g. ll (ll for looped), otherwise default to a1, b1, c1 etc
//
// TODO(chowie): Can work with AABB and/or spherical collision, might
// need LOD if box is large?
// TODO(chowie): A* back to player to find if sounds should hit the
// player or occluded.
// NOTE(chowie): Typically you need a string bank
//

// TODO(chowie): Properly parse string
// TODO(chowie): Autoconvert to bitset with preprocessor?

// IMPORTANT(chowie): Allows mix and match words of different conlang
// families together
enum conlang_family_type : u8
{
    Conlang_Family_None,

    Conlang_Family_Base10,
    Conlang_Family_Eurolang,
    Conlang_Family_Asialang,
    Conlang_Family_Calclang,

    Conlang_Count,
};

// NOTE(chowie): POS = Parts of Speech
// IMPORTANT(chowie): Only __one__ primary tag can be active on a
// word's definition. Although, words can have multiple primary tags
// only one of its definition can be active (in text).
enum conlang_pos_tag : u8
{
    Conlang_POS_Invalid, // NOTE(chowie): Word not found/incomplete (still accepts string)

    Conlang_POS_N_, // Noun
    Conlang_POS_V_, // Verb
    Conlang_POS_Q_, // Question
    Conlang_POS_Cl, // Classifier
    Conlang_POS_Pt, // Particle
    Conlang_POS_Cj, // Conjunction
    Conlang_POS_Pc, // Place
    Conlang_POS_Pp, // Preposition
    Conlang_POS_Dt, // Time
    Conlang_POS_Em, // Emotion
    Conlang_POS_On, // Onomatpoeia
    Conlang_POS_Nm, // Numeral

    Conlang_POS_Count,
};

// TODO(chowie): Why does u64 bitflag doesn't work?
// TODO(chowie): Try ryan fleury's tagging system
// TODO(chowie): Run this via the game's dialogue world-state belief system like Inkle?
// IMPORTANT(chowie): Conlangs can have as many flags/subtags, but
// aren't encoded in the word. Must search hash table! These tags aids
// game to evalute how to interpret a word's meaning/intent
enum conlang_pos_flag : u64
{
    Conlang_POSF_Cmd = BitSet(0), // Command
    Conlang_POSF_Pl = BitSet(1), // Plural
    Conlang_POSF_Neg = BitSet(2), // Negation
    Conlang_POSF_Name = BitSet(3), // Name
    Conlang_POSF_Arch = BitSet(4), // Archiac

    Conlang_POSF_Fml = BitSet(5), // Formal
    Conlang_POSF_Col = BitSet(6), // Collective Noun
    Conlang_POSF_Deg = BitSet(7), // Numeric Degree
    Conlang_POSF_Perf = BitSet(8), // Performance (Encouragement)
    Conlang_POSF_Rep = BitSet(9), // Repetition
    Conlang_POSF_Fill = BitSet(10), // Filler words e.g. hm, um etc
    Conlang_POSF_Soc = BitSet(11), // Social words
    Conlang_POSF_Cau = BitSet(12), // Caution
    Conlang_POSF_Move = BitSet(13), // Motion
    Conlang_POSF_Rest = BitSet(14), // Rest
    Conlang_POSF_Aff = BitSet(15), // Affect
    Conlang_POSF_Met = BitSet(16), // Meterological
    Conlang_POSF_Bcpl = BitSet(17), // Body Corporeal
    Conlang_POSF_Give = BitSet(18), // Giving
    Conlang_POSF_Att = BitSet(19), // Attention
    Conlang_POSF_Spk = BitSet(20), // Speaking
    Conlang_POSF_Hmm = BitSet(21), // Thinking
    Conlang_POSF_Thx = BitSet(22), // Thanks
    Conlang_POSF_Lik = BitSet(23), // Like
    Conlang_POSF_Pref = BitSet(24), // Preference
    Conlang_POSF_Fun = BitSet(25), // Amusement
    Conlang_POSF_Comp = BitSet(26), // Competing
    Conlang_POSF_Phr = BitSet(27), // Phrase
    Conlang_POSF_Interj = BitSet(28), // Interjection

    Conlang_POSF_Hedge = BitSet(29), // Hedging/Guessing/Predicting
//    Conlang_POSF_Phatic = BitSet(30), //
//    Conlang_POSF_Interr = BitSet(31), //
//    Conlang_POSF_Fact = BitSet(32), //
//    Conlang_POSF_Impera = BitSet(33), //
//    Conlang_POSF_Hypo = BitSet(34), //
//    Conlang_POSF_ = BitSet(), //
};

struct conlang_radix40
{
    u8 Family;
    u8 POS;
    string Word; // NOTE(chowie): Max 10 char
};

internal conlang_radix40
ConlangRadix40(u8 Family, u8 POS, string Source)
{
    Assert(Source.Size <= 10);
    conlang_radix40 Result = {Radix40ToAscii.Data[Family], Radix40ToAscii.Data[POS], Source};
    return(Result);
}

internal string
InitConlangRadix40(string Source, u8 Family = 0, u8 POS = 0)
{
    Assert(Source.Size != 11);

    string Result = Source;
    if(Result.Size <= 10)
    {
        conlang_radix40 Radix40 = ConlangRadix40(Conlang_Family_Eurolang, Conlang_POS_Nm,
                                                 WrapZ("chrisgreen"));
        // COULDDO(chowie): Can I simplify this step (concat to convert to string)?
        // Hopefully this should get covered by the gap buffer?
        Result = WrapZ(d7sam_concat((char)Radix40.Family)((char)Radix40.POS)((char *)Radix40.Word.Data));
    }

    return(Result);
}

// TODO(chowie): History chain word list + random fair shuffle?
//    = RESOURCE(): Shuffling a POSET - https://www.youtube.com/watch?v=dr-jUCelobk
//      > Topological sort in two passes odd and even in pairs
//      > Useful for randomising a list fairly while still mailing certain ordered rules.
//      > Might be useful for language learning e.g. transitive verbs are tested before intransitive!
//      > Even/Odd int http://marc-b-reynolds.github.io/math/2022/12/16/ParityWalks.html
//      > https://marc-b-reynolds.github.io/math/2022/01/28/RNGParity.html
//        ~"u  |= 1; return u ^ (u >> 1);" = "return u ^ rotl(u,1)" // 3u
//        ~"u <<= 1; return u ^ (u >> 1);" = "return u ^ rotl(u,1) ^ 1" // 3u + 1

constexpr s32
ToRadix40(char c)
{
    // NOTE(chowie): Normal order, 0-9, Special char, a-z.
    // Below is optimised by most frequent/typeable on a keyboard.

    s32 Result = 0;
    if((c >= 'a') && (c <= 'z'))
    {
        Result = 1 + 10 + 1 + (c - 'a');
    }
    else if((c >= 'A') && (c <= 'Z'))
    {
        Result = 1 + 10 + 1 + (c - 'A');
    }
    else if(( c >= '0' ) && ( c <= '9' ))
    {
        Result = 1 + c - '0';
    }
    else
    {
        // NOTE(chowie): Offsets 1 + 10
        switch(c)
        {
            case '(':
            {
                Result = 1;
            } break;

            case '!':
            {
                Result = 1 + 1;
            } break;

            case '[':
            {
                Result = 1 + 2;
            } break;

            case '{':
            {
                Result = 1 + 3;
            } break;

            case ',':
            {
                Result = 1 + 4;
            } break;

            case '%':
            {
                Result = 1 + 5;
            } break;

            case '}':
            {
                Result = 1 + 6;
            } break;

            case ']':
            {
                Result = 1 + 7;
            } break;

            case '?':
            {
                Result = 1 + 8;
            } break;

            case ')':
            {
                Result = 1 + 9;
            } break;

            case '_':
            {
                Result = 1 + 10;
            } break;
        };
    }

    return(Result);
}

internal u64
ToRadix40(string String)
{
    Assert(String.Size <= 12);

    u64 Result = 0;
    for(u8 *Scan = String.Data;
        *Scan;
        Scan++)
    {
        Result = 40*Result + ToRadix40(*Scan);
    }

    return(Result);
}

internal string
FromRadix40(u64 Radix, string Source, string Ref = Radix40ToAscii) // sym_type Radix
{
    Assert(Radix != 0);
    Assert(Source.Size >= 15);

    string Result = Source;
    *(--Result.Data) = '\0'; // TODO(chowie): Unnecessary? To remove because of pascal-style strings
    while(Radix)
    {
        *(--Result.Data) = Ref.Data[Radix % 40];
        Radix /= 40;
    }

    if(Ref.Data == Radix40ToAscii.Data)
    {
        *(--Result.Data) = '$'; // COULDDO(chowie): Optional symbol indicator, "sym_"?
    }

    return(Result);
}

//
//
//

// RESOURCE: https://www.youtube.com/watch?v=agUiYkvkoVg
// https://0xkiire.com/text_editor_data_structures1/
// TODO(chowie): Implement ropes with unicode!

// RESOURCE(): https://web.archive.org/web/20200917053102/https://github.com/RandyGaul/cute_headers/blob/master/cute_utf.h
// Want to convert between utf-8 and utf-16!
// IMPORTANT(chowie): TODO(chowie): Above library is unicode for game localization specifically!

struct gap_buffer
{
    buffer Buffer;
    umm Start;
    umm End;
    // TODO(chowie): Include memory arena
};

internal umm
GapSize(gap_buffer *GapBuffer)
{
    umm Result = GapBuffer->End - GapBuffer->Start;
    return(Result);
}

internal umm
GapLen(gap_buffer *GapBuffer)
{
    umm Result = GapBuffer->Buffer.Size - GapSize(GapBuffer);
    return(Result);
}

#define INITIAL_GAP 16

internal gap_buffer *
InitGap(void)
{
    gap_buffer *GapBuffer = (gap_buffer *)malloc(sizeof(gap_buffer));
    GapBuffer->Buffer.Size = INITIAL_GAP;
    GapBuffer->Buffer.Data = (u8 *)malloc(GapBuffer->Buffer.Size);
    GapBuffer->Start = 0;
    GapBuffer->End = GapBuffer->Buffer.Size;
    return(GapBuffer);
}

internal gap_buffer *
InitGapWrapZ(char *String)
{
    gap_buffer *GapBuffer = InitGap();
    while(*String)
    {
        GapBuffer->Buffer.Data[GapBuffer->Start++] = *String++;
    }

    return(GapBuffer);
}

internal void
ShiftGapTo(gap_buffer *GapBuffer, umm Cursor)
{
    Assert(Cursor >= 0 && Cursor <= GapLen(GapBuffer));

    if(Cursor < GapBuffer->Start)
    {
        umm Delta = GapBuffer->Start - Cursor;
        memmove(GapBuffer->Buffer.Data + GapBuffer->End - Delta,
                GapBuffer->Buffer.Data + Cursor,
                Delta);
        GapBuffer->Start -= Delta;
        GapBuffer->End -= Delta;
    }
    else if(Cursor > GapBuffer->Start)
    {
        umm Delta = Cursor - GapBuffer->Start;
        memmove(GapBuffer->Buffer.Data + GapBuffer->Start,
                GapBuffer->Buffer.Data + GapBuffer->End,
                Delta);
        GapBuffer->Start += Delta;
        GapBuffer->End += Delta;
    }
}

// NOTE(chowie): Gap shrinks, cursor advances
internal void
InsertCharInternal(gap_buffer *GapBuffer, char C)
{
    GapBuffer->Buffer.Data[GapBuffer->Start++] = C;
}

internal void
InsertChar(gap_buffer *GapBuffer, umm Cursor, char C)
{
    ShiftGapTo(GapBuffer, Cursor);
    InsertCharInternal(GapBuffer, C);
}

// COULDDO(chowie): Implement gap buffer replace string?
// TODO(chowie): Double check replace char works
internal void
ReplaceChar(gap_buffer *GapBuffer, umm Cursor, char C)
{
    Assert(Cursor >= 0 && Cursor <= GapLen(GapBuffer));

    GapBuffer->Buffer.Data[Cursor] = C;
}

internal void
InsertString(gap_buffer *GapBuffer, umm Cursor, char *String)
{
    ShiftGapTo(GapBuffer, Cursor);
    while(*String)
    {
        InsertCharInternal(GapBuffer, *String++);
    }
}

internal void
GapDelete(gap_buffer *GapBuffer, umm Cursor, u32 Count = 1, b32x Pre = true)
{
    ShiftGapTo(GapBuffer, Cursor);
    if(Pre)
    {
        if((GapBuffer->Start + Count) > 0)
        {
            GapBuffer->Start -= Count; // NOTE(chowie): Expand gap to left
        }
    }
    else
    {
        // NOTE(chowie): Post
        if((GapBuffer->End + Count) < GapBuffer->Buffer.Size)
        {
            GapBuffer->End += Count; // NOTE(chowie): Expand gap to right
        }
    }
}

// NOTE(chowie): Random read
internal char
GetGapBuffer(gap_buffer *GapBuffer, umm Cursor)
{
    Assert(Cursor >= 0 && Cursor <= GapLen(GapBuffer));
    char Result = 0;
    if(Cursor < GapBuffer->Start)
    {
        Result = GapBuffer->Buffer.Data[Cursor];
    }
    else
    {
        Result = GapBuffer->Buffer.Data[GapBuffer->End + (Cursor - GapBuffer->Start)];
    }

    return(Result);
}

internal char *
GapBufferToString(gap_buffer *GapBuffer)
{
    umm Len = GapLen(GapBuffer);
    char *Result = (char *)malloc(Len + 1);

    memcpy(Result, GapBuffer->Buffer.Data, GapBuffer->Start);
    umm Right = GapBuffer->Buffer.Size - GapBuffer->End;
    memcpy(Result + GapBuffer->Start, GapBuffer->Buffer.Data + GapBuffer->End, Right);
    Result[Len] = '\0';

    return(Result);
}

internal void
GapBufferDump(gap_buffer *GapBuffer)
{
    char TextBuffer[64];
    _snprintf_s(TextBuffer, sizeof(TextBuffer),
                "capacity=%d gap=[%d,%d) len=%d cursor=%d\n  [",
                GapBuffer->Buffer.Size, GapBuffer->Start, GapBuffer->End,
                GapLen(GapBuffer), GapBuffer->Start);
    OutputDebugStringA(TextBuffer);

    foreachN(s32, i, GapBuffer->Buffer.Size)
    {
        if(i == GapBuffer->Start)
        {
            char TextBufferHeader[8];
            _snprintf_s(TextBufferHeader, sizeof(TextBufferHeader),
                        " |GAP| ");
            OutputDebugStringA(TextBufferHeader);
        }

        if(i >= GapBuffer->Start && i < GapBuffer->End)
        {
            char TextBufferHeader[4];
            _snprintf_s(TextBufferHeader, sizeof(TextBufferHeader),
                        "_ ");
            OutputDebugStringA(TextBufferHeader);
        }
        else
        {
            char TextBufferHeader[64];
            _snprintf_s(TextBufferHeader, sizeof(TextBufferHeader),
                        "%c", GapBuffer->Buffer.Data[i]);
            OutputDebugStringA(TextBufferHeader);
        }
    }

    char TextBufferHeader[4];
    _snprintf_s(TextBufferHeader, sizeof(TextBufferHeader),
                "]\n");
    OutputDebugStringA(TextBufferHeader);
}

// TODO(chowie): Abstract the text buffer to switch between gap buffers + ropes
// RESOURCE: https://www.youtube.com/watch?v=_8tVObGKQ3k
struct text_buffer
{
    gap_buffer GapBuffer;
    umm Cursor;

    u32 LineCount;
    u32 *Lines;
};

// COULDDO(chowie): Clean-up? O(n) for now!
internal void
CalcLines()
{
}

//
//
//

// RESOURCE(): http://0x80.pl/notesen/2023-11-20-popcount-suggestions.html
// IMPORTANT(chowie): Alternative to Levenshtein Distance for fuzzy match!
internal void
ByteHistogram(u32 *Histogram, string String)
{
    foreachN(u32, i, String.Size)
    {
        u32 b = String.Data[i];
        Histogram[b]++;
    }
}

// NOTE(chowie): Byte Diff
#define ASCII_BYTE_HISTOGRAM_MAX 128
internal u32
FuzzyMatch(u32 *HistogramA, u32 *HistogramB)
{
    u32 Result = 0;
    foreachN(u32, i, ASCII_BYTE_HISTOGRAM_MAX)
    {
        Result += AbsDifferenceClampAboveZero(HistogramA[i], HistogramB[i]);
    }

    return(Result);
}

//
//
//

// RESOURCE(): http://0x80.pl/notesen/2016-11-28-simd-strfind.html#generic-sse-avx2
// COULDDO(chowie): Threshold? 0 = "full" match, 1 = partial match (but accept as "full")
internal umm
Substring(string Source, string Needle) // NOTE(chowie): Needle can be of any size
{
    Assert((Needle.Size > 0) && (Source.Size > 0));

    __m128i First = _mm_set1_epi8(Needle.Data[0]);
    __m128i Last  = _mm_set1_epi8(Needle.Data[Needle.Size - 1]);

    for(umm i = 0;
        i < Source.Size;
        i += 16)
    {
        __m128i FirstBlock = _mm_loadu_si128((__m128i *)(Source.Data + i));
        __m128i LastBlock  = _mm_loadu_si128((__m128i *)(Source.Data + i + Needle.Size - 1));

        __m128i EqFirst = _mm_cmpeq_epi8(First, FirstBlock);
        __m128i EqLast  = _mm_cmpeq_epi8(Last, LastBlock);

        u16 Mask = (u16)_mm_movemask_epi8(_mm_and_si128(EqFirst, EqLast));

        while(Mask != 0)
        {
            bit_scan_result BitPos = FindLeastSignificantBit((u32)Mask);

            if(memcmp(Source.Data + i + (umm)BitPos.Index + 1, Needle.Data + 1, Needle.Size - 2) == 0)
            {
                return(i + (umm)BitPos.Index);
            }

            Mask = (u16)ClearLeftMostSet((u32)Mask);
        }
    }

    // TODO(chowie): Error log
    return(999999);
}

int
main()
{
    gap_buffer *Gap = InitGapWrapZ("Hello world");
    GapBufferDump(Gap);

    InsertString(Gap, 5, " bb");
    GapBufferDump(Gap);

    GapDelete(Gap, GapLen(Gap), 5, true);

    char *s = GapBufferToString(Gap);
    char TextBufferHeader[64];
    _snprintf_s(TextBufferHeader, sizeof(TextBufferHeader),
                "Final: \"%s\"\n", s);
    OutputDebugStringA(TextBufferHeader);

    // TODO(chowie): Cut gap?

    free(Gap->Buffer.Data);
    free(Gap);

    //
    //
    //

    ToLower(s, StringLen(s));
    char LowerTextBuffer[64];
    _snprintf_s(LowerTextBuffer, sizeof(LowerTextBuffer),
                "To Lower Test: \"%s\"\n", s);
    OutputDebugStringA(LowerTextBuffer);

    //
    //
    //

    string Word = CONSTANT_STRING("rebase");
    string Misspelled = CONSTANT_STRING("revase");

    u32 h1[ASCII_BYTE_HISTOGRAM_MAX] = {};
    ByteHistogram(h1, Word);

    u32 h2[ASCII_BYTE_HISTOGRAM_MAX] = {};
    ByteHistogram(h2, Misspelled);

    u32 Diff = FuzzyMatch(h1, h2);

    char HistogramTextBuffer[32];
    _snprintf_s(HistogramTextBuffer, sizeof(HistogramTextBuffer),
                "Histogram Test: %u\n", Diff);
    OutputDebugStringA(HistogramTextBuffer);
    
    //
    //
    //

    string Needle = CONSTANT_STRING("as");
    umm Found = Substring(Word, Needle);

    char SubstringTextBuffer[128];
    _snprintf_s(SubstringTextBuffer, sizeof(SubstringTextBuffer),
                "Substring Test, found at CursorP: %d\n", Found);
    OutputDebugStringA(SubstringTextBuffer);

    //
    //
    //

    // NOTE(chowie): For long strings only like file paths or asset
    // naming? Otherwise, just use radix string (below)
    // COULDDO(chowie): Figure out if this hash should be used? I
    // believe radix40 is good enough.
    u32 h0Test = Hash("str");
    u32 h1Test = Hash(h0Test, "ing");
    u32 h2Test = Hash("string");

    Assert(h1Test == h2Test);
    CheckHash(h1Test, "string");

    char HashTextBuffer[128];
    _snprintf_s(HashTextBuffer, sizeof(HashTextBuffer),
                "Hash Test, h1: %d and h2: %d\n", h1Test, h2Test);
    OutputDebugStringA(HashTextBuffer);

    switch(h1Test)
    {
        case 0x1C93AFFC: // NOTE(chowie): Example test must convert u32 to hex!
        {
            char SwitchTextBuffer[128];
            _snprintf_s(SwitchTextBuffer, sizeof(SwitchTextBuffer),
                        "Switch Test Success\n");
            OutputDebugStringA(SwitchTextBuffer);
        } break;

        InvalidDefaultCase;
    }

    //
    //
    //

    // RESOURCE(): https://web.archive.org/web/20160207132838/https://github.com/foonathan/string_id
    // You could always do (using a pre-processor):
    // 1) hashed strings (can't retrieve original string value)
    // 2) string interning (only accessible at runtime, not compile time e.g. for switch statements)

    // TODO(chowie): Enforce enum Dictionary Editor vs gapbuffer Player
    // TODO(chowie): Move to using strings as function input
    char *ReconstructString;

    b32x IsEditor = true;
    if(IsEditor)
    {
        conlang_radix40 Radix40 = ConlangRadix40(Conlang_Family_Eurolang, Conlang_POS_Nm,
                                                 WrapZ("chrisgreen"));
        // COULDDO(chowie): Can I skip this step (concat to convert to string)?
        // Hopefully this should get covered by the gap buffer?
        ReconstructString = d7sam_concat((char)Radix40.Family)((char)Radix40.POS)((char *)Radix40.Word.Data);
    }
    else
    {
        ReconstructString = "27chrisgreen";
    }

    string RadixStringTest = WrapZ(ReconstructString);

    // TODO(chowie):
    // - Read in each gapbuff string/line
    // - Convert each to u64 radix
    // - De/Compress array of u64

    u64 nValue = ToRadix40(RadixStringTest);

    // COULDDO(chowie): Remove "char FormatTextBuffer" and use "string FormatTextBuffer"?
    char FormatTextBuffer[15];
    string FromRadixStringTest = FromRadix40(nValue, BundleString(FormatTextBuffer, sizeof(FormatTextBuffer)));

    _snprintf_s(FormatTextBuffer, sizeof(FormatTextBuffer),
                "%s\n", FromRadixStringTest.Data);
    OutputDebugStringA(FormatTextBuffer);

    return(0);
}

/*
// TODO(chowie): I wonder if I should split the maxsize of the buffer inside of the gapbuffer itself?
//global_variable umm BufferPosition;
struct gap_buffer
{
    u8 *Buffer;
    umm Length;
    umm Start;
    umm End; // STUDY(chowie): Could use start and len/gap
    // TODO(chowie): Pass an arena allocator in! Check back in at 15:28!
};

internal umm
GetGapBufferLength(gap_buffer *GapBuffer)
{
    umm Result = Len(GapBuffer) - (GapBuffer->End - GapBuffer->Start); // NOTE(chowie): Does not include gap
    return(Result);
}

#define GapBufferLenWithGap(GapBuffer) (GapBuffer->Length + GapBufferLength)

internal void
GapBufferInit(gap_buffer *GapBuffer, umm Size)
{
    GapBuffer->Buffer = (u8 *)malloc(Size);
    GapBuffer->Length = 0;
    GapBuffer->Start = 0;
    GapBuffer->End = Size;
}

// NOTE(chowie): Moves the Gap to the cursor position. Cursors are clamped [0,n) where n is the filled count of the buffer.
internal void
ShiftGapTo(gap_buffer *GapBuffer, umm Cursor)
{
    umm GapLength = GapBuffer->End - GapBuffer->Start;
    Cursor = Clamp(0, Cursor, GapBuffer->Length - GapLength); // TODO(chowie): Check if clamped is correct?
    //printf("GapLength: %d, Cursor: %d\n", GapLength, Cursor); // NOTE(chowie): Accounts for expansion
    if(Cursor != GapBuffer->Start) // NOTE(chowie): Something inside
    {
        if(GapBuffer->Start < Cursor)
        {
            // NOTE(chowie): Gap is before the cursor
            //   v~~~v
            //[12]              [3456789abc]
            //--------|----------------------------------- Gap is BEFORE Cursor
            //[123456]              [789abc]
            umm Delta = Cursor - GapBuffer->Start;
            memcpy(&GapBuffer->Buffer[GapBuffer->Start], &GapBuffer->Buffer[GapBuffer->End], Delta);
            GapBuffer->Start += Delta; // NOTE(chowie): Both buffers must move to the right
            GapBuffer->End += Delta;
        }
        else if(GapBuffer->Start > Cursor)
        {
            // NOTE(chowie): Gap is after the cursor
            //   v~~~v
            //[123456]              [789abc]
            //---|---------------------------------------- Gap is AFTER Cursor
            //[12]              [3456789abc]
            umm Delta = GapBuffer->Start - Cursor;
            memcpy(&GapBuffer->Buffer[GapBuffer->End - Delta], &GapBuffer->Buffer[GapBuffer->Start - Delta], Delta);
            GapBuffer->Start -= Delta; // NOTE(chowie): Both buffers must move to the left
            GapBuffer->End -= Delta;
        }
    }
}

// NOTE(chowie): Verifies the buffer can hold the needed write. Resizes the array if not. By default doubles array size.
internal void
CheckGapSize(gap_buffer *GapBuffer, umm Required)
{
    umm GapLength = GapBuffer->End - GapBuffer->Start;
    if(GapLength < Required)
    {
        ShiftGapTo(GapBuffer, GapBuffer->Length - GapLength);
        umm RequiredBufferSize = Required + GapBuffer->Length - GapLength; // NOTE(chowie): Optimisation, as you know the gap buffer len is the end size!
        u8 *NewBuffer = (u8 *)malloc((2 * RequiredBufferSize) * sizeof(u8)); // NOTE(chowie): Maximum to account for big writes // TODO(chowie): Check if it's correct! TIMESTAMP: 12:44
        memcpy(NewBuffer, GapBuffer->Buffer, GapBuffer->Length); // TODO(chowie): Check if it's correct! It should only take until the end. Off by one?
        GapBuffer->Length = (2 * RequiredBufferSize);
        free(GapBuffer->Buffer);
        GapBuffer->Buffer = NewBuffer;
        GapBuffer->End = GapBuffer->Length; // NOTE(chowie): All of the good data is in the start-to-middle. IMPORTANT: Gap start does not change here!
    }
}

// NOTE(chowie): Moves the gap to the cursor, then moves the gap pointer beyond count, effectively deleting it.
// NOTE: Do not rely on the gap being 0, remove will leave as-is values behind in the gap  
// IMPORTANT: Does not protect for unicode at present, simply deletes bytes  
internal void
Remove(gap_buffer *GapBuffer, umm Cursor, u32 Count)
{
    u32 Delete = Abs(Count);
    umm NewCursor = Cursor;
    if(Count < 0)
    {
        NewCursor = Max(0, NewCursor - Delete);
    }
    ShiftGapTo(GapBuffer, NewCursor);
    GapBuffer->End = Min(GapBuffer->End + Delete, GapBuffer->Length); // NOTE(chowie): Protect from runoff!
    //--------|-----------------------------------
    //[123456]              [789abc]

    char RemTextLengthBuffer[256];
    _snprintf_s(RemTextLengthBuffer, sizeof(RemTextLengthBuffer),
                "\nInside Gap Buffer Cursor: %d, Contents: %s, Len: %d, Start: %d, End: %d",
                Cursor, GapBuffer->Buffer, GapBuffer->Length, GapBuffer->Start, GapBuffer->End);
    OutputDebugStringA(RemTextLengthBuffer);
}

internal void
InsertChar(gap_buffer *GapBuffer, umm Cursor, u8 Char)
{
    CheckGapSize(GapBuffer, 1);
    ShiftGapTo(GapBuffer, Cursor);
    GapBuffer->Buffer[GapBuffer->Start++] = Char;
    GapBuffer->Length++;

    char AddTextLengthBuffer[256];
    _snprintf_s(AddTextLengthBuffer, sizeof(AddTextLengthBuffer),
                "\nInside Gap Buffer Cursor: %d, Contents: %s, Len: %d, Start: %d, End: %d",
                Cursor, GapBuffer->Buffer, GapBuffer->Length, GapBuffer->Start, GapBuffer->End);
    OutputDebugStringA(AddTextLengthBuffer);
}


// internal void
// InsertString(gap_buffer *GapBuffer, umm Cursor, char *String)
// {
//     CheckGapSize(GapBuffer, StringLen(String));
//     ShiftGapTo(GapBuffer, Cursor);
//     strcpy((char *)GapBuffer->Buffer, String); // TODO(chowie): This is totally not correct!
//     GapBuffer->Start += StringLen(String);
// }
// TODO(chowie): utf-8 support? How does one encode a rune in C? TIMESTAMP: 17:00-18:00
// internal void
// InsertRune(gap_buffer *GapBuffer, umm Cursor, u8 Char)
// {
//     CheckGapSize(GapBuffer, 1);
//     ShiftGapTo(GapBuffer, Cursor);
//     GapBuffer->Buffer[GapBuffer->Start] = Char;
//     GapBuffer->Start += 1;
// }
// 
// // TODO(chowie): How does one insert a string in C? TIMESTAMP: 17:00-18:00
// internal void
// InsertString(gap_buffer *GapBuffer, umm Cursor, char *String)
// {
//     CheckGapSize(GapBuffer, StringLength(String));
//     ShiftGapTo(GapBuffer, Cursor);
//     strcpy(GapBuffer->Buffer, *String); // TODO(chowie): This is totally not correct!
//     GapBuffer->Start += StringLength(String);
// }

int
main(void)
{
    gap_buffer GapBuffer = {};

    GapBufferInit(&GapBuffer, 32);
    InsertChar(&GapBuffer, 0, '0');
    InsertChar(&GapBuffer, 1, '1');
    InsertChar(&GapBuffer, 2, '2');
    InsertChar(&GapBuffer, 3, '3');
    InsertChar(&GapBuffer, 4, '4');
    InsertChar(&GapBuffer, 2, 'A');
    InsertChar(&GapBuffer, 5, '5');

    Remove(&GapBuffer, 2, 2);
    // TODO(chowie): Insertion/Removal of the character is wrong LOL

    //InsertString(&GapBuffer, 4, "Hello");

    return(0);
}
*/

/*
// NOTE(chowie): For scaling with other data types with a union (avoids casting).
typedef struct node_data
{
    s32 Info;
} node_data;

typedef struct node
{
    node_data Data;
    struct node *Next;
} node;

internal node *
Push(node *Node, node_data Data)
{
    node *Result = (node *)malloc(sizeof(node));
    Assert(Result);
    Result->Data = Data;
    Result->Next = Node;
    return(Result);
}

internal node *
Pop(node *Node, node_data *Data)
{
    node *Result = Node;
    if(Result)
    {
        *Data = Node->Data;
        Node = Node->Next;
        free(Result);
    }
    else
    {
        // TODO(chowie): Logging!
    }
    return(Node);
}

internal node_data *
StackTop(node *Node)
{
    return(&Node->Data);
}

internal b32x
StackIsEmpty(node* Node)
{
    return((Node == NULL) ? 1 : 0);
}

int
main(void)
{
    node *Top = {};
    node_data Element;

    Element.Info = 11;
    Top = Push(Top, Element);

    char TestBuffer[256];
    _snprintf_s(TestBuffer, sizeof(TestBuffer),
                "Stack Top: %d\n", StackTop(Top)->Info);
    OutputDebugStringA(TestBuffer);

    Element.Info = 33;
    Top = Push(Top, Element);

    char TestBuffer2[256];
    _snprintf_s(TestBuffer2, sizeof(TestBuffer2),
                "Stack Top2: %d\n", StackTop(Top)->Info);
    OutputDebugStringA(TestBuffer2);

    while(!StackIsEmpty(Top))
    {
        Top = Pop(Top, &Element);

        char PopBuffer[256];
        _snprintf_s(PopBuffer, sizeof(PopBuffer),
                    "Popping: %d\n", Element.Info);
        OutputDebugStringA(PopBuffer);
    }

    return(0);
}
*/

