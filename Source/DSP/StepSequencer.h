#pragma once
#include <array>
#include <cmath>

/**
 * 8-step sequencer with interpolation between steps.
 * Outputs value in -1 to +1 range (step values are 0–1, centered at 0.5).
 */
class StepSequencer
{
public:
    static constexpr int NumSteps = 8;

    void prepare (double sampleRate)
    {
        sr = sampleRate;
        phase = 0.0;
        currentStep = 0;
    }

    void reset()
    {
        phase = 0.0;
        currentStep = 0;
    }

    void setStepValue (int step, float value)
    {
        if (step >= 0 && step < NumSteps)
            steps[static_cast<size_t> (step)] = value;
    }

    void setStepRateHz (float hz) { stepRateHz = hz; }

    /** Process one sample. Returns interpolated value in -1..+1. */
    float processSample()
    {
        double phaseInc = static_cast<double> (stepRateHz) / sr;
        phase += phaseInc;

        if (phase >= 1.0)
        {
            phase -= 1.0;
            currentStep = (currentStep + 1) % NumSteps;
        }

        // Smooth interpolation between current and next step
        int nextStep = (currentStep + 1) % NumSteps;
        float frac = static_cast<float> (phase);

        // Cosine interpolation for smoother transitions
        float t = (1.0f - std::cos (frac * juce::MathConstants<float>::pi)) * 0.5f;

        float value = steps[static_cast<size_t> (currentStep)] * (1.0f - t)
                    + steps[static_cast<size_t> (nextStep)] * t;

        // Center around 0: 0.5 → 0, 0.0 → -1, 1.0 → +1
        return value * 2.0f - 1.0f;
    }

    int getCurrentStep() const { return currentStep; }
    float getPhase() const { return static_cast<float> (phase); }

    const std::array<float, NumSteps>& getSteps() const { return steps; }

private:
    double sr = 44100.0;
    double phase = 0.0;
    int currentStep = 0;
    float stepRateHz = 1.0f;
    std::array<float, NumSteps> steps { 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f, 0.5f };
};
