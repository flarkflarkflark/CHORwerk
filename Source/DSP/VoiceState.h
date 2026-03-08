#pragma once
#include "DelayLine.h"
#include "StateVariableFilter.h"

/**
 * State for a single chorus voice.
 * Each voice has its own delay line, filter state, and modulation targets.
 */
struct VoiceState
{
    // Delay
    DelayLine delayLine;
    float currentDelay = 2.0f;       // Current delay in ms
    float targetDelay = 2.0f;        // Target delay after randomization
    float delayVelocity = 0.0f;      // Rate of delay change (ms/s)

    // Random modulation
    float currentRate = 1.0f;        // Current modulation rate (ms/s)
    float targetRate = 1.0f;         // Target rate after update
    float ratePhase = 0.0f;          // Phase within current rate cycle

    // Filter
    StateVariableFilter filter;

    // Feedback
    float feedbackSample = 0.0f;

    // Stereo
    float panPosition = 0.0f;       // -1 (left) to +1 (right)

    // Timing
    float updateCountdown = 0.0f;    // Samples until next parameter re-randomization

    void prepare (double sampleRate, float maxDelayMs)
    {
        delayLine.prepare (sampleRate, maxDelayMs + 30.0f);  // Extra headroom for modulation
        filter.prepare (sampleRate);
        reset();
    }

    void reset()
    {
        delayLine.reset();
        filter.reset();
        currentDelay = 2.0f;
        targetDelay = 2.0f;
        delayVelocity = 0.0f;
        currentRate = 1.0f;
        targetRate = 1.0f;
        ratePhase = 0.0f;
        feedbackSample = 0.0f;
        panPosition = 0.0f;
        updateCountdown = 0.0f;
    }
};
