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
        if (mode == Mode::Off)
            return 0.0f;

        float absInput = std::abs (input);

        // Attack is always fast
        float attackCoeff = 1.0f - std::exp (-1.0f / (static_cast<float> (sr) * 0.001f));

        // Decay depends on mode
        float decayTime;
        switch (mode)
        {
            case Mode::SlowDecay:  decayTime = 0.5f;  break;  // 500ms
            case Mode::NormalRate: decayTime = 0.1f;  break;  // 100ms
            case Mode::FastDecay:  decayTime = 0.02f; break;  // 20ms
            default:               return 0.0f;
        }
        float decayCoeff = 1.0f - std::exp (-1.0f / (static_cast<float> (sr) * decayTime));

        if (absInput > envelope)
            envelope += attackCoeff * (absInput - envelope);
        else
            envelope += decayCoeff * (absInput - envelope);

        return std::clamp (envelope, 0.0f, 1.0f);
    }

private:
    double sr = 44100.0;
    float envelope = 0.0f;
    Mode mode = Mode::Off;
};
