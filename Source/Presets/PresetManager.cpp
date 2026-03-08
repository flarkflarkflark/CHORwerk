#include "PresetManager.h"

//==============================================================================
// Factory presets embedded as raw XML string
// In production, use JUCE BinaryData, but this works cross-platform.
//==============================================================================

static const char* getFactoryPresetsXML();  // defined at bottom of file

//==============================================================================
PresetManager::PresetManager (juce::AudioProcessorValueTreeState& vts)
    : apvts (vts)
{
}

void PresetManager::initialize()
{
    loadFactoryPresets();
    scanUserPresets();
    buildFilteredList();

    if (! allPresets.empty())
        loadPreset (0);
}

//==============================================================================
// Factory presets
//==============================================================================
void PresetManager::loadFactoryPresets()
{
    auto xml = juce::parseXML (juce::String (getFactoryPresetsXML()));
    if (xml == nullptr) return;

    int index = 0;
    for (auto* presetElem : xml->getChildWithTagNameIterator ("Preset"))
    {
        PresetInfo info;
        info.name        = presetElem->getStringAttribute ("name", "Unnamed");
        info.author      = presetElem->getStringAttribute ("author", "flarkAUDIO");
        info.category    = presetElem->getStringAttribute ("category", "Subtle");
        info.description = presetElem->getStringAttribute ("description", "");
        info.isFactory   = true;
        info.index       = index;

        // Parse parameter values into a ValueTree
        auto paramsElem = presetElem->getChildByName ("Params");
        juce::ValueTree state ("PresetState");

        if (paramsElem != nullptr)
        {
            for (int i = 0; i < paramsElem->getNumAttributes(); ++i)
            {
                auto name = paramsElem->getAttributeName (i);
                auto value = paramsElem->getDoubleAttribute (name, 0.0);
                state.setProperty (juce::Identifier (name), value, nullptr);
            }
        }

        allPresets.push_back (info);
        presetStates.push_back (state);
        ++index;
    }
}

//==============================================================================
// User presets
//==============================================================================
juce::File PresetManager::getUserPresetsDirectory() const
{
    auto dir = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
                   .getChildFile ("flarkAUDIO")
                   .getChildFile ("Charsiesis")
                   .getChildFile ("Presets");

    if (! dir.exists())
        dir.createDirectory();

    return dir;
}

void PresetManager::scanUserPresets()
{
    // Remove existing user presets from list
    for (int i = static_cast<int> (allPresets.size()) - 1; i >= 0; --i)
    {
        if (! allPresets[static_cast<size_t> (i)].isFactory)
        {
            allPresets.erase (allPresets.begin() + i);
            presetStates.erase (presetStates.begin() + i);
        }
    }

    auto dir = getUserPresetsDirectory();
    auto files = dir.findChildFiles (juce::File::findFiles, false, "*.cpreset");
    files.sort();

    int index = static_cast<int> (allPresets.size());

    for (auto& file : files)
    {
        auto xml = juce::parseXML (file);
        if (xml == nullptr) continue;

        PresetInfo info;
        info.name        = xml->getStringAttribute ("name", file.getFileNameWithoutExtension());
        info.author      = xml->getStringAttribute ("author", "User");
        info.category    = xml->getStringAttribute ("category", "User");
        info.description = xml->getStringAttribute ("description", "");
        info.file        = file;
        info.isFactory   = false;
        info.index       = index;

        auto paramsElem = xml->getChildByName ("Params");
        juce::ValueTree state ("PresetState");

        if (paramsElem != nullptr)
        {
            for (int i = 0; i < paramsElem->getNumAttributes(); ++i)
            {
                auto name = paramsElem->getAttributeName (i);
                auto value = paramsElem->getDoubleAttribute (name, 0.0);
                state.setProperty (juce::Identifier (name), value, nullptr);
            }
        }

        allPresets.push_back (info);
        presetStates.push_back (state);
        ++index;
    }

    // Re-index all presets
    for (size_t i = 0; i < allPresets.size(); ++i)
        allPresets[i].index = static_cast<int> (i);
}

void PresetManager::rescanUserPresets()
{
    int prevIdx = currentPresetIndex;
    auto prevName = getCurrentPresetName();

    scanUserPresets();
    buildFilteredList();

    // Try to keep the same preset selected
    bool found = false;
    for (size_t i = 0; i < allPresets.size(); ++i)
    {
        if (allPresets[i].name == prevName)
        {
            currentPresetIndex = static_cast<int> (i);
            found = true;
            break;
        }
    }
    if (! found)
        currentPresetIndex = std::min (prevIdx, static_cast<int> (allPresets.size()) - 1);

    notifyListChanged();
}

//==============================================================================
// Navigation
//==============================================================================
int PresetManager::getNumPresets() const
{
    return static_cast<int> (allPresets.size());
}

int PresetManager::getCurrentPresetIndex() const
{
    return currentPresetIndex;
}

const PresetManager::PresetInfo& PresetManager::getPresetInfo (int index) const
{
    if (index >= 0 && index < static_cast<int> (allPresets.size()))
        return allPresets[static_cast<size_t> (index)];
    return emptyInfo;
}

const PresetManager::PresetInfo& PresetManager::getCurrentPresetInfo() const
{
    return getPresetInfo (currentPresetIndex);
}

juce::String PresetManager::getCurrentPresetName() const
{
    auto& info = getCurrentPresetInfo();
    if (dirty)
        return info.name + " *";
    return info.name;
}

void PresetManager::loadPreset (int index)
{
    if (index < 0 || index >= static_cast<int> (allPresets.size()))
        return;

    // Store undo state
    undoState = captureCurrentState();

    // Apply
    applyPresetToAPVTS (presetStates[static_cast<size_t> (index)]);
    currentPresetIndex = index;
    dirty = false;

    // Also store as A state
    stateA = presetStates[static_cast<size_t> (index)].createCopy();
    showingA = true;

    notifyListeners();
}

void PresetManager::loadNextPreset()
{
    if (currentCategory == Category::All)
    {
        int next = (currentPresetIndex + 1) % getNumPresets();
        loadPreset (next);
    }
    else
    {
        // Navigate within filtered list
        int currentFiltered = -1;
        for (int i = 0; i < static_cast<int> (filteredIndices.size()); ++i)
        {
            if (filteredIndices[static_cast<size_t> (i)] == currentPresetIndex)
            {
                currentFiltered = i;
                break;
            }
        }
        int nextFiltered = (currentFiltered + 1) % getNumFilteredPresets();
        loadFilteredPreset (nextFiltered);
    }
}

void PresetManager::loadPreviousPreset()
{
    if (currentCategory == Category::All)
    {
        int prev = (currentPresetIndex - 1 + getNumPresets()) % getNumPresets();
        loadPreset (prev);
    }
    else
    {
        int currentFiltered = -1;
        for (int i = 0; i < static_cast<int> (filteredIndices.size()); ++i)
        {
            if (filteredIndices[static_cast<size_t> (i)] == currentPresetIndex)
            {
                currentFiltered = i;
                break;
            }
        }
        int prevFiltered = (currentFiltered - 1 + getNumFilteredPresets()) % getNumFilteredPresets();
        loadFilteredPreset (prevFiltered);
    }
}

//==============================================================================
// Filtered navigation
//==============================================================================
void PresetManager::setCategory (Category c)
{
    currentCategory = c;
    buildFilteredList();
    notifyListeners();
}

void PresetManager::buildFilteredList()
{
    filteredIndices.clear();

    if (currentCategory == Category::All)
    {
        for (int i = 0; i < static_cast<int> (allPresets.size()); ++i)
            filteredIndices.push_back (i);
    }
    else
    {
        auto catName = categoryToString (currentCategory);
        for (int i = 0; i < static_cast<int> (allPresets.size()); ++i)
        {
            if (allPresets[static_cast<size_t> (i)].category.equalsIgnoreCase (catName))
                filteredIndices.push_back (i);
        }
    }
}

int PresetManager::getNumFilteredPresets() const
{
    return static_cast<int> (filteredIndices.size());
}

const PresetManager::PresetInfo& PresetManager::getFilteredPresetInfo (int filteredIndex) const
{
    if (filteredIndex >= 0 && filteredIndex < static_cast<int> (filteredIndices.size()))
        return allPresets[static_cast<size_t> (filteredIndices[static_cast<size_t> (filteredIndex)])];
    return emptyInfo;
}

void PresetManager::loadFilteredPreset (int filteredIndex)
{
    if (filteredIndex >= 0 && filteredIndex < static_cast<int> (filteredIndices.size()))
        loadPreset (filteredIndices[static_cast<size_t> (filteredIndex)]);
}

//==============================================================================
// Save / Delete
//==============================================================================
bool PresetManager::saveUserPreset (const juce::String& name,
                                     const juce::String& category,
                                     const juce::String& author,
                                     const juce::String& description)
{
    if (name.isEmpty()) return false;

    // Create XML
    auto xml = std::make_unique<juce::XmlElement> ("CharsiesiPreset");
    xml->setAttribute ("name", name);
    xml->setAttribute ("author", author.isEmpty() ? "User" : author);
    xml->setAttribute ("category", category.isEmpty() ? "User" : category);
    xml->setAttribute ("description", description);
    xml->setAttribute ("version", "2.0");

    // Capture current parameter state
    auto state = captureCurrentState();
    auto paramsElem = xml->createNewChildElement ("Params");
    for (int i = 0; i < state.getNumProperties(); ++i)
    {
        auto propName = state.getPropertyName (i);
        paramsElem->setAttribute (propName.toString(), static_cast<double> (state[propName]));
    }

    // Save file
    auto sanitizedName = name.replaceCharacters ("/\\:*?\"<>|", "_________");
    auto file = getUserPresetsDirectory().getChildFile (sanitizedName + ".cpreset");

    if (! xml->writeTo (file))
        return false;

    // Rescan and select the new preset
    rescanUserPresets();

    // Find and select the saved preset
    for (size_t i = 0; i < allPresets.size(); ++i)
    {
        if (allPresets[i].name == name && ! allPresets[i].isFactory)
        {
            currentPresetIndex = static_cast<int> (i);
            break;
        }
    }

    dirty = false;
    notifyListeners();
    return true;
}

bool PresetManager::saveCurrentPreset()
{
    auto& current = getCurrentPresetInfo();
    if (current.isFactory) return false;  // Can't overwrite factory presets

    return saveUserPreset (current.name, current.category, current.author, current.description);
}

bool PresetManager::deleteUserPreset (int index)
{
    if (index < 0 || index >= static_cast<int> (allPresets.size()))
        return false;

    auto& info = allPresets[static_cast<size_t> (index)];
    if (info.isFactory) return false;
    if (! info.file.existsAsFile()) return false;

    info.file.deleteFile();
    rescanUserPresets();

    if (currentPresetIndex >= static_cast<int> (allPresets.size()))
        currentPresetIndex = std::max (0, static_cast<int> (allPresets.size()) - 1);

    notifyListeners();
    return true;
}

bool PresetManager::renameUserPreset (int index, const juce::String& newName)
{
    if (index < 0 || index >= static_cast<int> (allPresets.size()))
        return false;

    auto& info = allPresets[static_cast<size_t> (index)];
    if (info.isFactory) return false;
    if (! info.file.existsAsFile()) return false;

    // Read, modify name, rewrite
    auto xml = juce::parseXML (info.file);
    if (xml == nullptr) return false;

    xml->setAttribute ("name", newName);

    auto sanitizedName = newName.replaceCharacters ("/\\:*?\"<>|", "_________");
    auto newFile = getUserPresetsDirectory().getChildFile (sanitizedName + ".cpreset");

    if (! xml->writeTo (newFile))
        return false;

    // Delete old file if name changed
    if (info.file != newFile)
        info.file.deleteFile();

    rescanUserPresets();
    notifyListeners();
    return true;
}

//==============================================================================
// A/B Comparison
//==============================================================================
void PresetManager::storeA()
{
    stateA = captureCurrentState();
}

void PresetManager::storeB()
{
    stateB = captureCurrentState();
}

void PresetManager::recallA()
{
    if (stateA.isValid())
    {
        applyPresetToAPVTS (stateA);
        showingA = true;
        notifyListeners();
    }
}

void PresetManager::recallB()
{
    if (stateB.isValid())
    {
        applyPresetToAPVTS (stateB);
        showingA = false;
        notifyListeners();
    }
}

void PresetManager::toggleAB()
{
    if (showingA)
    {
        stateA = captureCurrentState();
        recallB();
    }
    else
    {
        stateB = captureCurrentState();
        recallA();
    }
}

//==============================================================================
// Undo
//==============================================================================
void PresetManager::undoPresetChange()
{
    if (undoState.isValid())
    {
        auto currentState = captureCurrentState();
        applyPresetToAPVTS (undoState);
        undoState = currentState;  // Now undo becomes redo
        notifyListeners();
    }
}

//==============================================================================
// Import / Export
//==============================================================================
bool PresetManager::importPreset (const juce::File& file)
{
    auto xml = juce::parseXML (file);
    if (xml == nullptr) return false;

    // Validate it's a Charsiesis preset
    if (xml->getTagName() != "CharsiesiPreset" && xml->getChildByName ("Params") == nullptr)
        return false;

    // Copy to user presets directory
    auto destName = file.getFileNameWithoutExtension();
    auto dest = getUserPresetsDirectory().getChildFile (destName + ".cpreset");

    // Avoid overwrites
    int counter = 1;
    while (dest.existsAsFile())
    {
        dest = getUserPresetsDirectory().getChildFile (destName + " (" + juce::String (counter++) + ").cpreset");
    }

    file.copyFileTo (dest);
    rescanUserPresets();
    return true;
}

bool PresetManager::exportPreset (int index, const juce::File& destination)
{
    if (index < 0 || index >= static_cast<int> (allPresets.size()))
        return false;

    auto& info = allPresets[static_cast<size_t> (index)];

    auto xml = std::make_unique<juce::XmlElement> ("CharsiesiPreset");
    xml->setAttribute ("name", info.name);
    xml->setAttribute ("author", info.author);
    xml->setAttribute ("category", info.category);
    xml->setAttribute ("description", info.description);
    xml->setAttribute ("version", "2.0");

    auto& state = presetStates[static_cast<size_t> (index)];
    auto paramsElem = xml->createNewChildElement ("Params");
    for (int i = 0; i < state.getNumProperties(); ++i)
    {
        auto propName = state.getPropertyName (i);
        paramsElem->setAttribute (propName.toString(), static_cast<double> (state[propName]));
    }

    return xml->writeTo (destination);
}

//==============================================================================
// Internal
//==============================================================================
void PresetManager::applyPresetToAPVTS (const juce::ValueTree& preset)
{
    for (int i = 0; i < preset.getNumProperties(); ++i)
    {
        auto propName = preset.getPropertyName (i).toString();
        auto value = static_cast<float> (preset.getProperty (preset.getPropertyName (i)));

        if (auto* param = apvts.getParameter (propName))
        {
            // Convert from raw value to normalized
            param->setValueNotifyingHost (param->convertTo0to1 (value));
        }
    }
}

juce::ValueTree PresetManager::captureCurrentState() const
{
    juce::ValueTree state ("PresetState");

    // Iterate all parameters in the APVTS and store their values
    auto& params = apvts.processor.getParameters();
    for (auto* param : params)
    {
        if (auto* rangedParam = dynamic_cast<juce::RangedAudioParameter*> (param))
        {
            auto id = rangedParam->getParameterID();
            auto value = rangedParam->convertFrom0to1 (rangedParam->getValue());
            state.setProperty (juce::Identifier (id), value, nullptr);
        }
    }

    return state;
}

void PresetManager::notifyListeners()
{
    for (auto* l : listeners)
        l->presetChanged();
}

void PresetManager::notifyListChanged()
{
    for (auto* l : listeners)
        l->presetListChanged();
}

//==============================================================================
// Embedded factory presets (generated from FactoryPresets.xml)
// This avoids needing BinaryData for now - easy to switch later.
//==============================================================================
static const char* getFactoryPresetsXML()
{
    return R"XML(<?xml version="1.0" encoding="UTF-8"?>
<FactoryPresets>

  <Preset name="Init" category="Subtle" author="flarkAUDIO" description="Default starting point">
    <Params voices="4" minRate="1.0" rateRange="2.0" rateUpdate="10.0" minDelay="2.0" delayRange="5.0" stereoMode="0" mix="50.0" feedback="0.0" rotation="0.0" pregain="0.0" postgain="0.0" lowpass="100.0" lpRes="0.0" highpass="0.0" hpRes="0.0" followLevel="0" envLP="0.0" envHP="0.0" envDelay="0.0" lfoShape="0" lfoRate="1.0" lfoUnit="0" lfoQuant="0" lfoLP="0.0" lfoHP="0.0" lfoDelay="0.0" seq0="50" seq1="50" seq2="50" seq3="50" seq4="50" seq5="50" seq6="50" seq7="50" seqStep="1.0" seqUnit="0" seqQuant="0" seqLP="0.0" seqHP="0.0" seqDelay="0.0" updateUnit="0" updateQuantize="0" updateOnCollision="0"/>
  </Preset>

  <Preset name="Warm Detune" category="Subtle" author="flarkAUDIO" description="Gentle doubling with slight pitch variation">
    <Params voices="2" minRate="0.5" rateRange="0.3" rateUpdate="100.0" minDelay="1.0" delayRange="1.5" stereoMode="2" mix="50.0" feedback="0.0" rotation="0.0" pregain="0.0" postgain="0.0" lowpass="80.0" lpRes="0.0" highpass="0.0" hpRes="0.0" followLevel="0" envLP="0.0" envHP="0.0" envDelay="0.0" lfoShape="0" lfoRate="1.0" lfoUnit="0" lfoQuant="0" lfoLP="0.0" lfoHP="0.0" lfoDelay="0.0" seq0="50" seq1="50" seq2="50" seq3="50" seq4="50" seq5="50" seq6="50" seq7="50" seqStep="1.0" seqUnit="0" seqQuant="0" seqLP="0.0" seqHP="0.0" seqDelay="0.0" updateUnit="0" updateQuantize="0" updateOnCollision="0"/>
  </Preset>

  <Preset name="Slow Drift" category="Subtle" author="flarkAUDIO" description="Glacially slow modulation for pads">
    <Params voices="4" minRate="0.1" rateRange="0.2" rateUpdate="80.0" minDelay="3.0" delayRange="6.0" stereoMode="0" mix="40.0" feedback="10.0" rotation="0.3" pregain="0.0" postgain="0.0" lowpass="100.0" lpRes="0.0" highpass="0.0" hpRes="0.0" followLevel="0" envLP="0.0" envHP="0.0" envDelay="0.0" lfoShape="0" lfoRate="0.05" lfoUnit="2" lfoQuant="0" lfoLP="0.0" lfoHP="0.0" lfoDelay="1.0" seq0="50" seq1="50" seq2="50" seq3="50" seq4="50" seq5="50" seq6="50" seq7="50" seqStep="1.0" seqUnit="0" seqQuant="0" seqLP="0.0" seqHP="0.0" seqDelay="0.0" updateUnit="0" updateQuantize="0" updateOnCollision="0"/>
  </Preset>

  <Preset name="Silk Thread" category="Subtle" author="flarkAUDIO" description="Ultra-clean thickening, minimal movement">
    <Params voices="2" minRate="0.3" rateRange="0.1" rateUpdate="50.0" minDelay="0.8" delayRange="0.5" stereoMode="1" mix="35.0" feedback="0.0" rotation="0.0" pregain="0.0" postgain="0.0" lowpass="70.0" lpRes="0.0" highpass="5.0" hpRes="0.0" followLevel="0" envLP="0.0" envHP="0.0" envDelay="0.0" lfoShape="0" lfoRate="1.0" lfoUnit="0" lfoQuant="0" lfoLP="0.0" lfoHP="0.0" lfoDelay="0.0" seq0="50" seq1="50" seq2="50" seq3="50" seq4="50" seq5="50" seq6="50" seq7="50" seqStep="1.0" seqUnit="0" seqQuant="0" seqLP="0.0" seqHP="0.0" seqDelay="0.0" updateUnit="0" updateQuantize="0" updateOnCollision="0"/>
  </Preset>

  <Preset name="Lush Chorus" category="Chorus" author="flarkAUDIO" description="Rich, classic chorus with wide stereo spread">
    <Params voices="6" minRate="0.8" rateRange="3.0" rateUpdate="15.0" minDelay="3.0" delayRange="8.0" stereoMode="0" mix="55.0" feedback="10.0" rotation="0.25" pregain="0.0" postgain="0.0" lowpass="85.0" lpRes="5.0" highpass="5.0" hpRes="0.0" followLevel="0" envLP="0.0" envHP="0.0" envDelay="0.0" lfoShape="0" lfoRate="1.0" lfoUnit="0" lfoQuant="0" lfoLP="0.0" lfoHP="0.0" lfoDelay="0.0" seq0="50" seq1="50" seq2="50" seq3="50" seq4="50" seq5="50" seq6="50" seq7="50" seqStep="1.0" seqUnit="0" seqQuant="0" seqLP="0.0" seqHP="0.0" seqDelay="0.0" updateUnit="0" updateQuantize="0" updateOnCollision="0"/>
  </Preset>

  <Preset name="Vintage Rotary" category="Chorus" author="flarkAUDIO" description="Leslie-style rotary speaker simulation">
    <Params voices="2" minRate="5.0" rateRange="1.0" rateUpdate="50.0" minDelay="1.5" delayRange="2.0" stereoMode="3" mix="60.0" feedback="5.0" rotation="0.5" pregain="0.0" postgain="0.0" lowpass="70.0" lpRes="10.0" highpass="10.0" hpRes="0.0" followLevel="0" envLP="0.0" envHP="0.0" envDelay="0.0" lfoShape="0" lfoRate="5.5" lfoUnit="0" lfoQuant="0" lfoLP="0.5" lfoHP="0.0" lfoDelay="1.0" seq0="50" seq1="50" seq2="50" seq3="50" seq4="50" seq5="50" seq6="50" seq7="50" seqStep="1.0" seqUnit="0" seqQuant="0" seqLP="0.0" seqHP="0.0" seqDelay="0.0" updateUnit="0" updateQuantize="0" updateOnCollision="0"/>
  </Preset>

  <Preset name="Tape Wobble" category="Chorus" author="flarkAUDIO" description="Warm, imperfect tape machine chorus">
    <Params voices="3" minRate="1.5" rateRange="4.0" rateUpdate="8.0" minDelay="4.0" delayRange="6.0" stereoMode="0" mix="45.0" feedback="8.0" rotation="0.15" pregain="0.0" postgain="0.0" lowpass="65.0" lpRes="8.0" highpass="8.0" hpRes="0.0" followLevel="0" envLP="0.0" envHP="0.0" envDelay="0.0" lfoShape="4" lfoRate="2.0" lfoUnit="0" lfoQuant="0" lfoLP="0.0" lfoHP="0.0" lfoDelay="2.5" seq0="50" seq1="50" seq2="50" seq3="50" seq4="50" seq5="50" seq6="50" seq7="50" seqStep="1.0" seqUnit="0" seqQuant="0" seqLP="0.0" seqHP="0.0" seqDelay="0.0" updateUnit="0" updateQuantize="0" updateOnCollision="0"/>
  </Preset>

  <Preset name="Wide Ensemble" category="Ensemble" author="flarkAUDIO" description="Full 8-voice string ensemble effect">
    <Params voices="8" minRate="0.5" rateRange="4.0" rateUpdate="20.0" minDelay="4.0" delayRange="10.0" stereoMode="0" mix="45.0" feedback="15.0" rotation="0.5" pregain="0.0" postgain="0.0" lowpass="90.0" lpRes="0.0" highpass="3.0" hpRes="0.0" followLevel="0" envLP="0.0" envHP="0.0" envDelay="0.0" lfoShape="0" lfoRate="1.0" lfoUnit="0" lfoQuant="0" lfoLP="0.0" lfoHP="0.0" lfoDelay="0.0" seq0="50" seq1="50" seq2="50" seq3="50" seq4="50" seq5="50" seq6="50" seq7="50" seqStep="1.0" seqUnit="0" seqQuant="0" seqLP="0.0" seqHP="0.0" seqDelay="0.0" updateUnit="0" updateQuantize="0" updateOnCollision="0"/>
  </Preset>

  <Preset name="Shimmer Pad" category="Ensemble" author="flarkAUDIO" description="Ethereal shimmer with LFO-modulated filters">
    <Params voices="6" minRate="0.3" rateRange="1.5" rateUpdate="30.0" minDelay="5.0" delayRange="12.0" stereoMode="0" mix="40.0" feedback="25.0" rotation="0.3" pregain="0.0" postgain="0.0" lowpass="75.0" lpRes="15.0" highpass="15.0" hpRes="5.0" followLevel="0" envLP="0.0" envHP="0.0" envDelay="0.0" lfoShape="0" lfoRate="0.2" lfoUnit="2" lfoQuant="0" lfoLP="1.5" lfoHP="0.0" lfoDelay="3.0" seq0="50" seq1="50" seq2="50" seq3="50" seq4="50" seq5="50" seq6="50" seq7="50" seqStep="1.0" seqUnit="0" seqQuant="0" seqLP="0.0" seqHP="0.0" seqDelay="0.0" updateUnit="0" updateQuantize="0" updateOnCollision="0"/>
  </Preset>

  <Preset name="Deep Space" category="Ensemble" author="flarkAUDIO" description="Vast, slowly evolving ambient texture">
    <Params voices="8" minRate="0.2" rateRange="0.5" rateUpdate="50.0" minDelay="8.0" delayRange="15.0" stereoMode="0" mix="35.0" feedback="35.0" rotation="0.4" pregain="0.0" postgain="-3.0" lowpass="65.0" lpRes="10.0" highpass="5.0" hpRes="0.0" followLevel="0" envLP="0.0" envHP="0.0" envDelay="0.0" lfoShape="5" lfoRate="0.1" lfoUnit="2" lfoQuant="0" lfoLP="0.0" lfoHP="0.0" lfoDelay="5.0" seq0="50" seq1="50" seq2="50" seq3="50" seq4="50" seq5="50" seq6="50" seq7="50" seqStep="1.0" seqUnit="0" seqQuant="0" seqLP="0.0" seqHP="0.0" seqDelay="0.0" updateUnit="0" updateQuantize="0" updateOnCollision="0"/>
  </Preset>

  <Preset name="Broken Tape" category="Creative" author="flarkAUDIO" description="Lo-fi warble with collision re-triggering">
    <Params voices="3" minRate="2.0" rateRange="8.0" rateUpdate="5.0" minDelay="6.0" delayRange="15.0" stereoMode="0" mix="50.0" feedback="20.0" rotation="0.0" pregain="0.0" postgain="0.0" lowpass="60.0" lpRes="20.0" highpass="8.0" hpRes="0.0" followLevel="0" envLP="0.0" envHP="0.0" envDelay="0.0" lfoShape="0" lfoRate="1.0" lfoUnit="0" lfoQuant="0" lfoLP="0.0" lfoHP="0.0" lfoDelay="0.0" seq0="50" seq1="50" seq2="50" seq3="50" seq4="50" seq5="50" seq6="50" seq7="50" seqStep="1.0" seqUnit="0" seqQuant="0" seqLP="0.0" seqHP="0.0" seqDelay="0.0" updateUnit="0" updateQuantize="0" updateOnCollision="1"/>
  </Preset>

  <Preset name="Metallic Flutter" category="Creative" author="flarkAUDIO" description="Fast, resonant modulation for metallic textures">
    <Params voices="4" minRate="8.0" rateRange="12.0" rateUpdate="3.0" minDelay="0.5" delayRange="1.0" stereoMode="0" mix="45.0" feedback="30.0" rotation="0.2" pregain="0.0" postgain="0.0" lowpass="95.0" lpRes="25.0" highpass="20.0" hpRes="15.0" followLevel="0" envLP="0.0" envHP="0.0" envDelay="0.0" lfoShape="0" lfoRate="1.0" lfoUnit="0" lfoQuant="0" lfoLP="0.0" lfoHP="0.0" lfoDelay="0.0" seq0="50" seq1="50" seq2="50" seq3="50" seq4="50" seq5="50" seq6="50" seq7="50" seqStep="1.0" seqUnit="0" seqQuant="0" seqLP="0.0" seqHP="0.0" seqDelay="0.0" updateUnit="0" updateQuantize="0" updateOnCollision="0"/>
  </Preset>

  <Preset name="Dark Swirl" category="Creative" author="flarkAUDIO" description="Moody, envelope-reactive chorus with LFO filter sweep">
    <Params voices="6" minRate="1.5" rateRange="5.0" rateUpdate="12.0" minDelay="4.0" delayRange="10.0" stereoMode="0" mix="55.0" feedback="20.0" rotation="0.6" pregain="0.0" postgain="0.0" lowpass="50.0" lpRes="30.0" highpass="10.0" hpRes="10.0" followLevel="2" envLP="1.5" envHP="0.0" envDelay="2.0" lfoShape="2" lfoRate="0.5" lfoUnit="2" lfoQuant="0" lfoLP="2.0" lfoHP="1.0" lfoDelay="4.0" seq0="50" seq1="50" seq2="50" seq3="50" seq4="50" seq5="50" seq6="50" seq7="50" seqStep="1.0" seqUnit="0" seqQuant="0" seqLP="0.0" seqHP="0.0" seqDelay="0.0" updateUnit="0" updateQuantize="0" updateOnCollision="0"/>
  </Preset>

  <Preset name="Underwater" category="Creative" author="flarkAUDIO" description="Submerged, heavily filtered modulation">
    <Params voices="5" minRate="0.4" rateRange="2.0" rateUpdate="25.0" minDelay="6.0" delayRange="12.0" stereoMode="0" mix="60.0" feedback="30.0" rotation="0.35" pregain="0.0" postgain="-2.0" lowpass="40.0" lpRes="35.0" highpass="3.0" hpRes="0.0" followLevel="1" envLP="2.0" envHP="0.0" envDelay="3.0" lfoShape="5" lfoRate="0.3" lfoUnit="2" lfoQuant="0" lfoLP="1.5" lfoHP="0.0" lfoDelay="4.0" seq0="50" seq1="50" seq2="50" seq3="50" seq4="50" seq5="50" seq6="50" seq7="50" seqStep="1.0" seqUnit="0" seqQuant="0" seqLP="0.0" seqHP="0.0" seqDelay="0.0" updateUnit="0" updateQuantize="0" updateOnCollision="0"/>
  </Preset>

  <Preset name="Seq Pump" category="Rhythmic" author="flarkAUDIO" description="Sequencer-driven rhythmic chorus">
    <Params voices="4" minRate="1.0" rateRange="2.0" rateUpdate="10.0" minDelay="3.0" delayRange="5.0" stereoMode="0" mix="50.0" feedback="15.0" rotation="0.25" pregain="0.0" postgain="0.0" lowpass="85.0" lpRes="10.0" highpass="5.0" hpRes="0.0" followLevel="0" envLP="0.0" envHP="0.0" envDelay="0.0" lfoShape="0" lfoRate="1.0" lfoUnit="0" lfoQuant="0" lfoLP="0.0" lfoHP="0.0" lfoDelay="0.0" seq0="90" seq1="30" seq2="70" seq3="10" seq4="80" seq5="20" seq6="60" seq7="40" seqStep="4.0" seqUnit="4" seqQuant="1" seqLP="1.0" seqHP="0.0" seqDelay="5.0" updateUnit="0" updateQuantize="0" updateOnCollision="0"/>
  </Preset>

  <Preset name="Stutter Gate" category="Rhythmic" author="flarkAUDIO" description="Sharp on/off sequencer pattern">
    <Params voices="4" minRate="1.0" rateRange="2.0" rateUpdate="10.0" minDelay="2.0" delayRange="4.0" stereoMode="0" mix="55.0" feedback="10.0" rotation="0.2" pregain="0.0" postgain="0.0" lowpass="90.0" lpRes="5.0" highpass="3.0" hpRes="0.0" followLevel="0" envLP="0.0" envHP="0.0" envDelay="0.0" lfoShape="3" lfoRate="2.0" lfoUnit="5" lfoQuant="1" lfoLP="0.0" lfoHP="0.0" lfoDelay="0.0" seq0="100" seq1="0" seq2="100" seq3="0" seq4="100" seq5="100" seq6="0" seq7="50" seqStep="2.0" seqUnit="4" seqQuant="1" seqLP="0.0" seqHP="0.0" seqDelay="8.0" updateUnit="0" updateQuantize="0" updateOnCollision="0"/>
  </Preset>

  <Preset name="Triplet Bounce" category="Rhythmic" author="flarkAUDIO" description="Synced to triplet grid with filter movement">
    <Params voices="3" minRate="0.8" rateRange="1.5" rateUpdate="15.0" minDelay="2.5" delayRange="5.0" stereoMode="0" mix="48.0" feedback="12.0" rotation="0.3" pregain="0.0" postgain="0.0" lowpass="80.0" lpRes="15.0" highpass="8.0" hpRes="0.0" followLevel="0" envLP="0.0" envHP="0.0" envDelay="0.0" lfoShape="2" lfoRate="1.0" lfoUnit="8" lfoQuant="1" lfoLP="1.0" lfoHP="0.5" lfoDelay="2.0" seq0="80" seq1="40" seq2="90" seq3="30" seq4="70" seq5="50" seq6="85" seq7="35" seqStep="1.0" seqUnit="7" seqQuant="1" seqLP="0.5" seqHP="0.0" seqDelay="3.0" updateUnit="0" updateQuantize="0" updateOnCollision="0"/>
  </Preset>

</FactoryPresets>
)XML";
}
