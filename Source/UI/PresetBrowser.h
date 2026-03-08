#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../Presets/PresetManager.h"
#include "LookAndFeel.h"

/**
 * Compact preset browser bar for the plugin header.
 *
 * Layout: [◀] [preset name / dirty indicator] [▶] [category ▼] [Save] [A/B]
 *
 * Clicking the preset name opens a full dropdown list.
 * Right-clicking a user preset gives rename/delete options.
 */
class PresetBrowser : public juce::Component,
                      public PresetManager::Listener
{
public:
    PresetBrowser (PresetManager& pm)
        : presetManager (pm)
    {
        presetManager.addListener (this);

        // Prev button
        addAndMakeVisible (prevButton);
        prevButton.setButtonText (juce::CharPointer_UTF8 ("\xe2\x97\x80"));
        prevButton.onClick = [this] { presetManager.loadPreviousPreset(); };
        styleNavButton (prevButton);

        // Next button
        addAndMakeVisible (nextButton);
        nextButton.setButtonText (juce::CharPointer_UTF8 ("\xe2\x96\xb6"));
        nextButton.onClick = [this] { presetManager.loadNextPreset(); };
        styleNavButton (nextButton);

        // Preset name (clickable)
        addAndMakeVisible (nameButton);
        nameButton.onClick = [this] { showPresetMenu(); };
        nameButton.setMouseCursor (juce::MouseCursor::PointingHandCursor);
        styleNameButton();

        // Category dropdown
        addAndMakeVisible (categoryBox);
        auto cats = PresetManager::getCategoryNames();
        for (int i = 0; i < cats.size(); ++i)
            categoryBox.addItem (cats[i], i + 1);
        categoryBox.setSelectedId (1, juce::dontSendNotification);
        categoryBox.onChange = [this]
        {
            auto idx = categoryBox.getSelectedId() - 1;
            presetManager.setCategory (static_cast<PresetManager::Category> (idx));
        };

        // Save button
        addAndMakeVisible (saveButton);
        saveButton.setButtonText ("Save");
        saveButton.onClick = [this] { showSaveMenu(); };
        styleActionButton (saveButton);

        // A/B button
        addAndMakeVisible (abButton);
        abButton.setButtonText ("A");
        abButton.onClick = [this]
        {
            presetManager.toggleAB();
            abButton.setButtonText (presetManager.isShowingA() ? "A" : "B");
        };
        styleActionButton (abButton);

        // Undo button
        addAndMakeVisible (undoButton);
        undoButton.setButtonText (juce::CharPointer_UTF8 ("\xe2\x86\xa9"));
        undoButton.onClick = [this] { presetManager.undoPresetChange(); };
        undoButton.setEnabled (false);
        styleActionButton (undoButton);

        updateDisplay();
    }

    ~PresetBrowser() override
    {
        presetManager.removeListener (this);
    }

    void resized() override
    {
        auto b = getLocalBounds().reduced (2, 2);

        prevButton.setBounds (b.removeFromLeft (28));
        b.removeFromLeft (2);

        nextButton.setBounds (b.removeFromRight (28));
        b.removeFromRight (2);

        undoButton.setBounds (b.removeFromRight (28));
        b.removeFromRight (2);

        abButton.setBounds (b.removeFromRight (28));
        b.removeFromRight (2);

        saveButton.setBounds (b.removeFromRight (40));
        b.removeFromRight (4);

        categoryBox.setBounds (b.removeFromRight (80));
        b.removeFromRight (4);

        nameButton.setBounds (b);
    }

    // PresetManager::Listener
    void presetChanged() override { updateDisplay(); }
    void presetListChanged() override { updateDisplay(); }

private:
    void updateDisplay()
    {
        nameButton.setButtonText (presetManager.getCurrentPresetName());
        undoButton.setEnabled (presetManager.canUndoPresetChange());

        auto catIdx = static_cast<int> (presetManager.getCategory());
        categoryBox.setSelectedId (catIdx + 1, juce::dontSendNotification);

        repaint();
    }

    void showPresetMenu()
    {
        juce::PopupMenu menu;

        // Group by category
        auto categories = PresetManager::getCategoryNames();

        for (int c = 1; c < categories.size(); ++c)  // Skip "All"
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
                    if (! info.isFactory)
                        displayName += " (user)";

                    subMenu.addItem (i + 1, displayName, true, isCurrent);
                    hasItems = true;
                }
            }

            if (hasItems)
                menu.addSubMenu (categories[c], subMenu);
        }

        menu.showMenuAsync (juce::PopupMenu::Options()
                                .withTargetComponent (&nameButton)
                                .withMinimumWidth (200),
                            [this] (int result)
                            {
                                if (result > 0)
                                    presetManager.loadPreset (result - 1);
                            });
    }

    void showSaveMenu()
    {
        juce::PopupMenu menu;

        menu.addItem (1, "Save As New...");

        auto& current = presetManager.getCurrentPresetInfo();
        if (! current.isFactory)
        {
            menu.addItem (2, "Save (overwrite \"" + current.name + "\")");
            menu.addSeparator();
            menu.addItem (3, "Rename...");
            menu.addItem (4, "Delete");
        }

        menu.addSeparator();
        menu.addItem (5, "Import Preset...");
        menu.addItem (6, "Export Current Preset...");

        menu.showMenuAsync (juce::PopupMenu::Options()
                                .withTargetComponent (&saveButton)
                                .withMinimumWidth (180),
                            [this] (int result)
                            {
                                handleSaveMenuResult (result);
                            });
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
        window->enterModalState (true, juce::ModalCallbackFunction::create (
            [window] (int) { delete window; }), true);
    }

    void showRenameDialog()
    {
        auto currentIdx = presetManager.getCurrentPresetIndex();
        auto& info = presetManager.getPresetInfo (currentIdx);

        auto* alertWindow = new juce::AlertWindow ("Rename Preset", "Enter new name:", juce::MessageBoxIconType::QuestionIcon);
        alertWindow->addTextEditor ("name", info.name);
        alertWindow->addButton ("Rename", 1);
        alertWindow->addButton ("Cancel", 0);

        alertWindow->enterModalState (true, juce::ModalCallbackFunction::create (
            [this, alertWindow, currentIdx] (int result)
            {
                if (result == 1)
                {
                    auto newName = alertWindow->getTextEditorContents ("name");
                    if (newName.isNotEmpty())
                        presetManager.renameUserPreset (currentIdx, newName);
                }
                delete alertWindow;
            }), true);
    }

    void showDeleteConfirmation()
    {
        auto currentIdx = presetManager.getCurrentPresetIndex();
        auto& info = presetManager.getPresetInfo (currentIdx);

        auto result = juce::AlertWindow::showOkCancelBox (
            juce::MessageBoxIconType::WarningIcon,
            "Delete Preset",
            "Delete \"" + info.name + "\"? This cannot be undone.",
            "Delete", "Cancel");

        if (result)
            presetManager.deleteUserPreset (currentIdx);
    }

    void showImportDialog()
    {
        fileChooser = std::make_unique<juce::FileChooser> (
            "Import Preset", juce::File(), "*.cpreset");

        fileChooser->launchAsync (juce::FileBrowserComponent::openMode,
            [this] (const juce::FileChooser& fc)
            {
                auto file = fc.getResult();
                if (file.existsAsFile())
                    presetManager.importPreset (file);
            });
    }

    void showExportDialog()
    {
        auto& info = presetManager.getCurrentPresetInfo();
        auto defaultFile = juce::File::getSpecialLocation (juce::File::userDesktopDirectory)
                               .getChildFile (info.name + ".cpreset");

        fileChooser = std::make_unique<juce::FileChooser> (
            "Export Preset", defaultFile, "*.cpreset");

        fileChooser->launchAsync (juce::FileBrowserComponent::saveMode,
            [this] (const juce::FileChooser& fc)
            {
                auto file = fc.getResult();
                if (file != juce::File())
                    presetManager.exportPreset (presetManager.getCurrentPresetIndex(), file);
            });
    }

    //==========================================================================
    void styleNavButton (juce::TextButton& btn)
    {
        btn.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff152030));
        btn.setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xff1e293b));
        btn.setColour (juce::TextButton::textColourOffId, juce::Colour (0xff94a3b8));
        btn.setColour (juce::TextButton::textColourOnId, juce::Colour (0xffe2e8f0));
    }

    void styleNameButton()
    {
        nameButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff0a1018));
        nameButton.setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xff152030));
        nameButton.setColour (juce::TextButton::textColourOffId, juce::Colour (0xffe2e8f0));
        nameButton.setColour (juce::TextButton::textColourOnId, juce::Colour (0xffffffff));
    }

    void styleActionButton (juce::TextButton& btn)
    {
        btn.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff152030));
        btn.setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xff1e293b));
        btn.setColour (juce::TextButton::textColourOffId, juce::Colour (0xff8b9caa));
        btn.setColour (juce::TextButton::textColourOnId, juce::Colour (0xffe2e8f0));
    }

    //==========================================================================
    // Save As dialog window
    //==========================================================================
    class SavePresetWindow : public juce::DialogWindow
    {
    public:
        SavePresetWindow (PresetManager& pm)
            : DialogWindow ("Save Preset", juce::Colour (0xff0c1520), true),
              presetManager (pm)
        {
            setContentOwned (new Content (pm, *this), true);
            centreWithSize (320, 240);
            setResizable (false, false);
        }

        void closeButtonPressed() override { exitModalState (0); }

    private:
        class Content : public juce::Component
        {
        public:
            Content (PresetManager& pm, SavePresetWindow& w) : presetManager (pm), window (w)
            {
                addAndMakeVisible (nameLabel);
                nameLabel.setText ("Name:", juce::dontSendNotification);
                nameLabel.setColour (juce::Label::textColourId, juce::Colour (0xffe2e8f0));

                addAndMakeVisible (nameEditor);
                nameEditor.setColour (juce::TextEditor::backgroundColourId, juce::Colour (0xff0a1018));
                nameEditor.setColour (juce::TextEditor::textColourId, juce::Colour (0xffe2e8f0));
                nameEditor.setColour (juce::TextEditor::outlineColourId, juce::Colour (0xff1e293b));

                addAndMakeVisible (categoryLabel);
                categoryLabel.setText ("Category:", juce::dontSendNotification);
                categoryLabel.setColour (juce::Label::textColourId, juce::Colour (0xffe2e8f0));

                addAndMakeVisible (categoryBox);
                categoryBox.addItem ("Subtle", 1);
                categoryBox.addItem ("Chorus", 2);
                categoryBox.addItem ("Ensemble", 3);
                categoryBox.addItem ("Creative", 4);
                categoryBox.addItem ("Rhythmic", 5);
                categoryBox.addItem ("User", 6);
                categoryBox.setSelectedId (6);

                addAndMakeVisible (authorLabel);
                authorLabel.setText ("Author:", juce::dontSendNotification);
                authorLabel.setColour (juce::Label::textColourId, juce::Colour (0xffe2e8f0));

                addAndMakeVisible (authorEditor);
                authorEditor.setColour (juce::TextEditor::backgroundColourId, juce::Colour (0xff0a1018));
                authorEditor.setColour (juce::TextEditor::textColourId, juce::Colour (0xffe2e8f0));
                authorEditor.setColour (juce::TextEditor::outlineColourId, juce::Colour (0xff1e293b));

                addAndMakeVisible (descLabel);
                descLabel.setText ("Description:", juce::dontSendNotification);
                descLabel.setColour (juce::Label::textColourId, juce::Colour (0xffe2e8f0));

                addAndMakeVisible (descEditor);
                descEditor.setMultiLine (true);
                descEditor.setColour (juce::TextEditor::backgroundColourId, juce::Colour (0xff0a1018));
                descEditor.setColour (juce::TextEditor::textColourId, juce::Colour (0xffe2e8f0));
                descEditor.setColour (juce::TextEditor::outlineColourId, juce::Colour (0xff1e293b));

                addAndMakeVisible (saveBtn);
                saveBtn.setButtonText ("Save");
                saveBtn.onClick = [this]
                {
                    auto name = nameEditor.getText().trim();
                    if (name.isEmpty()) return;

                    auto catIdx = categoryBox.getSelectedId();
                    auto categories = juce::StringArray { "Subtle", "Chorus", "Ensemble", "Creative", "Rhythmic", "User" };
                    auto category = categories[catIdx - 1];

                    presetManager.saveUserPreset (name, category, authorEditor.getText(), descEditor.getText());
                    window.exitModalState (1);
                };

                addAndMakeVisible (cancelBtn);
                cancelBtn.setButtonText ("Cancel");
                cancelBtn.onClick = [this] { window.exitModalState (0); };

                setSize (300, 220);
            }

            void resized() override
            {
                auto b = getLocalBounds().reduced (12);
                int rowH = 24;
                int gap = 4;
                int labelW = 80;

                auto row = b.removeFromTop (rowH);
                nameLabel.setBounds (row.removeFromLeft (labelW));
                nameEditor.setBounds (row);
                b.removeFromTop (gap);

                row = b.removeFromTop (rowH);
                categoryLabel.setBounds (row.removeFromLeft (labelW));
                categoryBox.setBounds (row);
                b.removeFromTop (gap);

                row = b.removeFromTop (rowH);
                authorLabel.setBounds (row.removeFromLeft (labelW));
                authorEditor.setBounds (row);
                b.removeFromTop (gap);

                row = b.removeFromTop (rowH);
                descLabel.setBounds (row.removeFromLeft (labelW));
                descEditor.setBounds (row.withHeight (40));
                b.removeFromTop (40 + gap);

                auto buttons = b.removeFromBottom (28);
                cancelBtn.setBounds (buttons.removeFromRight (70));
                buttons.removeFromRight (8);
                saveBtn.setBounds (buttons.removeFromRight (70));
            }

            void paint (juce::Graphics& g) override
            {
                g.fillAll (juce::Colour (0xff0c1520));
            }

        private:
            PresetManager& presetManager;
            SavePresetWindow& window;
            juce::Label nameLabel, categoryLabel, authorLabel, descLabel;
            juce::TextEditor nameEditor, authorEditor, descEditor;
            juce::ComboBox categoryBox;
            juce::TextButton saveBtn, cancelBtn;
        };

        PresetManager& presetManager;
    };

    //==========================================================================
    PresetManager& presetManager;
    juce::TextButton prevButton, nextButton, nameButton, saveButton, abButton, undoButton;
    juce::ComboBox categoryBox;
    std::unique_ptr<juce::FileChooser> fileChooser;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetBrowser)
};
