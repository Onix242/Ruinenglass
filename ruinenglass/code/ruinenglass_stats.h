#if !defined(RUINENGLASS_STATS_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Michael Chow $
   $Notice: $
   ======================================================================== */

// RESOURCE(): https://www.reddit.com/r/gamedev/comments/wy91pc/what_is_correctly_stats_meaning/
// RESOURCE(): https://www.reddit.com/r/gamedev/comments/wz2f7w/attack_damage_defense_etc_statistics_or_status/
// NOTE(chowie): East Asia uses STATUS and STATUS EFFECT (see Monster
// Hunter), so there's some confusion vs West uses STATS and STATUS
// (see League of Legends)! Abbributes could be clearer. I'm using
// the word, 'stats' as a double entendre for both statistics (debug
// vis) and stats for games (and hopefully status)

//
// General
//

// NOTE(chowie): Don't use this function unless you clamp it
inline f32
PercentTo01(f32 Value)
{
    f32 Result = Value/100.0f;
    return(Result);
}

inline v2
PercentTo01(v2 Value)
{
    v2 Result = Value/100.0f;
    return(Result);
}

// RESOURCE(): Averaging with Lerp - https://blog.demofox.org/2016/08/23/incremental-averaging/
inline f32
IncrementalAvg(f32 Avg, f32 tNewSampleCount, f32 NewValue)
{
    f32 Result = Lerp(Avg, 1.0f/tNewSampleCount, NewValue);
    return(Result);
}

//
// Fairmath
//

// RESOURCE(): https://emshort.blog/2016/02/15/set-check-or-gate-a-problem-in-personality-stats/
// RESOURCE(): https://www.reddit.com/r/choiceofgames/comments/k92tbs/survey_that_no_one_asked_for/
// NOTE(from emshort): Not a good way for a system that rewards the
// extremes to define a character's personality. Averaging systems
// tends to moderates themselves (but could appropriate for others).
// NOTE(chowie): Example use-case is each action is less effective
// than the previous with the same trait/ability. In other words,
// diminishing returns for repeat actions, prevents min/max of stats.

// RESOURCE(): https://web.archive.org/web/20150815012902/http://community.failbettergames.com/topic20369-favours-and-renown.aspx
// RESOURCE(inkle): https://gdcvault.com/play/1021774/Adventures-in-Text-Innovating-in
// COULDDO(chowie): Don't need sub if you're using an always using an
// increasing system like Inkle (choice for impact + easier to debug):
// Tension-Up and Tension-Down are always saved. A renown system is
// always increasing, favours can decrease = exchanging currency.

// RESOURCE(): https://choicescriptdev.fandom.com/wiki/Arithmetic_operators
// RESOURCE(): https://emshort.blog/2016/02/15/set-check-or-gate-a-problem-in-personality-stats/
// NOTE(from cvaneseltine): "The idea of fairmath is the closer to 100
// (the higher it is), the harder it is to increase. The closer to 0
// (the lower it is), the harder it is to decrease." Outputs values
// heavily towards the center! (Never increases past 100 or decreases
// past 1 unless if either are set as the starting value)
// IMPORTANT(chowie): Fairmath is a percentage-based incremental
// averaging, XCOM would tie this to shooting chance and map this to
// distance, setting 100 and 0 manually (for close and long distances
// especially shotguns and snipers)! I think this is where 

struct fairmath
{
    // NOTE(chowie): These are split to lerp if you want to visualise as a bar
    v2 Norm;
};

enum fairmath_op
{
    FairmathOp_Add,
    FairmathOp_Sub,
};

enum fairmath_stat_outcome
{
    StatOutcome_Punchy, // Against Enemies = Lerp(A, B, 1)   for add or A - B*A for sub
    StatOutcome_Large,  // Protect Players = Eerp(A, B, 1)   for add or Eerp(A, -B, 1) for sub
    StatOutcome_Medium, // Protect Players = Eerp(A, B*A, 1) for add or Eerp(A, -B*A, 1) for sub
    StatOutcome_Small,  // Protect Players = Eerp(A, B*B, 1) for add or Eerp(A, -B*B, 1) for sub
};

inline f32
FairmathLerp01(f32 Avg, f32 t)
{
    f32 Result = Lerp(Avg, t, 1);
    return(Result);
}

// NOTE(chowie): Multiplicative scaling for slow starts (hard
// mode). Large gains (t) at <50 avg are tougher/smaller, evens
// out on ~50+ base. More resilient losses overall.
// IMPORTANT(chowie): Eerp is used over lerp since lerp can produce
// negative percentages and eerp generates stable values more often.
inline f32
FairmathEerp01(f32 Avg, f32 t)
{
    f32 Result = Eerp(Avg, t, 1);
    return(Result);
}

inline f32
FairmathFnma(f32 Avg, f32 t)
{
    f32 Result = Avg - t*Avg;
    return(Result);
}

// RESOURCE(): Averaging with Lerp - https://blog.demofox.org/2016/08/23/incremental-averaging/
// IMPORTANT(chowie): Weave in the types of gains you're looking for
internal fairmath
FairmathOpInternal(fairmath_op Op, v2 Value, fairmath_stat_outcome StatOutcome = StatOutcome_Punchy)
{
    fairmath Result = {};

    Result.Norm.Avg = Value.Avg;
    b32x Negative = (Op == FairmathOp_Sub) ? -1 : 1;
    switch(StatOutcome)
    {
        case StatOutcome_Punchy:
        {
            if(Op == FairmathOp_Add)
            {
                // A = Avg | t = tNewSampleCount | NewValue = 1
                // Step 1: A + ((1 - A)*(B/1))
                // Step 2: A + (1 - A)/(1/B)
                // Step 3: A + B*(1 - A) is the same as lerp eq "A + t*(B - A)"
                Result.Norm.t = FairmathLerp01(Value.Avg, Value.t);
            }
            else if(Op == FairmathOp_Sub)
            {
                // A = Avg | t = tNewSampleCount | NewValue = 1
                // Step 1: A - (A*(B/1))
                // Step 2: A - (A/(1/B))
                // Step 3: A - B*A
                Result.Norm.t = FairmathFnma(Value.Avg, Value.t);
            }
            else
            {
                InvalidCodePath;
            }
        } break;

        // NOTE(chowie): Good for impactful negative values, not as debilitating as Punchy
        case StatOutcome_Large:
        {
            Result.Norm.t = FairmathEerp01(Value.Avg, Negative*Value.t);
        } break;

        // NOTE(chowie): Medium/Small have similar values sometimes
        case StatOutcome_Medium:
        {
            Result.Norm.t = FairmathEerp01(Value.Avg, Negative*Square(Value.t));
        } break;

        case StatOutcome_Small:
        {
            Result.Norm.t = FairmathEerp01(Value.Avg, Negative*Value.t*Value.Avg);
        } break;

        InvalidDefaultCase;
    }

    return(Result);
}

// STUDY(chowie): Old method to round decimal floats accurately,
// times float by 100 -> round -> divide back to float
struct fairmath_result
{
    fairmath Percent;
    fairmath Decimal;
};

// NOTE(chowie): Dev-facing 01 v2 - can do this for subsequent times
internal fairmath_result
FairmathOp01(fairmath_op Op, v2 Value, fairmath_stat_outcome StatOutcome = StatOutcome_Punchy)
{
    fairmath FairmathOp = FairmathOpInternal(Op, Value, StatOutcome);

    fairmath_result Result = {};
    Result.Percent.Norm = Round(100.0f*FairmathOp.Norm);
    Result.Decimal.Norm = PercentTo01(Result.Percent.Norm);
    return(Result);
}

// NOTE(chowie): User-facing percent v2 - can do this for the first time
inline fairmath_result
FairmathOp(fairmath_op Op, v2 Value, fairmath_stat_outcome StatOutcome = StatOutcome_Punchy)
{
    fairmath_result Result = FairmathOp01(Op, Clamp01MapToRange(0, Value, 100), StatOutcome);
    return(Result);
}

// NOTE(chowie): Custom range that's _the same_ for both .Avg and .t
inline fairmath_result
FairmathOp(fairmath_op Op, f32 Min, v2 Value, f32 Max, fairmath_stat_outcome StatOutcome = StatOutcome_Punchy)
{
    fairmath_result Result = FairmathOp01(Op, Clamp01MapToRange(Min, Value, Max), StatOutcome);
    return(Result);
}

// NOTE(chowie): Optional subgating/clamp in lerp if you don't want
// the stat gains to be too dramatic e.g. t = max of 20%
// NOTE(chowie): Custom range that's _different_ for both .Avg and .t
inline fairmath_result
FairmathOp(fairmath_op Op, v2 Min, v2 Value, v2 Max, fairmath_stat_outcome StatOutcome = StatOutcome_Punchy)
{
    fairmath_result Result = FairmathOp01(Op, Clamp01MapToRange(Min, Value, Max), StatOutcome);
    return(Result);
}

//  NOTE(chowie): Fairmath Usage:
//  v2 StrengthStat = {60, 20};
//  printf("Lerp  Add: %.f%%\n", FairmathOp(FairmathOp_Add, StrengthStat, StatOutcome_Punchy).Percent.Norm.t);
//  printf("Eerp1 Add: %.f%%\n", FairmathOp(FairmathOp_Add, StrengthStat, StatOutcome_Large).Percent.Norm.t);
//  printf("Eerp2 Add: %.f%%\n", FairmathOp(FairmathOp_Add, StrengthStat, StatOutcome_Medium).Percent.Norm.t);
//  printf("Eerp3 Add: %.f%%\n", FairmathOp(FairmathOp_Add, StrengthStat, StatOutcome_Small).Percent.Norm.t);
//  printf("Lerp  Sub: %.f%%\n", FairmathOp(FairmathOp_Sub, StrengthStat, StatOutcome_Punchy).Percent.Norm.t);
//  printf("Eerp1 Sub: %.f%%\n", FairmathOp(FairmathOp_Sub, StrengthStat, StatOutcome_Large).Percent.Norm.t);
//  printf("Eerp2 Sub: %.f%%\n", FairmathOp(FairmathOp_Sub, StrengthStat, StatOutcome_Medium).Percent.Norm.t);
//  printf("Eerp3 Sub: %.f%%\n", FairmathOp(FairmathOp_Sub, StrengthStat, StatOutcome_Small).Percent.Norm.t);
//  Lerp  Add: 68%
//  Eerp1 Add: 68%
//  Eerp2 Add: 65%
//  Eerp3 Add: 62%
//  Lerp  Sub: 48%
//  Eerp1 Sub: 52%
//  Eerp2 Sub: 55%
//  Eerp3 Sub: 58%

#define RUINENGLASS_STATS_H
#endif
