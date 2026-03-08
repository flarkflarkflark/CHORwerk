#include "ChorusEngine.h"

ChorusEngine::ChorusEngine()
{
    rng.seed (std::random_device{}());
}

void ChorusEngine::prepare (double sr, int maxBlockSize)
{
    sampleRate = sr;
    blockSize = maxBlockSize;

    for (auto& voice : voices)
        voice.prepare (sr, MaxDelayMs);

    lfo.prepare (sr);
    sequencer.prepare (sr);
    envFollowerL.prepare (sr);
    envFollowerR.prepare (sr);

    // Smoothed values
    const float smoothTimeMs = 20.0f;
    smoothMix.reset (sr, smoothTimeMs * 0.001);
    smoothFeedback.reset (sr, smoothTimeMs * 0.001);
    smoothRotation.reset (sr, smoothTimeMs * 0.001);
    smoothPregain.reset (sr, smoothTimeMs * 0.001);
    smoothPostgain.reset (sr, smoothTimeMs * 0.001);
    smoothLPCutoff.reset (sr, smoothTimeMs * 0.001);
    smoothHPCutoff.reset (sr, smoothTimeMs * 0.001);
    smoothLPRes.reset (sr, smoothTimeMs * 0.001);
    smoothHPRes.reset (sr, smoothTimeMs * 0.001);

    reset();
}

void ChorusEngine::reset()
{
    for (auto& voice : voices)
        voice.reset();

    lfo.reset();
    sequencer.reset();
    envFollowerL.reset();
    envFollowerR.reset();
    envLevel = 0.0f;

    // Initialize voice pan positions for spread
    for (int v = 0; v < MaxVoices; ++v)
    {
        if (MaxVoices > 1)
            voices[static_cast<size_t>(v)].panPosition =
                -1.0f + 2.0f * static_cast<float>(v) / static_cast<float>(MaxVoices - 1);
        else
            voices[0].panPosition = 0.0f;

        randomizeVoice (v);
    }
}

void ChorusEngine::process (juce::AudioBuffer<float>& buffer,
                            juce::AudioPlayHead* playHead,
                            juce::AudioProcessorValueTreeState& apvts)
{
    const int numSamples = buffer.getNumSamples();
    const int numChannels = std::min (buffer.getNumChannels(), 2);
    if (numChannels < 1 || numSamples < 1) return;

    // === Read parameters ===
    numVoices = static_cast<int> (*apvts.getRawParameterValue (Param::Voices));
    minRate = *apvts.getRawParameterValue (Param::MinRate);
    rateRange = *apvts.getRawParameterValue (Param::RateRange);
    minDelay = *apvts.getRawParameterValue (Param::MinDelay);
    delayRange = *apvts.getRawParameterValue (Param::DelayRange);
    stereoMode = static_cast<StereoMode> (static_cast<int> (*apvts.getRawParameterValue (Param::StereoMode)));

    smoothMix.setTargetValue (*apvts.getRawParameterValue (Param::Mix) * 0.01f);
    smoothFeedback.setTargetValue (*apvts.getRawParameterValue (Param::Feedback) * 0.01f);
    smoothRotation.setTargetValue (*apvts.getRawParameterValue (Param::Rotation));
    smoothPregain.setTargetValue (juce::Decibels::decibelsToGain (*apvts.getRawParameterValue (Param::Pregain)));
    smoothPostgain.setTargetValue (juce::Decibels::decibelsToGain (*apvts.getRawParameterValue (Param::Postgain)));

    smoothLPCutoff.setTargetValue (*apvts.getRawParameterValue (Param::Lowpass) * 0.01f);
    smoothHPCutoff.setTargetValue (*apvts.getRawParameterValue (Param::Highpass) * 0.01f);
    smoothLPRes.setTargetValue (*apvts.getRawParameterValue (Param::LPRes) * 0.01f);
    smoothHPRes.setTargetValue (*apvts.getRawParameterValue (Param::HPRes) * 0.01f);

    // Envelope
    followMode = static_cast<FollowMode> (static_cast<int> (*apvts.getRawParameterValue (Param::FollowLevel)));
    envFollowerL.setMode (static_cast<EnvelopeFollower::Mode> (static_cast<int> (followMode)));
    envFollowerR.setMode (static_cast<EnvelopeFollower::Mode> (static_cast<int> (followMode)));
    envToLP = *apvts.getRawParameterValue (Param::EnvLP);
    envToHP = *apvts.getRawParameterValue (Param::EnvHP);
    envToDelay = *apvts.getRawParameterValue (Param::EnvDelay);

    // LFO
    auto lfoShapeIdx = static_cast<int> (*apvts.getRawParameterValue (Param::LFOShape));
    lfo.setShape (static_cast<LFO::Shape> (lfoShapeIdx));
    lfoToLP = *apvts.getRawParameterValue (Param::LFOLP);
    lfoToHP = *apvts.getRawParameterValue (Param::LFOHP);
    lfoToDelay = *apvts.getRawParameterValue (Param::LFODelay);

    // Sequencer
    for (int i = 0; i < StepSequencer::NumSteps; ++i)
    {
        auto id = "seq" + std::to_string (i);
        sequencer.setStepValue (i, *apvts.getRawParameterValue (id) * 0.01f);
    }
    seqToLP = *apvts.getRawParameterValue (Param::SeqLP);
    seqToHP = *apvts.getRawParameterValue (Param::SeqHP);
    seqToDelay = *apvts.getRawParameterValue (Param::SeqDelay);

    updateOnCollision = *apvts.getRawParameterValue (Param::UpdateOnCollision) > 0.5f;

    // Tempo
    float bpm = 120.0f;
    if (playHead != nullptr)
    {
        if (auto pos = playHead->getPosition())
        {
            if (pos->getBpm().hasValue())
                bpm = static_cast<float> (*pos->getBpm());
        }
    }

    // LFO & Seq rate from tempo sync
    auto lfoUnit = static_cast<TempoSync::Unit> (static_cast<int> (*apvts.getRawParameterValue (Param::LFOUnit)));
    float lfoRateVal = *apvts.getRawParameterValue (Param::LFORate);
    bool lfoQuant = *apvts.getRawParameterValue (Param::LFOQuant) > 0.5f;
    if (lfoQuant)
        lfoRateVal = std::round (lfoRateVal);  // Snap to integer units
    lfo.setFrequencyHz (TempoSync::toHz (lfoUnit, std::max (lfoRateVal, 0.01f), bpm));

    auto seqUnit = static_cast<TempoSync::Unit> (static_cast<int> (*apvts.getRawParameterValue (Param::SeqUnit)));
    float seqStepVal = *apvts.getRawParameterValue (Param::SeqStep);
    bool seqQuant = *apvts.getRawParameterValue (Param::SeqQuant) > 0.5f;
    if (seqQuant)
        seqStepVal = std::round (seqStepVal);
    sequencer.setStepRateHz (TempoSync::toHz (seqUnit, std::max (seqStepVal, 0.01f), bpm));

    // Rate update period
    auto updateUnit = static_cast<TempoSync::Unit> (static_cast<int> (*apvts.getRawParameterValue (Param::UpdateUnit)));
    float rateUpdateVal = *apvts.getRawParameterValue (Param::RateUpdate);
    bool updateQuant = *apvts.getRawParameterValue (Param::UpdateQuantize) > 0.5f;
    if (updateQuant)
        rateUpdateVal = std::round (rateUpdateVal);
    rateUpdatePeriodMs = TempoSync::toPeriodMs (updateUnit, std::max (rateUpdateVal, 0.01f), bpm);

    // === Process samples ===
    auto* leftIn  = buffer.getReadPointer (0);
    auto* rightIn = (numChannels > 1) ? buffer.getReadPointer (1) : leftIn;

    // Create output buffer
    juce::AudioBuffer<float> wetBuffer (numChannels, numSamples);
    wetBuffer.clear();

    for (int s = 0; s < numSamples; ++s)
    {
        float pregain = smoothPregain.getNextValue();
        float postgain = smoothPostgain.getNextValue();
        float mix = smoothMix.getNextValue();
        float feedback = smoothFeedback.getNextValue();
        float rotation = smoothRotation.getNextValue();
        float lpCutoff = smoothLPCutoff.getNextValue();
        float hpCutoff = smoothHPCutoff.getNextValue();
        float lpRes = smoothLPRes.getNextValue();
        float hpRes = smoothHPRes.getNextValue();

        float inL = leftIn[s] * pregain;
        float inR = rightIn[s] * pregain;
        float inMono = (inL + inR) * 0.5f;

        // Envelope follower
        float envL = envFollowerL.processSample (inL);
        float envR = envFollowerR.processSample (inR);
        envLevel = (envL + envR) * 0.5f;

        // LFO
        float lfoVal = lfo.processSample();

        // Sequencer
        float seqVal = sequencer.processSample();

        // Combined modulation for filter cutoffs
        float lpMod = lfoVal * lfoToLP + seqVal * seqToLP + envLevel * envToLP;
        float hpMod = lfoVal * lfoToHP + seqVal * seqToHP + envLevel * envToHP;
        float delayMod = lfoVal * lfoToDelay + seqVal * seqToDelay + envLevel * envToDelay;

        // Modulated filter cutoffs (in octaves, convert to normalized)
        float modLPCutoff = std::clamp (lpCutoff + lpMod * 0.25f, 0.0f, 1.0f);
        float modHPCutoff = std::clamp (hpCutoff + hpMod * 0.25f, 0.0f, 1.0f);

        float wetL = 0.0f, wetR = 0.0f;

        // Process each voice
        for (int v = 0; v < numVoices; ++v)
        {
            auto& voice = voices[static_cast<size_t>(v)];

            // Update voice timing
            float updatePeriodSamples = rateUpdatePeriodMs * 0.001f * static_cast<float> (sampleRate);
            voice.updateCountdown -= 1.0f;
            if (voice.updateCountdown <= 0.0f)
            {
                randomizeVoice (v);
                voice.updateCountdown = updatePeriodSamples;
            }

            // Collision detection
            if (updateOnCollision)
            {
                for (int ov = 0; ov < v; ++ov)
                {
                    float diff = std::abs (voice.currentDelay - voices[static_cast<size_t>(ov)].currentDelay);
                    if (diff < 0.1f)  // Within 0.1ms = collision
                    {
                        randomizeVoice (v);
                        break;
                    }
                }
            }

            // Smoothly move toward target delay/rate
            float rateSmoothCoeff = 1.0f - std::exp (-1.0f / (static_cast<float>(sampleRate) * 0.05f));
            voice.currentRate += rateSmoothCoeff * (voice.targetRate - voice.currentRate);

            // Delay modulation: base delay oscillates at voice rate
            voice.ratePhase += voice.currentRate / static_cast<float>(sampleRate);
            if (voice.ratePhase > 1.0f) voice.ratePhase -= 1.0f;

            float voiceDelayMod = std::sin (voice.ratePhase * juce::MathConstants<float>::twoPi);
            float totalDelay = voice.targetDelay + voiceDelayMod * (delayRange * 0.5f) + delayMod;
            totalDelay = std::clamp (totalDelay, 0.1f, MaxDelayMs);

            // Smooth delay to avoid clicks
            float delaySmoothCoeff = 1.0f - std::exp (-1.0f / (static_cast<float>(sampleRate) * 0.002f));
            voice.currentDelay += delaySmoothCoeff * (totalDelay - voice.currentDelay);

            // Write to delay line
            float inputForVoice = inMono + voice.feedbackSample * feedback;
            voice.delayLine.pushSample (inputForVoice);

            // Read from delay line
            float delayed = voice.delayLine.readSample (voice.currentDelay);

            // Apply filter
            float filtered = voice.filter.process (delayed, modLPCutoff, lpRes, modHPCutoff, hpRes);

            // Gain distribution
            float gain = computeGainDistribution (v, numVoices);

            // Store feedback (with rotation for cross-channel)
            float rotAngle = rotation * juce::MathConstants<float>::pi;
            voice.feedbackSample = filtered * std::cos (rotAngle);

            // Pan to stereo
            float pan = voice.panPosition;

            // Apply stereo mode modifications
            switch (stereoMode)
            {
                case StereoMode::Free:
                    break;  // Independent panning
                case StereoMode::Slave:
                    pan = voices[0].panPosition;  // All follow voice 0
                    break;
                case StereoMode::AntiSlave:
                    pan = -voices[0].panPosition;  // Mirror voice 0
                    break;
                case StereoMode::Half:
                    pan = voice.panPosition * 0.5f;  // Reduced spread
                    break;
            }

            // Equal-power panning
            float panAngle = (pan + 1.0f) * 0.25f * juce::MathConstants<float>::pi;
            float panL = std::cos (panAngle);
            float panR = std::sin (panAngle);

            wetL += filtered * gain * panL;
            wetR += filtered * gain * panR;
        }

        // Postgain
        wetL *= postgain;
        wetR *= postgain;

        // Mix dry/wet
        float dryL = leftIn[s];
        float dryR = (numChannels > 1) ? rightIn[s] : leftIn[s];

        wetBuffer.setSample (0, s, dryL * (1.0f - mix) + wetL * mix);
        if (numChannels > 1)
            wetBuffer.setSample (1, s, dryR * (1.0f - mix) + wetR * mix);
    }

    // Copy wet buffer to output
    for (int ch = 0; ch < numChannels; ++ch)
        buffer.copyFrom (ch, 0, wetBuffer, ch, 0, numSamples);
}

void ChorusEngine::randomizeVoice (int voiceIdx)
{
    auto& voice = voices[static_cast<size_t>(voiceIdx)];

    // Randomize rate within range
    voice.targetRate = minRate + dist(rng) * rateRange;

    // Randomize delay within range
    voice.targetDelay = minDelay + dist(rng) * delayRange;

    // Randomize pan spread
    if (numVoices > 1)
    {
        float basePos = -1.0f + 2.0f * static_cast<float>(voiceIdx) / static_cast<float>(numVoices - 1);
        voice.panPosition = basePos + (dist(rng) - 0.5f) * 0.3f;  // Small random offset
        voice.panPosition = std::clamp (voice.panPosition, -1.0f, 1.0f);
    }
}

float ChorusEngine::computeGainDistribution (int voiceIdx, int totalVoices) const
{
    // Equal gain distribution with slight tapering at edges
    // Original had a configurable gain distribution curve
    if (totalVoices <= 1) return 1.0f;

    float baseGain = 1.0f / std::sqrt (static_cast<float>(totalVoices));

    // Slight center-weighting
    float pos = static_cast<float>(voiceIdx) / static_cast<float>(totalVoices - 1);
    float centerWeight = 1.0f - 0.15f * std::abs (pos * 2.0f - 1.0f);

    return baseGain * centerWeight;
}
