cmake_minimum_required(VERSION 3.22)
project(CHORwerk VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- JUCE ---
# Set JUCE path here or use CPM/FetchContent
# option(JUCE_PATH "Path to JUCE" "")
include(FetchContent)
FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        7.0.12
)
FetchContent_MakeAvailable(JUCE)

# --- CLAP support via clap-juce-extensions ---
FetchContent_Declare(
    clap-juce-extensions
    GIT_REPOSITORY https://github.com/free-audio/clap-juce-extensions.git
    GIT_TAG        main
)
FetchContent_MakeAvailable(clap-juce-extensions)

# --- Plugin Target ---
juce_add_plugin(CHORwerk
    COMPANY_NAME           "flarkAUDIO"
    PLUGIN_MANUFACTURER_CODE Flrk
    PLUGIN_CODE            Chr1
    FORMATS                VST3 AU Standalone  # CLAP added via extension
    PRODUCT_NAME           "CHORwerk v1-0-0"
    BUNDLE_ID              "com.flarkaudio.chorwerk"
    IS_SYNTH               FALSE
    NEEDS_MIDI_INPUT       FALSE
    NEEDS_MIDI_OUTPUT      FALSE
    IS_MIDI_EFFECT         FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    VST3_CATEGORIES        "Fx|Modulation|Chorus"
    AU_MAIN_TYPE           "kAudioUnitType_Effect"
    COPY_PLUGIN_AFTER_BUILD TRUE
)

# Enable CLAP
clap_juce_extensions_plugin(
    TARGET CHORwerk
    CLAP_ID "com.flarkaudio.chorwerk"
    CLAP_FEATURES "audio-effect" "chorus" "multi-voice" "stereo"
)

# --- Sources ---
target_sources(CHORwerk PRIVATE
    # Core
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
    Source/Parameters.h

    # DSP
    Source/DSP/ChorusEngine.cpp
    Source/DSP/ChorusEngine.h
    Source/DSP/DelayLine.h
    Source/DSP/StateVariableFilter.h
    Source/DSP/EnvelopeFollower.h
    Source/DSP/LFO.h
    Source/DSP/StepSequencer.h
    Source/DSP/TempoSync.h
    Source/DSP/VoiceState.h

    # Presets
    Source/Presets/PresetManager.cpp
    Source/Presets/PresetManager.h

    # UI
    Source/UI/LookAndFeel.cpp
    Source/UI/LookAndFeel.h
    Source/UI/KnobComponent.h
    Source/UI/WaveformDisplay.h
    Source/UI/StepSequencerComponent.h
    Source/UI/SectionComponent.h
    Source/UI/PresetBrowser.h
)

target_compile_definitions(CHORwerk PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0
)

target_link_libraries(CHORwerk
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
