/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Michael Chow $
   $Notice: $
   ======================================================================== */

internal void
InitialiseAudioState(audio_state *AudioState)
{
    AudioState->tSine1 = 0;
    AudioState->tSine2 = 0;
}

internal void
TestOutputWilwaDialTone(audio_state *AudioState,
                        game_sound_output_buffer *SoundBuffer)
{
    test_output_sound WilwaDialTone = {};
    WilwaDialTone.ToneVolume = 3000;

    wilwa_dial_tone *Wave1 = &WilwaDialTone.Wave1; // STUDY(chowie): Syntactic convience by snapping the pointer
    Wave1->Tag = "Wave 1";
    Wave1->ToneHz = 350;
    Wave1->Period = SoundBuffer->SamplesPerSecond / Wave1->ToneHz;
    wilwa_dial_tone *Wave2 = &WilwaDialTone.Wave2;
    Wave2->Tag = "Wave 2";
    Wave2->ToneHz = 440;
    Wave2->Period = SoundBuffer->SamplesPerSecond / Wave2->ToneHz;

    s16 *SampleOut = SoundBuffer->Samples;
    for(u32 SampleIndex = 0;
        SampleIndex < SoundBuffer->SampleCount;
        ++SampleIndex)
    {
        Wave1->Value = Sin(AudioState->tSine1);
        Wave2->Value = Sin(AudioState->tSine2);

        r32 TotalValue = 0;
        for(u32 ToneIndex = 0;
            ToneIndex < ArrayCount(WilwaDialTone.E); // STUDY(chowie): I'm really proud to be able to convert a range-based for loop into a regular one!
            ++ToneIndex)
        {
            wilwa_dial_tone *Sound = WilwaDialTone.E + ToneIndex;
            TotalValue += Sound->Value;
        }
        s16 SampleValue = (s16)(TotalValue * WilwaDialTone.ToneVolume);
        *SampleOut++ = SampleValue;
        *SampleOut++ = SampleValue;

        // STUDY(chowie): If the ToneHz changes and the period stayed
        // the same, it would produce audio clicks; the wave period
        // must continuous at the point of change. 1.0f Samples
        // increments on sine.
        AudioState->tSine1 += Tau32*1.0f / (r32)Wave1->Period; // NOTE(chowie): 2*Pi is how many wave periods elapsed since we started
        if(AudioState->tSine1 > Tau32)
        {
            AudioState->tSine1 -= Tau32; // NOTE(chowie): Normalising to its period
        }
        AudioState->tSine2 += Tau32*1.0f / (r32)Wave2->Period;
        if(AudioState->tSine2 > Tau32)
        {
            AudioState->tSine2 -= Tau32;
        }
    }
}
