#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

namespace Param
{
    // Voice Engine
    inline constexpr const char* Voices            = "voices";
    inline constexpr const char* MinRate            = "minRate";
    inline constexpr const char* RateRange          = "rateRange";
    inline constexpr const char* RateUpdate         = "rateUpdate";
    inline constexpr const char* MinDelay           = "minDelay";
    inline constexpr const char* DelayRange         = "delayRange";
    inline constexpr const char* StereoMode         = "stereoMode";

    // Mix
    inline constexpr const char* Mix                = "mix";
    inline constexpr const char* Feedback           = "feedback";
    inline constexpr const char* Rotation           = "rotation";
    inline constexpr const char* Pregain            = "pregain";
    inline constexpr const char* Postgain           = "postgain";

    // Filters
    inline constexpr const char* Lowpass            = "lowpass";
    inline constexpr const char* LPRes              = "lpRes";
    inline constexpr const char* Highpass           = "highpass";
    inline constexpr const char* HPRes              = "hpRes";

    // Envelope
    inline constexpr const char* FollowLevel        = "followLevel";
    inline constexpr const char* EnvLP              = "envLP";
    inline constexpr const char* EnvHP              = "envHP";
    inline constexpr const char* EnvDelay           = "envDelay";

    // LFO
    inline constexpr const char* LFOShape           = "lfoShape";
    inline constexpr const char* LFORate            = "lfoRate";
    inline constexpr const char* LFOUnit            = "lfoUnit";
    inline constexpr const char* LFOQuant           = "lfoQuant";
    inline constexpr const char* LFOLP              = "lfoLP";
    inline constexpr const char* LFOHP              = "lfoHP";
    inline constexpr const char* LFODelay           = "lfoDelay";

    // Step Sequencer
    inline constexpr const char* SeqStep            = "seqStep";
    inline constexpr const char* SeqUnit            = "seqUnit";
    inline constexpr const char* SeqQuant           = "seqQuant";
    inline constexpr const char* SeqLP              = "seqLP";
    inline constexpr const char* SeqHP              = "seqHP";
    inline constexpr const char* SeqDelay           = "seqDelay";

    // Seq values (8 steps)
    inline constexpr const char* Seq0               = "seq0";
    inline constexpr const char* Seq1               = "seq1";
    inline constexpr const char* Seq2               = "seq2";
    inline constexpr const char* Seq3               = "seq3";
    inline constexpr const char* Seq4               = "seq4";
    inline constexpr const char* Seq5               = "seq5";
    inline constexpr const char* Seq6               = "seq6";
    inline constexpr const char* Seq7               = "seq7";

    // Update control
    inline constexpr const char* UpdateUnit         = "updateUnit";
    inline constexpr const char* UpdateQuantize     = "updateQuantize";
    inline constexpr const char* UpdateOnCollision  = "updateOnCollision";
}

// Enums
enum class StereoMode  { Free, Slave, AntiSlave, Half };
enum class FollowMode  { Off, SlowDecay, NormalRate, FastDecay };
enum class LFOShape    { Sine, Ramp, Triangle, Square, SteppingRandom, SmoothRandom, User };

// Time unit for tempo sync
enum class TimeUnit
{
    Milliseconds, TenMs, Seconds,
    ThirtySeconds, Sixteenths, Eighths, Quarters,
    TripletThirtySeconds, TripletSixteenths, TripletEighths, TripletQuarters,
    QuintupletThirtySeconds, QuintupletSixteenths, QuintupletEighths, QuintupletQuarters
};

//==============================================================================
// Parameter layout factory
//==============================================================================
inline juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    using namespace juce;
    std::vector<std::unique_ptr<RangedAudioParameter>> params;

    // --- Voice Engine ---
    params.push_back (std::make_unique<AudioParameterInt>(
        ParameterID { Param::Voices, 1 }, "Voices", 1, 8, 4));

    params.push_back (std::make_unique<AudioParameterFloat>(
        ParameterID { Param::MinRate, 1 }, "Min Rate",
        NormalisableRange<float> (0.01f, 50.0f, 0.01f, 0.4f), 1.0f, "ms/s"));

    params.push_back (std::make_unique<AudioParameterFloat>(
        ParameterID { Param::RateRange, 1 }, "Rate Range",
        NormalisableRange<float> (0.0f, 50.0f, 0.01f, 0.4f), 2.0f, "ms/s"));

    params.push_back (std::make_unique<AudioParameterFloat>(
        ParameterID { Param::RateUpdate, 1 }, "Rate Update",
        NormalisableRange<float> (0.1f, 100.0f, 0.1f, 0.4f), 10.0f, "units"));

    params.push_back (std::make_unique<AudioParameterFloat>(
        ParameterID { Param::MinDelay, 1 }, "Min Delay",
        NormalisableRange<float> (0.1f, 20.0f, 0.01f, 0.5f), 2.0f, "ms"));

    params.push_back (std::make_unique<AudioParameterFloat>(
        ParameterID { Param::DelayRange, 1 }, "Delay Range",
        NormalisableRange<float> (0.0f, 20.0f, 0.01f, 0.5f), 5.0f, "ms"));

    params.push_back (std::make_unique<AudioParameterChoice>(
        ParameterID { Param::StereoMode, 1 }, "Stereo Mode",
        StringArray { "Free", "Slave", "Anti-Slave", "Half" }, 0));

    // --- Mix ---
    params.push_back (std::make_unique<AudioParameterFloat>(
        ParameterID { Param::Mix, 1 }, "Mix",
        NormalisableRange<float> (0.0f, 100.0f, 0.1f), 50.0f, "%"));

    params.push_back (std::make_unique<AudioParameterFloat>(
        ParameterID { Param::Feedback, 1 }, "Feedback",
        NormalisableRange<float> (0.0f, 99.0f, 0.1f), 0.0f, "%"));

    params.push_back (std::make_unique<AudioParameterFloat>(
        ParameterID { Param::Rotation, 1 }, "Rotation",
        NormalisableRange<float> (0.0f, 1.0f, 0.001f), 0.0f, "π"));

    params.push_back (std::make_unique<AudioParameterFloat>(
        ParameterID { Param::Pregain, 1 }, "Pre Gain",
        NormalisableRange<float> (-24.0f, 24.0f, 0.1f), 0.0f, "dB"));

    params.push_back (std::make_unique<AudioParameterFloat>(
        ParameterID { Param::Postgain, 1 }, "Post Gain",
        NormalisableRange<float> (-24.0f, 24.0f, 0.1f), 0.0f, "dB"));

    // --- Filters ---
    params.push_back (std::make_unique<AudioParameterFloat>(
        ParameterID { Param::Lowpass, 1 }, "Lowpass",
        NormalisableRange<float> (0.0f, 100.0f, 0.1f), 100.0f, "%"));

    params.push_back (std::make_unique<AudioParameterFloat>(
        ParameterID { Param::LPRes, 1 }, "LP Resonance",
        NormalisableRange<float> (0.0f, 100.0f, 0.1f), 0.0f, "%"));

    params.push_back (std::make_unique<AudioParameterFloat>(
        ParameterID { Param::Highpass, 1 }, "Highpass",
        NormalisableRange<float> (0.0f, 100.0f, 0.1f), 0.0f, "%"));

    params.push_back (std::make_unique<AudioParameterFloat>(
        ParameterID { Param::HPRes, 1 }, "HP Resonance",
        NormalisableRange<float> (0.0f, 100.0f, 0.1f), 0.0f, "%"));

    // --- Envelope ---
    params.push_back (std::make_unique<AudioParameterChoice>(
        ParameterID { Param::FollowLevel, 1 }, "Follow Level",
        StringArray { "Off", "Slow Decay", "Normal", "Fast Decay" }, 0));

    params.push_back (std::make_unique<AudioParameterFloat>(
        ParameterID { Param::EnvLP, 1 }, "Env → LP",
        NormalisableRange<float> (-4.0f, 4.0f, 0.01f), 0.0f, "oct"));

    params.push_back (std::make_unique<AudioParameterFloat>(
        ParameterID { Param::EnvHP, 1 }, "Env → HP",
        NormalisableRange<float> (-4.0f, 4.0f, 0.01f), 0.0f, "oct"));

    params.push_back (std::make_unique<AudioParameterFloat>(
        ParameterID { Param::EnvDelay, 1 }, "Env → Delay",
        NormalisableRange<float> (-20.0f, 20.0f, 0.01f), 0.0f, "ms"));

    // --- LFO ---
    params.push_back (std::make_unique<AudioParameterChoice>(
        ParameterID { Param::LFOShape, 1 }, "LFO Shape",
        StringArray { "Sine", "Ramp", "Triangle", "Square",
                      "Stepping Random", "Smooth Random", "User" }, 0));

    params.push_back (std::make_unique<AudioParameterFloat>(
        ParameterID { Param::LFORate, 1 }, "LFO Rate",
        NormalisableRange<float> (0.01f, 100.0f, 0.01f, 0.3f), 1.0f, "units"));

    params.push_back (std::make_unique<AudioParameterChoice>(
        ParameterID { Param::LFOUnit, 1 }, "LFO Unit",
        StringArray { "ms", "10 ms", "sec",
                      "32nds", "16ths", "8ths", "Quarters",
                      "2/3 32nds", "2/3 16ths", "2/3 8ths", "2/3 Quarters",
                      "4/5 32nds", "4/5 16ths", "4/5 8ths", "4/5 Quarters" }, 0));

    params.push_back (std::make_unique<AudioParameterBool>(
        ParameterID { Param::LFOQuant, 1 }, "LFO Quantize", false));

    params.push_back (std::make_unique<AudioParameterFloat>(
        ParameterID { Param::LFOLP, 1 }, "LFO → LP",
        NormalisableRange<float> (-4.0f, 4.0f, 0.01f), 0.0f, "oct"));

    params.push_back (std::make_unique<AudioParameterFloat>(
        ParameterID { Param::LFOHP, 1 }, "LFO → HP",
        NormalisableRange<float> (-4.0f, 4.0f, 0.01f), 0.0f, "oct"));

    params.push_back (std::make_unique<AudioParameterFloat>(
        ParameterID { Param::LFODelay, 1 }, "LFO → Delay",
        NormalisableRange<float> (-20.0f, 20.0f, 0.01f), 0.0f, "ms"));

    // --- Step Sequencer ---
    for (int i = 0; i < 8; ++i)
    {
        auto id = "seq" + std::to_string (i);
        auto name = "Seq #" + std::to_string (i);
        params.push_back (std::make_unique<AudioParameterFloat>(
            ParameterID { id, 1 }, name,
            NormalisableRange<float> (0.0f, 100.0f, 0.1f), 50.0f, "%"));
    }

    params.push_back (std::make_unique<AudioParameterFloat>(
        ParameterID { Param::SeqStep, 1 }, "Seq Step",
        NormalisableRange<float> (0.01f, 100.0f, 0.01f, 0.3f), 1.0f, "units"));

    params.push_back (std::make_unique<AudioParameterChoice>(
        ParameterID { Param::SeqUnit, 1 }, "Seq Unit",
        StringArray { "ms", "10 ms", "sec",
                      "32nds", "16ths", "8ths", "Quarters",
                      "2/3 32nds", "2/3 16ths", "2/3 8ths", "2/3 Quarters",
                      "4/5 32nds", "4/5 16ths", "4/5 8ths", "4/5 Quarters" }, 0));

    params.push_back (std::make_unique<AudioParameterBool>(
        ParameterID { Param::SeqQuant, 1 }, "Seq Quantize", false));

    params.push_back (std::make_unique<AudioParameterFloat>(
        ParameterID { Param::SeqLP, 1 }, "Seq → LP",
        NormalisableRange<float> (-4.0f, 4.0f, 0.01f), 0.0f, "oct"));

    params.push_back (std::make_unique<AudioParameterFloat>(
        ParameterID { Param::SeqHP, 1 }, "Seq → HP",
        NormalisableRange<float> (-4.0f, 4.0f, 0.01f), 0.0f, "oct"));

    params.push_back (std::make_unique<AudioParameterFloat>(
        ParameterID { Param::SeqDelay, 1 }, "Seq → Delay",
        NormalisableRange<float> (-20.0f, 20.0f, 0.01f), 0.0f, "ms"));

    // --- Update Control ---
    params.push_back (std::make_unique<AudioParameterChoice>(
        ParameterID { Param::UpdateUnit, 1 }, "Update Unit",
        StringArray { "ms", "10 ms", "sec",
                      "32nds", "16ths", "8ths", "Quarters",
                      "2/3 32nds", "2/3 16ths", "2/3 8ths", "2/3 Quarters",
                      "4/5 32nds", "4/5 16ths", "4/5 8ths", "4/5 Quarters" }, 0));

    params.push_back (std::make_unique<AudioParameterBool>(
        ParameterID { Param::UpdateQuantize, 1 }, "Update Quantize", false));

    params.push_back (std::make_unique<AudioParameterBool>(
        ParameterID { Param::UpdateOnCollision, 1 }, "On Collision", false));

    return { params.begin(), params.end() };
}
