#pragma once
#include <cmath>
#include <algorithm>

/**
 * Envelope follower with 4 modes matching original:
 * Off, Slow Decay, Normal Rate, Fast Decay
 */
class EnvelopeFollower
{
public:
    enum class Mode { Off, SlowDecay, NormalRate, FastDecay };

    void prepare (double sampleRate)
    {
        sr = sampleRate;
        envelope = 0.0f;
    }

    void reset() { envelope = 0.0f; }

    void setMode (Mode m) { mode = m; }

    /** Process one sample, returns envelope value 0–1. */
    float processSample (float input)
    {
        float absInput = std::abs (input);

        // Attack is always fast
        float attackCoeff = 1.0f - std::exp (-1.0f / (static_cast<float> (sr) * 0.001f));

        // Decay depends on mode for modulation, but we always have a base decay for the meter
        float decayTime = 0.1f; // Default for meter
        if (mode == Mode::SlowDecay)  decayTime = 0.5f;
        else if (mode == Mode::NormalRate) decayTime = 0.1f;
        else if (mode == Mode::FastDecay)  decayTime = 0.02f;
        
        float decayCoeff = 1.0f - std::exp (-1.0f / (static_cast<float> (sr) * decayTime));

        if (absInput > envelope)
            envelope += attackCoeff * (absInput - envelope);
        else
            envelope += decayCoeff * (absInput - envelope);

        // If mode is off, the meter still moves but we return 0 for modulation
        if (mode == Mode::Off)
            return 0.0f;

        return std::clamp (envelope, 0.0f, 1.0f);
    }

    /** Always return the tracking level for the meter, regardless of mode. */
    float getLevel() const { return std::clamp (envelope, 0.0f, 1.0f); }

private:
    double sr = 44100.0;
    float envelope = 0.0f;
    Mode mode = Mode::Off;
};
