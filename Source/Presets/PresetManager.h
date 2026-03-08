#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <vector>
#include <functional>

/**
 * Preset Manager for Charsiesis v2.
 *
 * Handles:
 * - Factory presets (embedded, read-only)
 * - User presets (saved to disk as .cpreset XML files)
 * - Categories (Subtle, Chorus, Ensemble, Creative, Rhythmic, User)
 * - Dirty flag tracking (modified since last load/save)
 * - A/B comparison buffer
 * - Undo on preset change (stores previous state)
 *
 * Preset file format: XML with all APVTS parameter IDs as attributes.
 * User presets stored in: <AppData>/flarkAUDIO/Charsiesis/Presets/
 */
class PresetManager
{
public:
    //==========================================================================
    struct PresetInfo
    {
        juce::String name;
        juce::String author;
        juce::String category;
        juce::String description;
        juce::File   file;       // Empty for factory presets
        bool         isFactory;
        int          index;      // Position in combined list
    };

    //==========================================================================
    enum class Category
    {
        All,
        Subtle,
        Chorus,
        Ensemble,
        Creative,
        Rhythmic,
        User
    };

    static juce::StringArray getCategoryNames()
    {
        return { "All", "Subtle", "Chorus", "Ensemble", "Creative", "Rhythmic", "User" };
    }

    static juce::String categoryToString (Category c)
    {
        return getCategoryNames()[static_cast<int> (c)];
    }

    //==========================================================================
    PresetManager (juce::AudioProcessorValueTreeState& apvts);
    ~PresetManager() = default;

    /** Initialize: load factory presets, scan user presets. */
    void initialize();

    //--- Navigation -----------------------------------------------------------
    int getNumPresets() const;
    int getCurrentPresetIndex() const;
    const PresetInfo& getPresetInfo (int index) const;
    const PresetInfo& getCurrentPresetInfo() const;
    juce::String getCurrentPresetName() const;

    void loadPreset (int index);
    void loadNextPreset();
    void loadPreviousPreset();

    //--- Filtered list (by category) ------------------------------------------
    void setCategory (Category c);
    Category getCategory() const { return currentCategory; }
    int getNumFilteredPresets() const;
    const PresetInfo& getFilteredPresetInfo (int filteredIndex) const;
    void loadFilteredPreset (int filteredIndex);

    //--- Save / Delete --------------------------------------------------------
    /** Save current state as a user preset. Returns true on success. */
    bool saveUserPreset (const juce::String& name,
                         const juce::String& category = "User",
                         const juce::String& author = "",
                         const juce::String& description = "");

    /** Save over current preset (only if it's a user preset). */
    bool saveCurrentPreset();

    /** Delete a user preset. Returns true on success. */
    bool deleteUserPreset (int index);

    /** Rename a user preset. */
    bool renameUserPreset (int index, const juce::String& newName);

    //--- Dirty flag -----------------------------------------------------------
    bool isDirty() const { return dirty; }
    void setDirty (bool d = true) { dirty = d; notifyListeners(); }

    //--- A/B Comparison -------------------------------------------------------
    void storeA();
    void storeB();
    void recallA();
    void recallB();
    bool isShowingA() const { return showingA; }
    void toggleAB();

    //--- Undo -----------------------------------------------------------------
    /** Revert to state before last preset load. */
    void undoPresetChange();
    bool canUndoPresetChange() const { return undoState.isValid(); }

    //--- Import / Export ------------------------------------------------------
    bool importPreset (const juce::File& file);
    bool exportPreset (int index, const juce::File& destination);

    //--- Listeners ------------------------------------------------------------
    struct Listener
    {
        virtual ~Listener() = default;
        virtual void presetChanged() = 0;
        virtual void presetListChanged() {}
    };

    void addListener (Listener* l)    { listeners.push_back (l); }
    void removeListener (Listener* l) { listeners.erase (std::remove (listeners.begin(), listeners.end(), l), listeners.end()); }

    //--- Utilities ------------------------------------------------------------
    juce::File getUserPresetsDirectory() const;
    void rescanUserPresets();

private:
    void loadFactoryPresets();
    void scanUserPresets();
    void buildFilteredList();
    void applyPresetToAPVTS (const juce::ValueTree& preset);
    juce::ValueTree captureCurrentState() const;
    juce::ValueTree presetInfoToValueTree (const PresetInfo& info, const juce::ValueTree& params) const;
    void notifyListeners();
    void notifyListChanged();

    juce::AudioProcessorValueTreeState& apvts;

    std::vector<PresetInfo> allPresets;
    std::vector<int> filteredIndices;  // Indices into allPresets for current category
    std::vector<juce::ValueTree> presetStates;  // Parallel to allPresets

    int currentPresetIndex = 0;
    Category currentCategory = Category::All;
    bool dirty = false;

    // A/B
    juce::ValueTree stateA, stateB;
    bool showingA = true;

    // Undo
    juce::ValueTree undoState;

    // Listeners
    std::vector<Listener*> listeners;

    // Placeholder for empty info
    PresetInfo emptyInfo { "Init", "", "Subtle", "", {}, true, 0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetManager)
};
