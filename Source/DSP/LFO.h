#pragma once
#include <cmath>
#include <array>
#include <random>
#include <juce_core/juce_core.h>

/**
 * LFO matching original Charsiesis shapes:
 * Sine, Ramp, Triangle, Square, Stepping Random, Smooth Random, User (16-point)
 */
class LFO
{
public:
    enum class Shape { Sine, Ramp, Triangle, Square, SteppingRandom, SmoothRandom, User };

    void prepare (double sampleRate)
    {
        sr = sampleRate;
        phase = 0.0;
        randomCurrent = 0.0f;
        randomTarget = 0.0f;
        rng.seed (42);
    }

    void reset()
    {
        phase = 0.0;
        randomCurrent = 0.0f;
        randomTarget = 0.0f;
    }

    void setShape (Shape s) { shape = s; }
    void setFrequencyHz (float hz) { freqHz = hz; }

    /** Set user waveform (16 points, each 0–1). */
    void setUserWaveform (const std::array<float, 16>& waveform)
    {
        userWaveform = waveform;
    }

    /** Advance phase and return LFO value in range -1 to +1. */
    float processSample()
    {
        double phaseInc = static_cast<double> (freqHz) / sr;
        phase += phaseInc;

        if (phase >= 1.0)
        {
            phase -= 1.0;
            onCycleWrap();
        }

        return computeValue (static_cast<float> (phase));
    }

    /** Get current value without advancing (for display). */
    float getCurrentValue() const
    {
        return computeValue (static_cast<float> (phase));
    }

    float getPhase() const { return static_cast<float> (phase); }

private:
    float computeValue (float p) const
    {
        switch (shape)
        {
            case Shape::Sine:
                return std::sin (p * juce::MathConstants<float>::twoPi);

            case Shape::Ramp:
                return 2.0f * p - 1.0f;

            case Shape::Triangle:
                return (p < 0.5f) ? (4.0f * p - 1.0f) : (3.0f - 4.0f * p);

            case Shape::Square:
                return (p < 0.5f) ? 1.0f : -1.0f;

            case Shape::SteppingRandom:
                return randomCurrent;

            case Shape::SmoothRandom:
            {
                // Smooth interpolation between random targets
                float t = p;  // within current cycle
                return randomPrev + (randomCurrent - randomPrev) * t;
            }

            case Shape::User:
            {
                // 16-point interpolated lookup
                float pos = p * 16.0f;
                int idx = static_cast<int> (pos);
                float frac = pos - static_cast<float> (idx);
                idx = idx % 16;
                int next = (idx + 1) % 16;
                float val = userWaveform[static_cast<size_t> (idx)] * (1.0f - frac)
                          + userWaveform[static_cast<size_t> (next)] * frac;
                return val * 2.0f - 1.0f;  // scale to -1..+1
            }

            default:
                return 0.0f;
        }
    }

    void onCycleWrap()
    {
        // Generate new random targets on cycle boundaries
        std::uniform_real_distribution<float> dist (-1.0f, 1.0f);
        randomPrev = randomCurrent;
        randomCurrent = randomTarget;
        randomTarget = dist (rng);
    }

    double sr = 44100.0;
    double phase = 0.0;
    float freqHz = 1.0f;
    Shape shape = Shape::Sine;

    // Random state
    float randomCurrent = 0.0f;
    float randomTarget = 0.0f;
    float randomPrev = 0.0f;
    std::mt19937 rng;

    // User waveform (16 points)
    std::array<float, 16> userWaveform {};
};
