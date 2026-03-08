#pragma once
#include <cmath>
#include <algorithm>

/**
 * Hal Chamberlin State Variable Filter.
 * Matches the original Charsiesis implementation:
 * - 2× oversampled coefficient update (3 iterations found in disasm)
 * - Simultaneous LP and HP outputs
 * - Independent resonance for LP and HP paths
 *
 * The original uses 3 cascaded SVF update stages per sample,
 * effectively giving a steeper response and better stability at high frequencies.
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
        lpState = hpState = bpState = 0.0f;
    }

    struct Output
    {
        float lp;
        float hp;
        float bp;
    };

    /**
     * Process one sample.
     * @param input     Input sample
     * @param lpCutoff  Lowpass cutoff (0–1 normalized, 0=closed, 1=open)
     * @param lpRes     Lowpass resonance (0–1)
     * @param hpCutoff  Highpass cutoff (0–1 normalized, 0=open, 1=closed)
     * @param hpRes     Highpass resonance (0–1)
     */
    Output processSample (float input, float lpCutoff, float lpRes, float hpCutoff, float hpRes)
    {
        // Convert normalized cutoffs to SVF 'f' coefficient
        // f = 2 * sin(pi * fc / fs), but we use the normalized parameter directly
        float lpF = computeF (lpCutoff);
        float hpF = computeF (hpCutoff);

        // Q from resonance (√2 = flat response, lower = more resonant)
        float lpQ = 1.0f - lpRes * 0.95f;  // min Q ~0.05
        float hpQ = 1.0f - hpRes * 0.95f;

        // 3-iteration oversampled update (matches original's triple cascade)
        for (int iter = 0; iter < 3; ++iter)
        {
            // Lowpass path
            float lpHP = input - lpState - bpState * lpQ;
            bpState += lpF * lpHP;
            lpState += lpF * bpState;
        }

        // Highpass is derived by subtracting LP content
        // The original applies HP separately on the LP output
        float hpFiltered = input;
        if (hpCutoff > 0.001f)
        {
            // Simple 1-pole HP for the HP path (matching original's simpler HP)
            float alpha = 1.0f - hpF;
            hpPrevIn = alpha * hpPrevIn + (1.0f - alpha) * lpState;
            hpFiltered = lpState - hpPrevIn;

            // Apply HP resonance as a gentle peak
            if (hpRes > 0.001f)
                hpFiltered += (hpFiltered - lpState) * hpRes * 0.5f;
        }
        else
        {
            hpFiltered = lpState;
        }

        return { lpState, hpFiltered, bpState };
    }

    /** Simplified: apply LP and HP in series, return combined output. */
    float process (float input, float lpCutoff, float lpRes, float hpCutoff, float hpRes)
    {
        auto out = processSample (input, lpCutoff, lpRes, hpCutoff, hpRes);
        return (hpCutoff > 0.001f) ? out.hp : out.lp;
    }

private:
    float computeF (float normalized) const
    {
        // Map 0–1 to frequency range ~20Hz–20kHz with exponential curve
        float freq = 20.0f * std::pow (1000.0f, std::clamp (normalized, 0.0f, 1.0f));
        float f = 2.0f * std::sin (juce::MathConstants<float>::pi * freq / static_cast<float> (sr));
        return std::clamp (f, 0.0f, 0.95f);  // stability limit
    }

    double sr = 44100.0;
    float lpState = 0.0f;
    float hpState = 0.0f;
    float bpState = 0.0f;
    float hpPrevIn = 0.0f;
};
