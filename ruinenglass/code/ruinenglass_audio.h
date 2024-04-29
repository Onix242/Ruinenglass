#if !defined(RUINENGLASS_AUDIO_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Michael Chow $
   $Notice: $
   ======================================================================== */

// RESOURCE: https://hero.handmade.network/forums/code-discussion/t/1380-day_20_tiny_bit_of_fun_with_the_code
struct wilwa_dial_tone
{
    char *Tag; // TODO(chowie): Display this?

    s32 Period;
    s32 ToneHz; // NOTE(chowie): For testing 200-500hz is a good range!
    r32 Value; // NOTE(chowie): Sine value
};
struct test_output_sound
{
    s16 ToneVolume;
    FIELD_ARRAY(wilwa_dial_tone,
    {
        wilwa_dial_tone Wave1;
        wilwa_dial_tone Wave2;
    });
};

struct audio_state
{
    r32 tSine1;
    r32 tSine2;
};

#define RUINENGLASS_AUDIO_H
#endif
