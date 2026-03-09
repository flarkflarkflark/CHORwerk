#include "ChorusEngine.h"

ChorusEngine::ChorusEngine()
{
    rng.seed (std::random_device{}());
}

void ChorusEngine::prepare (double sr, int maxBlockSize)
{
    sampleRate = sr;
    blockSize = maxBlockSize;

    wetBuffer.setSize (2, maxBlockSize + 8, false, true, true);
    wetBuffer.clear();

    for (auto& voice : voices)
        voice.prepare (sr, MaxDelayMs);

    lfo.prepare (sr);
    sequencer.prepare (sr);
    envFollowerL.prepare (sr);
    envFollowerR.prepare (sr);

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
    lastWetL = lastWetR = 0.0f;

    for (int v = 0; v < MaxVoices; ++v)
    {
        if (MaxVoices > 1)
            voices[static_cast<size_t>(v)].panPosition =
                -1.0f + 2.0f * static_cast<float>(v) / static_cast<float>(MaxVoices - 1);
        else
            voices[0].panPosition = 0.0f;

        randomizeVoice (v);
        // Start voices at random positions within range
        voices[static_cast<size_t>(v)].currentDelay = minDelay + dist(rng) * delayRange;
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
    smoothPregain.setTargetValue (juce::Decibels::decibelsToGain (apvts.getRawParameterValue (Param::Pregain)->load()));
    smoothPostgain.setTargetValue (juce::Decibels::decibelsToGain (apvts.getRawParameterValue (Param::Postgain)->load()));

    smoothLPCutoff.setTargetValue (*apvts.getRawParameterValue (Param::Lowpass) * 0.01f);
    smoothHPCutoff.setTargetValue (*apvts.getRawParameterValue (Param::Highpass) * 0.01f);
    smoothLPRes.setTargetValue (*apvts.getRawParameterValue (Param::LPRes) * 0.01f);
    smoothHPRes.setTargetValue (*apvts.getRawParameterValue (Param::HPRes) * 0.01f);

    followMode = static_cast<FollowMode> (static_cast<int> (*apvts.getRawParameterValue (Param::FollowLevel)));
    envFollowerL.setMode (static_cast<EnvelopeFollower::Mode> (static_cast<int> (followMode)));
    envFollowerR.setMode (static_cast<EnvelopeFollower::Mode> (static_cast<int> (followMode)));
    envToLP = *apvts.getRawParameterValue (Param::EnvLP);
    envToHP = *apvts.getRawParameterValue (Param::EnvHP);
    envToDelay = *apvts.getRawParameterValue (Param::EnvDelay);

    auto lfoShapeIdx = static_cast<int> (*apvts.getRawParameterValue (Param::LFOShape));
    lfo.setShape (static_cast<LFO::Shape> (lfoShapeIdx));
    
    std::array<float, 16> userLfoPoints;
    for (int i = 0; i < 16; ++i)
        userLfoPoints[static_cast<size_t>(i)] = *apvts.getRawParameterValue ("lfoPoint" + std::to_string(i));
    lfo.setUserWaveform (userLfoPoints);

    lfoToLP = *apvts.getRawParameterValue (Param::LFOLP);
    lfoToHP = *apvts.getRawParameterValue (Param::LFOHP);
    lfoToDelay = *apvts.getRawParameterValue (Param::LFODelay);

    for (int i = 0; i < StepSequencer::NumSteps; ++i)
        sequencer.setStepValue (i, *apvts.getRawParameterValue ("seq" + std::to_string (i)) * 0.01f);
    
    seqToLP = *apvts.getRawParameterValue (Param::SeqLP);
    seqToHP = *apvts.getRawParameterValue (Param::SeqHP);
    seqToDelay = *apvts.getRawParameterValue (Param::SeqDelay);

    updateOnCollision = *apvts.getRawParameterValue (Param::UpdateOnCollision) > 0.5f;

    float bpm = 120.0f;
    if (playHead != nullptr) {
        if (auto pos = playHead->getPosition()) {
            if (pos->getBpm().hasValue()) bpm = static_cast<float> (*pos->getBpm());
        }
    }

    auto lfoUnit = static_cast<TempoSync::Unit> (static_cast<int> (*apvts.getRawParameterValue (Param::LFOUnit)));
    float lfoRateVal = *apvts.getRawParameterValue (Param::LFORate);
    if (*apvts.getRawParameterValue (Param::LFOQuant) > 0.5f) lfoRateVal = std::round (lfoRateVal);
    lfo.setFrequencyHz (TempoSync::toHz (lfoUnit, std::max (lfoRateVal, 0.01f), bpm));

    auto seqUnit = static_cast<TempoSync::Unit> (static_cast<int> (*apvts.getRawParameterValue (Param::SeqUnit)));
    float seqStepVal = *apvts.getRawParameterValue (Param::SeqStep);
    if (*apvts.getRawParameterValue (Param::SeqQuant) > 0.5f) seqStepVal = std::round (seqStepVal);
    sequencer.setStepRateHz (TempoSync::toHz (seqUnit, std::max (seqStepVal, 0.01f), bpm));

    auto updateUnit = static_cast<TempoSync::Unit> (static_cast<int> (*apvts.getRawParameterValue (Param::UpdateUnit)));
    float rateUpdateVal = *apvts.getRawParameterValue (Param::RateUpdate);
    if (*apvts.getRawParameterValue (Param::UpdateQuantize) > 0.5f) rateUpdateVal = std::round (rateUpdateVal);
    rateUpdatePeriodMs = TempoSync::toPeriodMs (updateUnit, std::max (rateUpdateVal, 0.01f), bpm);

    // === Process samples ===
    auto* leftIn  = buffer.getReadPointer (0);
    auto* rightIn = (numChannels > 1) ? buffer.getReadPointer (1) : leftIn;

    wetBuffer.setSize (numChannels, numSamples, false, true, true);
    wetBuffer.clear (0, 0, numSamples);
    if (numChannels > 1) wetBuffer.clear (1, 0, numSamples);

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
        
        if (*apvts.getRawParameterValue (Param::PrePhase) > 0.5f) {
            inL = -inL; inR = -inR;
        }

        // --- Feedback Rotation Matrix ---
        // Rotate the previous wet output samples
        float rotAngle = rotation * juce::MathConstants<float>::pi;
        float cosRot = std::cos (rotAngle);
        float sinRot = std::sin (rotAngle);
        
        float fedBackL = (lastWetL * cosRot - lastWetR * sinRot) * feedback;
        float fedBackR = (lastWetL * sinRot + lastWetR * cosRot) * feedback;

        // Envelope follower tracks the input for visualization and "tie output to level"
        float envModL = envFollowerL.processSample (inL);
        float envModR = envFollowerR.processSample (inR);
        envLevel = (envFollowerL.getLevel() + envFollowerR.getLevel()) * 0.5f;
        float modEnvLevel = (envModL + envModR) * 0.5f;

        float lfoVal = lfo.processSample();
        float seqVal = sequencer.processSample();

        float lpMod = sanitize (lfoVal * lfoToLP + seqVal * seqToLP + modEnvLevel * envToLP);
        float hpMod = sanitize (lfoVal * lfoToHP + seqVal * seqToHP + modEnvLevel * envToHP);
        float delayModMod = sanitize (lfoVal * lfoToDelay + seqVal * seqToDelay + modEnvLevel * envToDelay * 2.0f);

        float modLPCutoff = std::clamp (lpCutoff + lpMod * 0.3f, 0.01f, 0.99f);
        float modHPCutoff = std::clamp (hpCutoff + hpMod * 0.3f, 0.01f, 0.99f);

        float wetAccumL = 0.0f, wetAccumR = 0.0f;

        // Process each voice
        for (int v = 0; v < numVoices; ++v)
        {
            auto& voice = voices[static_cast<size_t>(v)];

            // 1. Rate Update (re-randomize velocity)
            float updatePeriodSamples = rateUpdatePeriodMs * 0.001f * static_cast<float> (sampleRate);
            voice.updateCountdown -= 1.0f;
            if (voice.updateCountdown <= 0.0f)
            {
                randomizeVoice (v);
                voice.updateCountdown = updatePeriodSamples;
            }

            // 2. Velocity-based Delay Modulation (Wanderer)
            // Move delay by current rate
            float rateSmoothCoeff = 1.0f - std::exp (-1.0f / (std::max(1.0f, static_cast<float>(sampleRate)) * 0.05f));
            voice.currentRate += rateSmoothCoeff * (voice.targetRate - voice.currentRate);
            
            float delayStep = (voice.currentRate / static_cast<float>(sampleRate)) * voice.rateDirection;
            voice.currentDelay += delayStep;

            // Bounce at bounds
            float boundMin = minDelay;
            float boundMax = minDelay + delayRange;
            
            if (voice.currentDelay > boundMax) {
                voice.currentDelay = boundMax;
                voice.rateDirection = -1.0f;
                if (updateOnCollision) randomizeVoice(v);
            } else if (voice.currentDelay < boundMin) {
                voice.currentDelay = boundMin;
                voice.rateDirection = 1.0f;
                if (updateOnCollision) randomizeVoice(v);
            }

            // Apply global modulations to the delay
            float totalDelay = std::clamp (voice.currentDelay + delayModMod, 0.1f, MaxDelayMs);

            // Write to delay line (L and R shared mono input for voices usually, 
            // but we can feed L to even and R to odd voices for "Buzz" style width)
            float voiceInput = (v % 2 == 0) ? (inL + fedBackL) : (inR + fedBackR);
            voice.delayLine.pushSample (softClip (voiceInput));

            // Read from delay line
            float delayed = voice.delayLine.readSample (totalDelay);

            // Apply filter (in the feedback loop as requested)
            float filtered = voice.filter.process (sanitize (delayed), modLPCutoff, lpRes, modHPCutoff, hpRes);
            filtered = sanitize (filtered);

            // Gain distribution
            float gain = computeGainDistribution (v, numVoices);

            // --- Stereo Mode Logic ---
            float pan = voice.panPosition;
            if (v % 2 == 1 && v > 0) 
            {
                switch (stereoMode)
                {
                    case StereoMode::Free: break;
                    case StereoMode::Slave: pan = voices[static_cast<size_t>(v - 1)].panPosition; break;
                    case StereoMode::AntiSlave: pan = -voices[static_cast<size_t>(v - 1)].panPosition; break;
                    case StereoMode::Half: pan = voices[static_cast<size_t>(v - 1)].panPosition * 0.5f; break;
                }
            }

            // Equal-power panning
            float panAngle = (pan + 1.0f) * 0.25f * juce::MathConstants<float>::pi;
            wetAccumL += filtered * gain * std::cos (panAngle);
            wetAccumR += filtered * gain * std::sin (panAngle);
        }

        // Postgain and "tie output to level"
        float outputScale = postgain;
        if (followMode != FollowMode::Off)
            outputScale *= std::sqrt(envLevel); // tied to input level

        lastWetL = sanitize (softClip (wetAccumL * outputScale));
        lastWetR = sanitize (softClip (wetAccumR * outputScale));
        
        if (*apvts.getRawParameterValue (Param::PostPhase) > 0.5f) {
            lastWetL = -lastWetL; lastWetR = -lastWetR;
        }

        float dryL = leftIn[s];
        float dryR = (numChannels > 1) ? rightIn[s] : leftIn[s];

        wetBuffer.setSample (0, s, dryL * (1.0f - mix) + lastWetL * mix);
        if (numChannels > 1)
            wetBuffer.setSample (1, s, dryR * (1.0f - mix) + lastWetR * mix);
    }

    for (int ch = 0; ch < numChannels; ++ch)
        buffer.copyFrom (ch, 0, wetBuffer, ch, 0, numSamples);
}

void ChorusEngine::randomizeVoice (int voiceIdx)
{
    auto& voice = voices[static_cast<size_t>(voiceIdx)];
    voice.targetRate = minRate + dist(rng) * rateRange;
}

float ChorusEngine::computeGainDistribution (int voiceIdx, int totalVoices) const
{
    if (totalVoices <= 1) return 1.0f;
    float baseGain = 1.0f / std::sqrt (static_cast<float>(totalVoices));
    float pos = static_cast<float>(voiceIdx) / static_cast<float>(totalVoices - 1);
    float centerWeight = 1.0f - 0.15f * std::abs (pos * 2.0f - 1.0f);
    return baseGain * centerWeight;
}
