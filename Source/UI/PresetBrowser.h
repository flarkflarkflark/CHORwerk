#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../Presets/PresetManager.h"
#include "LookAndFeel.h"

/**
 * Custom icon button for the preset browser.
 */
class IconButton : public juce::Button
{
public:
    enum IconType { Prev, Next, Save, AB, Undo, Zoom };

    IconButton (IconType type) : Button (""), iconType (type) {}

    void paintButton (juce::Graphics& g, bool isMouseOver, bool isButtonDown) override
    {
        auto bounds = getLocalBounds().toFloat().reduced (1.5f);
        auto cornerSize = 6.0f;
        bool isToggled = getToggleState();
        
        auto baseColor = isButtonDown ? juce::Colour (0xff334155) 
                                     : (isMouseOver ? juce::Colour (0xff1e293b) : juce::Colour (0xff111c2a));
        
        if (isToggled)
            baseColor = juce::Colour (CHORwerkLookAndFeel::Colors::voices).withAlpha (0.4f);

        juce::ColourGradient grad (baseColor.brighter (0.05f), 0, bounds.getY(),
                                   baseColor.darker (0.1f), 0, bounds.getBottom(), false);
        g.setGradientFill (grad);
        g.fillRoundedRectangle (bounds, cornerSize);
        
        auto highlightArea = bounds.reduced (1.0f);
        g.setColour (juce::Colour (0xffffffff).withAlpha (isMouseOver ? 0.08f : 0.04f));
        g.drawRoundedRectangle (highlightArea, cornerSize - 1.0f, 1.0f);

        g.setColour (isToggled ? juce::Colour (CHORwerkLookAndFeel::Colors::voices).withAlpha (0.9f) 
                               : juce::Colour (CHORwerkLookAndFeel::Colors::border).withAlpha (isMouseOver ? 0.6f : 0.3f));
        g.drawRoundedRectangle (bounds, cornerSize, 1.0f);

        auto iconArea = bounds.reduced (bounds.getHeight() * 0.32f);
        g.setColour (isEnabled() ? juce::Colour (CHORwerkLookAndFeel::Colors::textPrimary) 
                                   : juce::Colour (CHORwerkLookAndFeel::Colors::textDim));
        
        juce::Path p;
        if (iconType == Prev) {
            p.addTriangle (iconArea.getRight(), iconArea.getY(), 
                           iconArea.getRight(), iconArea.getBottom(), 
                           iconArea.getX(), iconArea.getCentreY());
        } else if (iconType == Next) {
            p.addTriangle (iconArea.getX(), iconArea.getY(), 
                           iconArea.getX(), iconArea.getBottom(), 
                           iconArea.getRight(), iconArea.getCentreY());
        } else if (iconType == Save) {
            p.addRoundedRectangle (iconArea.reduced (1, 0), 1.0f);
            p.addRectangle (iconArea.getCentreX() - 2, iconArea.getY(), 4, 3);
        } else if (iconType == AB) {
            g.setFont (juce::Font ("JetBrains Mono", iconArea.getHeight() * 1.2f, juce::Font::bold));
            g.drawText (getButtonText(), iconArea, juce::Justification::centred);
            return;
        } else if (iconType == Undo) {
            p.addCentredArc (iconArea.getCentreX(), iconArea.getCentreY(), 
                             iconArea.getWidth() * 0.4f, iconArea.getHeight() * 0.4f, 
                             0.0f, 0.5f, 5.5f, true);
            p.startNewSubPath (iconArea.getX(), iconArea.getCentreY());
            p.lineTo (iconArea.getX() + 4, iconArea.getCentreY() - 3);
            p.lineTo (iconArea.getX() + 4, iconArea.getCentreY() + 3);
            p.closeSubPath();
        } else if (iconType == Zoom) {
            p.addEllipse (iconArea.reduced (2));
            p.startNewSubPath (iconArea.getCentreX(), iconArea.getY() + 4);
            p.lineTo (iconArea.getCentreX(), iconArea.getBottom() - 4);
            p.startNewSubPath (iconArea.getX() + 4, iconArea.getCentreY());
            p.lineTo (iconArea.getRight() - 4, iconArea.getCentreY());
        }
        
        g.strokePath (p, juce::PathStrokeType (1.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    }

private:
    IconType iconType;
};

/**
 * Text button for preset name with relative scaling.
 */
class NameButton : public juce::Button
{
public:
    NameButton() : Button ("") {}

    void paintButton (juce::Graphics& g, bool isMouseOver, bool isButtonDown) override
    {
        auto bounds = getLocalBounds().toFloat();
        
        g.setColour (isButtonDown ? juce::Colour (0xff152030) : juce::Colour (0xff0a1018));
        g.fillRoundedRectangle (bounds, 4.0f);
        g.setColour (juce::Colour (0xff1a2a3a).withAlpha (isMouseOver ? 0.8f : 0.4f));
        g.drawRoundedRectangle (bounds, 4.0f, 1.0f);

        g.setColour (juce::Colour (0xffe2e8f0));
        float fontH = bounds.getHeight() * 0.45f;
        if (fontH < 8.0f) fontH = 8.0f;
        g.setFont (juce::Font ("JetBrains Mono", fontH, juce::Font::bold));
        g.drawText (getButtonText(), bounds, juce::Justification::centred);
    }
};

/**
 * Compact preset browser bar for the plugin header.
 */
class PresetBrowser : public juce::Component,
                      public PresetManager::Listener
{
public:
    PresetBrowser (PresetManager& pm)
        : presetManager (pm),
          prevButton (IconButton::Prev),
          nextButton (IconButton::Next),
          saveButton (IconButton::Save),
          aButton (IconButton::AB),
          bButton (IconButton::AB),
          undoButton (IconButton::Undo),
          zoomButton (IconButton::Zoom)
    {
        presetManager.addListener (this);

        addAndMakeVisible (prevButton);
        prevButton.setTooltip ("Previous preset");
        prevButton.onClick = [this] { presetManager.loadPreviousPreset(); };

        addAndMakeVisible (nextButton);
        nextButton.setTooltip ("Next preset");
        nextButton.onClick = [this] { presetManager.loadNextPreset(); };

        addAndMakeVisible (nameButton);
        nameButton.setTooltip ("Click to browse presets. Right-click to manage.");
        nameButton.onClick = [this] { showPresetMenu(); };
        nameButton.setMouseCursor (juce::MouseCursor::PointingHandCursor);

        addAndMakeVisible (categoryBox);
        categoryBox.setTooltip ("Filter presets by category");
        auto cats = PresetManager::getCategoryNames();
        for (int i = 0; i < cats.size(); ++i)
            categoryBox.addItem (cats[i], i + 1);
        categoryBox.setSelectedId (1, juce::dontSendNotification);
        categoryBox.onChange = [this]
        {
            auto idx = categoryBox.getSelectedId() - 1;
            presetManager.setCategory (static_cast<PresetManager::Category> (idx));
        };

        addAndMakeVisible (saveButton);
        saveButton.setTooltip ("Save/Manage presets");
        saveButton.onClick = [this] { showSaveMenu(); };

        addAndMakeVisible (aButton);
        aButton.setButtonText ("A");
        aButton.setTooltip ("State A");
        aButton.setClickingTogglesState (true);
        aButton.setRadioGroupId (100);
        aButton.onClick = [this] { 
            if (! presetManager.isShowingA()) presetManager.toggleAB(); 
            updateDisplay();
        };

        addAndMakeVisible (bButton);
        bButton.setButtonText ("B");
        bButton.setTooltip ("State B");
        bButton.setClickingTogglesState (true);
        bButton.setRadioGroupId (100);
        bButton.onClick = [this] { 
            if (presetManager.isShowingA()) presetManager.toggleAB(); 
            updateDisplay();
        };

        addAndMakeVisible (undoButton);
        undoButton.setTooltip ("Undo last preset change");
        undoButton.onClick = [this] { presetManager.undoPresetChange(); };
        undoButton.setEnabled (false);

        addAndMakeVisible (zoomButton);
        zoomButton.setTooltip ("UI Scale (25% - 400%)");
        zoomButton.onClick = [this] { showZoomMenu(); };

        updateDisplay();
    }

    ~PresetBrowser() override { presetManager.removeListener (this); }

    void resized() override
    {
        auto b = getLocalBounds().reduced (2, 2);
        int btnW = b.getHeight();

        prevButton.setBounds (b.removeFromLeft (btnW));
        b.removeFromLeft (4);
        
        // Slightly less for right side to avoid pushing name too far left
        auto rightSide = b.removeFromRight (static_cast<int> (b.getWidth() * 0.55f));
        
        nameButton.setBounds (b.removeFromLeft (b.getWidth() - btnW - 4));
        b.removeFromLeft (4);
        nextButton.setBounds (b.removeFromLeft (btnW));

        zoomButton.setBounds (rightSide.removeFromRight (btnW));
        rightSide.removeFromRight (4);

        undoButton.setBounds (rightSide.removeFromRight (btnW));
        rightSide.removeFromRight (8);

        bButton.setBounds (rightSide.removeFromRight (btnW));
        rightSide.removeFromRight (2);
        aButton.setBounds (rightSide.removeFromRight (btnW));
        rightSide.removeFromRight (8);

        saveButton.setBounds (rightSide.removeFromRight (btnW));
        rightSide.removeFromRight (8);

        categoryBox.setBounds (rightSide);
    }

    void presetChanged() override { updateDisplay(); }
    void presetListChanged() override { updateDisplay(); }

    std::function<void(float)> onScaleChanged;

private:
    void updateDisplay()
    {
        nameButton.setButtonText (presetManager.getCurrentPresetName());
        undoButton.setEnabled (presetManager.canUndoPresetChange());
        
        bool isA = presetManager.isShowingA();
        aButton.setToggleState (isA, juce::dontSendNotification);
        bButton.setToggleState (!isA, juce::dontSendNotification);

        auto catIdx = static_cast<int> (presetManager.getCategory());
        categoryBox.setSelectedId (catIdx + 1, juce::dontSendNotification);
        repaint();
    }

    void showPresetMenu()
    {
        juce::PopupMenu menu;
        auto categories = PresetManager::getCategoryNames();
        for (int c = 1; c < categories.size(); ++c)
        {
            juce::PopupMenu subMenu;
            bool hasItems = false;
            for (int i = 0; i < presetManager.getNumPresets(); ++i)
            {
                auto& info = presetManager.getPresetInfo (i);
                if (info.category.equalsIgnoreCase (categories[c]))
                {
                    bool isCurrent = (i == presetManager.getCurrentPresetIndex());
                    juce::String displayName = info.name;
                    if (! info.isFactory) displayName += " (user)";
                    subMenu.addItem (i + 1, displayName, true, isCurrent);
                    hasItems = true;
                }
            }
            if (hasItems) menu.addSubMenu (categories[c], subMenu);
        }
        menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&nameButton).withMinimumWidth (200),
                            [this] (int result) { if (result > 0) presetManager.loadPreset (result - 1); });
    }

    void showSaveMenu()
    {
        juce::PopupMenu menu;
        menu.addItem (1, "Save As New...");
        auto& current = presetManager.getCurrentPresetInfo();
        if (! current.isFactory) {
            menu.addItem (2, "Save (overwrite \"" + current.name + "\")");
            menu.addSeparator();
            menu.addItem (3, "Rename...");
            menu.addItem (4, "Delete");
        }
        menu.addSeparator();
        menu.addItem (5, "Import Preset...");
        menu.addItem (6, "Export Current Preset...");
        menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&saveButton).withMinimumWidth (180),
                            [this] (int result) { handleSaveMenuResult (result); });
    }

    void handleSaveMenuResult (int result)
    {
        switch (result)
        {
            case 1: showSaveAsDialog(); break;
            case 2: presetManager.saveCurrentPreset(); break;
            case 3: showRenameDialog(); break;
            case 4: showDeleteConfirmation(); break;
            case 5: showImportDialog(); break;
            case 6: showExportDialog(); break;
            default: break;
        }
    }

    void showSaveAsDialog()
    {
        auto* window = new SavePresetWindow (presetManager);
        window->enterModalState (true, juce::ModalCallbackFunction::create ([window] (int) { delete window; }), true);
    }

    void showRenameDialog()
    {
        auto currentIdx = presetManager.getCurrentPresetIndex();
        auto& info = presetManager.getPresetInfo (currentIdx);
        auto* alertWindow = new juce::AlertWindow ("Rename Preset", "Enter new name:", juce::MessageBoxIconType::QuestionIcon);
        alertWindow->addTextEditor ("name", info.name);
        alertWindow->addButton ("Rename", 1);
        alertWindow->addButton ("Cancel", 0);
        alertWindow->enterModalState (true, juce::ModalCallbackFunction::create ([this, alertWindow, currentIdx] (int result) {
            if (result == 1) {
                auto newName = alertWindow->getTextEditorContents ("name");
                if (newName.isNotEmpty()) presetManager.renameUserPreset (currentIdx, newName);
            }
            delete alertWindow;
        }), true);
    }

    void showDeleteConfirmation()
    {
        auto currentIdx = presetManager.getCurrentPresetIndex();
        auto& info = presetManager.getPresetInfo (currentIdx);
        auto result = juce::AlertWindow::showOkCancelBox (juce::MessageBoxIconType::WarningIcon, "Delete Preset",
                                                        "Delete \"" + info.name + "\"? This cannot be undone.", "Delete", "Cancel", nullptr, nullptr);
        if (result) presetManager.deleteUserPreset (currentIdx);
    }

    void showImportDialog()
    {
        fileChooser = std::make_unique<juce::FileChooser> ("Import Preset", juce::File(), "*.cpreset");
        fileChooser->launchAsync (juce::FileBrowserComponent::openMode, [this] (const juce::FileChooser& fc) {
            auto file = fc.getResult();
            if (file.existsAsFile()) presetManager.importPreset (file);
        });
    }

    void showExportDialog()
    {
        auto& info = presetManager.getCurrentPresetInfo();
        auto defaultFile = juce::File::getSpecialLocation (juce::File::userDesktopDirectory).getChildFile (info.name + ".cpreset");
        fileChooser = std::make_unique<juce::FileChooser> ("Export Preset", defaultFile, "*.cpreset");
        fileChooser->launchAsync (juce::FileBrowserComponent::saveMode, [this] (const juce::FileChooser& fc) {
            auto file = fc.getResult();
            if (file != juce::File()) presetManager.exportPreset (presetManager.getCurrentPresetIndex(), file);
        });
    }

    class SavePresetWindow : public juce::DialogWindow
    {
    public:
        SavePresetWindow (PresetManager& pm) : DialogWindow ("Save Preset", juce::Colour (0xff0c1520), true), presetManager (pm)
        {
            setContentOwned (new Content (pm, *this), true);
            centreWithSize (320, 240);
            setResizable (false, false);
        }
        void closeButtonPressed() override { exitModalState (0); }
    private:
        class Content : public juce::Component {
        public:
            Content (PresetManager& pm, SavePresetWindow& w) : presetManager (pm), window (w) {
                addAndMakeVisible (nameLabel); nameLabel.setText ("Name:", juce::dontSendNotification);
                addAndMakeVisible (nameEditor);
                addAndMakeVisible (categoryLabel); categoryLabel.setText ("Category:", juce::dontSendNotification);
                addAndMakeVisible (categoryBox);
                categoryBox.addItem ("Subtle", 1); categoryBox.addItem ("Chorus", 2); categoryBox.addItem ("Ensemble", 3);
                categoryBox.addItem ("Creative", 4); categoryBox.addItem ("Rhythmic", 5); categoryBox.addItem ("User", 6);
                categoryBox.setSelectedId (6);
                addAndMakeVisible (authorLabel); authorLabel.setText ("Author:", juce::dontSendNotification);
                addAndMakeVisible (authorEditor);
                addAndMakeVisible (descLabel); descLabel.setText ("Description:", juce::dontSendNotification);
                addAndMakeVisible (descEditor); descEditor.setMultiLine (true);
                addAndMakeVisible (saveBtn); saveBtn.setButtonText ("Save");
                saveBtn.onClick = [this] {
                    auto name = nameEditor.getText().trim();
                    if (name.isEmpty()) return;
                    presetManager.saveUserPreset (name, "User", authorEditor.getText(), descEditor.getText());
                    window.exitModalState (1);
                };
                addAndMakeVisible (cancelBtn); cancelBtn.setButtonText ("Cancel");
                cancelBtn.onClick = [this] { window.exitModalState (0); };
                setSize (300, 220);
            }
            void resized() override {
                auto b = getLocalBounds().reduced (12);
                auto row = b.removeFromTop (24); nameLabel.setBounds (row.removeFromLeft (80)); nameEditor.setBounds (row);
                b.removeFromTop (4); row = b.removeFromTop (24); categoryLabel.setBounds (row.removeFromLeft (80)); categoryBox.setBounds (row);
                b.removeFromTop (4); row = b.removeFromTop (24); authorLabel.setBounds (row.removeFromLeft (80)); authorEditor.setBounds (row);
                b.removeFromTop (4); row = b.removeFromTop (24); descLabel.setBounds (row.removeFromLeft (80)); descEditor.setBounds (row.withHeight (40));
                auto buttons = b.removeFromBottom (28); cancelBtn.setBounds (buttons.removeFromRight (70)); saveBtn.setBounds (buttons.removeFromLeft (70));
            }
            void paint (juce::Graphics& g) override { g.fillAll (juce::Colour (0xff0c1520)); }
        private:
            PresetManager& presetManager; SavePresetWindow& window;
            juce::Label nameLabel, categoryLabel, authorLabel, descLabel;
            juce::TextEditor nameEditor, authorEditor, descEditor;
            juce::ComboBox categoryBox; juce::TextButton saveBtn, cancelBtn;
        };
        PresetManager& presetManager;
    };

    void showZoomMenu()
    {
        juce::PopupMenu menu;
        menu.addItem (1, "25%", true, false);
        menu.addItem (2, "50%", true, false);
        menu.addItem (3, "75%", true, false);
        menu.addItem (4, "100%", true, true);
        menu.addItem (5, "125%", true, false);
        menu.addItem (6, "150%", true, false);
        menu.addItem (7, "200%", true, false);
        menu.addItem (8, "300%", true, false);
        menu.addItem (9, "400%", true, false);

        menu.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (&zoomButton),
            [this](int result) {
                if (result == 0) return;
                float scales[] = { 0.25f, 0.5f, 0.75f, 1.0f, 1.25f, 1.5f, 2.0f, 3.0f, 4.0f };
                if (onScaleChanged) onScaleChanged (scales[result - 1]);
            });
    }

    PresetManager& presetManager;
    IconButton prevButton, nextButton, saveButton, aButton, bButton, undoButton, zoomButton;
    NameButton nameButton;
    juce::ComboBox categoryBox;
    std::unique_ptr<juce::FileChooser> fileChooser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetBrowser)
};
