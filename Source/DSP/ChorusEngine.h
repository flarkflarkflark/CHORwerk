#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include <array>
#include <random>

#include "VoiceState.h"
#include "EnvelopeFollower.h"
#include "LFO.h"
#include "StepSequencer.h"
#include "TempoSync.h"
#include "../Parameters.h"

/**
 * Core CHORwerk multi-voice chorus engine.
 *
 * Architecture (reverse-engineered from original DLL):
 *
 *   Input → Pregain
 *         → Per-Voice:
 *             Delay Line (modulated by random rate + LFO + seq + env)
 *             → State Variable Filter (LP+HP, cutoff modulated)
 *             → Gain (distribution curve)
 *             → Feedback → Rotation
 *         → Sum all voices
 *         → Postgain
 *         → Mix (dry/wet)
 *         → Output
 */
class ChorusEngine
{
public:
    static constexpr int MaxVoices = 8;
    static constexpr float MaxDelayMs = 50.0f;  // Total max including modulation

    ChorusEngine();
    ~ChorusEngine() = default;

    void prepare (double sampleRate, int samplesPerBlock);
    void reset();

    /** Process a stereo buffer in-place. */
    void process (juce::AudioBuffer<float>& buffer,
                  juce::AudioPlayHead* playHead,
                  juce::AudioProcessorValueTreeState& apvts);

    // Accessors for UI visualization
    const LFO& getLFO() const { return lfo; }
    const StepSequencer& getSequencer() const { return sequencer; }
    float getEnvelopeLevel() const { return envLevel; }
    int getActiveVoices() const { return numVoices; }

    /** Get current delay time for a voice (for waveform display). */
    float getVoiceDelay (int voice) const
    {
        if (voice >= 0 && voice < MaxVoices)
            return voices[static_cast<size_t> (voice)].currentDelay;
        return 0.0f;
    }

private:
    void updateVoiceParameters (int voiceIdx, float bpm);
    void randomizeVoice (int voiceIdx);
    float computeGainDistribution (int voiceIdx, int totalVoices) const;

    double sampleRate = 44100.0;
    int blockSize = 512;

    // Voices
    std::array<VoiceState, MaxVoices> voices;
    int numVoices = 4;

    // Global modulation
    LFO lfo;
    StepSequencer sequencer;
    EnvelopeFollower envFollowerL, envFollowerR;
    float envLevel = 0.0f;

    // Random engine
    std::mt19937 rng;
    std::uniform_real_distribution<float> dist { 0.0f, 1.0f };

    // Smoothed parameters (to avoid zipper noise)
    juce::SmoothedValue<float> smoothMix;
    juce::SmoothedValue<float> smoothFeedback;
    juce::SmoothedValue<float> smoothRotation;
    juce::SmoothedValue<float> smoothPregain;
    juce::SmoothedValue<float> smoothPostgain;
    juce::SmoothedValue<float> smoothLPCutoff;
    juce::SmoothedValue<float> smoothHPCutoff;
    juce::SmoothedValue<float> smoothLPRes;
    juce::SmoothedValue<float> smoothHPRes;

    // Cached parameter values
    float minRate = 1.0f, rateRange = 2.0f;
    float minDelay = 2.0f, delayRange = 5.0f;
    float rateUpdatePeriodMs = 1000.0f;
    StereoMode stereoMode = StereoMode::Free;
    FollowMode followMode = FollowMode::Off;
    bool updateOnCollision = false;

    // Safety
    static inline float sanitize (float v) 
    {
        if (! std::isfinite (v)) return 0.0f;
        return std::clamp (v, -2.0f, 2.0f); 
    }

    static inline float softClip (float x)
    {
        if (x <= -1.0f) return -1.0f;
        if (x >= 1.0f) return 1.0f;
        return x - (x * x * x) / 3.0f;
    }

    // Pre-allocated wet buffer
    juce::AudioBuffer<float> wetBuffer;
    float lastWetL = 0.0f, lastWetR = 0.0f;

    // Modulation depths
    float envToLP = 0.0f, envToHP = 0.0f, envToDelay = 0.0f;
    float lfoToLP = 0.0f, lfoToHP = 0.0f, lfoToDelay = 0.0f;
    float seqToLP = 0.0f, seqToHP = 0.0f, seqToDelay = 0.0f;
};
