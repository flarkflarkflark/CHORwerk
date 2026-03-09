#pragma once
#include <cmath>
#include <algorithm>
#include <juce_audio_processors/juce_audio_processors.h>

/**
 * Hal Chamberlin State Variable Filter.
 * 3-iteration oversampled update for stability.
 */
class StateVariableFilter
{
public:
    void prepare (double sampleRate)
    {
        sr = sampleRate;
        reset();
    }

    void reset()
    {
        lpState = bpState = 0.0f;
        hpPrevIn = 0.0f;
    }

    struct Output
    {
        float lp;
        float hp;
        float bp;
    };

    Output processSample (float input, float lpCutoff, float lpRes, float hpCutoff, float hpRes)
    {
        float lpF = computeF (lpCutoff);
        float hpF = computeF (hpCutoff);
        float lpQ = 1.0f - lpRes * 0.95f;

        for (int iter = 0; iter < 3; ++iter)
        {
            float lpHP = input - lpState - bpState * lpQ;
            bpState += lpF * lpHP;
            lpState += lpF * bpState;
        }

        float hpFiltered = input;
        if (hpCutoff > 0.001f)
        {
            float alpha = 1.0f - hpF;
            hpPrevIn = alpha * hpPrevIn + (1.0f - alpha) * lpState;
            hpFiltered = lpState - hpPrevIn;

            if (hpRes > 0.001f)
                hpFiltered += (hpFiltered - lpState) * hpRes * 0.5f;
        }
        else
        {
            hpFiltered = lpState;
        }

        return { lpState, hpFiltered, bpState };
    }

    float process (float input, float lpCutoff, float lpRes, float hpCutoff, float hpRes)
    {
        auto out = processSample (input, lpCutoff, lpRes, hpCutoff, hpRes);
        return (hpCutoff > 0.001f) ? out.hp : out.lp;
    }

private:
    float computeF (float normalized) const
    {
        float freq = 20.0f * std::pow (1000.0f, std::clamp (normalized, 0.0f, 1.0f));
        float f = 2.0f * std::sin (juce::MathConstants<float>::pi * freq / static_cast<float> (sr));
        return std::clamp (f, 0.0f, 0.95f);
    }

    double sr = 44100.0;
    float lpState = 0.0f;
    float bpState = 0.0f;
    float hpPrevIn = 0.0f;
};
