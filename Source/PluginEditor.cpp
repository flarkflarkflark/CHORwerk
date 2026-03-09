#include "PluginEditor.h"

using Colors = CHORwerkLookAndFeel::Colors;

CHORwerkEditor::CHORwerkEditor (CHORwerkProcessor& p)
    : AudioProcessorEditor (p),
      processor (p),
      presetBrowser (p.getPresetManager()),
      //--- Toggles in header ---
      preTgl  ("Pre",  juce::Colour (Colors::voices), "", false),
      postTgl ("Post", juce::Colour (Colors::voices), "", false),
      //--- Waveform ---
      waveformDisplay (p.getChorusEngine()),
      //--- Voices section ---
      voicesSection ("Voices", juce::Colour (Colors::voices)),
      voicesKnob   ("Voices", "",   juce::Colour (Colors::voices)),
      mixKnob      ("Mix",    "%",  juce::Colour (Colors::voices)),
      feedbackKnob ("Fdbk",   "%",  juce::Colour (Colors::feedback)),
      rotationKnob ("Rotat",  "pi",  juce::Colour (Colors::rate), true),
      stereoTgl    ("Stereo",       juce::Colour (Colors::voices)),
      followLevelTgl ("Follow Lvl", juce::Colour (Colors::voices)),
      collisionTgl ("Collision",    juce::Colour (Colors::voices), "ON"),
      //--- Rate section ---
      rateSection ("Rate", juce::Colour (Colors::rate)),
      minRateKnob    ("Min",    "ms/s", juce::Colour (Colors::rate)),
      rateRangeKnob  ("Range",  "ms/s", juce::Colour (Colors::rate)),
      rateUpdateKnob ("Update", "",     juce::Colour (Colors::rate)),
      quantizeTgl    ("Quantize",       juce::Colour (Colors::rate), "SYNC"),
      //--- Delay section ---
      delaySection ("Delay", juce::Colour (Colors::delay)),
      minDelayKnob   ("Min",   "ms", juce::Colour (Colors::delay)),
      delayRangeKnob ("Range", "ms", juce::Colour (Colors::delay)),
      //--- Filter section ---
      filterSection ("Filter", juce::Colour (Colors::filter)),
      lowpassKnob  ("LP Freq", "%", juce::Colour (Colors::filter)),
      lpResKnob    ("LP Res",  "%", juce::Colour (Colors::filter)),
      highpassKnob ("HP Freq", "%", juce::Colour (Colors::feedback)),
      hpResKnob    ("HP Res",  "%", juce::Colour (Colors::feedback)),
      //--- Step Seq section ---
      seqSection ("Step Seq", juce::Colour (Colors::stepSeq)),
      stepSeqDisplay (p.getChorusEngine()),
      seqStepKnob  ("Step",  "",   juce::Colour (Colors::stepSeq)),
      seqUnitDrop  ("Unit",   juce::Colour (Colors::stepSeq)),
      seqQuantTgl  ("Quant",  juce::Colour (Colors::stepSeq), "SYNC"),
      //--- Envelope section ---
      envSection ("Envelope", juce::Colour (Colors::envelope)),
      envLPKnob    (">LP",  "oct", juce::Colour (Colors::envelope), true),
      envHPKnob    (">HP",  "oct", juce::Colour (Colors::envelope), true),
      envDelayKnob (">Dly", "ms",  juce::Colour (Colors::envelope), true),
      followModeDrop ("Follow", juce::Colour (Colors::envelope)),
      envMeter (p.getChorusEngine()),
      //--- LFO section ---
      lfoSection ("LFO", juce::Colour (Colors::lfo)),
      lfoRateKnob   ("Rate", "",    juce::Colour (Colors::lfo)),
      lfoShapeDrop   ("Shape",      juce::Colour (Colors::lfo)),
      lfoUnitDrop    ("Unit",       juce::Colour (Colors::lfo)),
      lfoQuantTgl    ("Quant",      juce::Colour (Colors::lfo), "SYNC"),
      lfoDisplay (p.getChorusEngine()),
      //--- Mod depths ---
      seqLPKnob    (">LP",  "oct", juce::Colour (Colors::stepSeq), true),
      seqHPKnob    (">HP",  "oct", juce::Colour (Colors::stepSeq), true),
      seqDelayKnob (">Dly", "ms",  juce::Colour (Colors::stepSeq), true),
      lfoLPKnob    (">LP",  "oct", juce::Colour (Colors::lfo), true),
      lfoHPKnob    (">HP",  "oct", juce::Colour (Colors::lfo), true),
      lfoDelayKnob (">Dly", "ms",  juce::Colour (Colors::lfo), true),
      //--- Other ---
      stereoModeDrop ("Stereo",  juce::Colour (Colors::voices)),
      updateUnitDrop ("Upd Unit", juce::Colour (Colors::rate)),
      updateQuantTgl ("Upd Quant", juce::Colour (Colors::rate)),
      pregainKnob  ("Pre",   "dB", juce::Colour (Colors::textSecondary), true),
      postgainKnob ("Post",  "dB", juce::Colour (Colors::textSecondary), true)
{
    setLookAndFeel (&lookAndFeel);

    auto& apvts = processor.apvts;

    //==========================================================================
    // Attach all parameters
    //==========================================================================

    // Header
    addAndMakeVisible (presetBrowser);
    presetBrowser.onScaleChanged = [this](float s) { setScale(s); };

    addAndMakeVisible (preTgl);   
    preTgl.attachToAPVTS (apvts, Param::PrePhase);
    preTgl.setTooltip ("Toggle input phase inversion (Ø)");
    addAndMakeVisible (postTgl);
    postTgl.attachToAPVTS (apvts, Param::PostPhase);
    postTgl.setTooltip ("Toggle output phase inversion (Ø)");

    // Waveform
    addAndMakeVisible (waveformDisplay);

    // Voices section
    addAndMakeVisible (voicesSection);
    addAndMakeVisible (voicesKnob);    voicesKnob.attachToAPVTS (apvts, Param::Voices);
    voicesKnob.setTooltip ("Number of chorus voices");
    addAndMakeVisible (mixKnob);       mixKnob.attachToAPVTS (apvts, Param::Mix);
    mixKnob.setTooltip ("Dry/Wet mix");
    addAndMakeVisible (feedbackKnob);  feedbackKnob.attachToAPVTS (apvts, Param::Feedback);
    feedbackKnob.setTooltip ("Feedback amount");
    addAndMakeVisible (rotationKnob);  rotationKnob.attachToAPVTS (apvts, Param::Rotation);
    rotationKnob.setTooltip ("Phase rotation for cross-channel feedback");
    addAndMakeVisible (stereoTgl);
    addAndMakeVisible (followLevelTgl);
    addAndMakeVisible (collisionTgl);  collisionTgl.attachToAPVTS (apvts, Param::UpdateOnCollision);
    collisionTgl.setTooltip ("Randomize voice when delay times collide");

    // Stereo mode dropdown (inside voices section)
    addAndMakeVisible (stereoModeDrop);
    stereoModeDrop.getBox().addItem ("Free", 1);
    stereoModeDrop.getBox().addItem ("Slave", 2);
    stereoModeDrop.getBox().addItem ("Anti-Slave", 3);
    stereoModeDrop.getBox().addItem ("Half", 4);
    stereoModeDrop.attachToAPVTS (apvts, Param::StereoMode);
    stereoModeDrop.setTooltip ("Stereo spread and voice correlation mode");

    // Follow level dropdown
    addAndMakeVisible (followModeDrop);
    followModeDrop.getBox().addItem ("Off", 1);
    followModeDrop.getBox().addItem ("Slow", 2);
    followModeDrop.getBox().addItem ("Normal", 3);
    followModeDrop.getBox().addItem ("Fast", 4);
    followModeDrop.attachToAPVTS (apvts, Param::FollowLevel);
    followModeDrop.setTooltip ("Envelope follower speed");

    // Rate section
    addAndMakeVisible (rateSection);
    addAndMakeVisible (minRateKnob);    minRateKnob.attachToAPVTS (apvts, Param::MinRate);
    minRateKnob.setTooltip ("Minimum modulation rate");
    addAndMakeVisible (rateRangeKnob);  rateRangeKnob.attachToAPVTS (apvts, Param::RateRange);
    rateRangeKnob.setTooltip ("Range for randomized modulation rates");
    addAndMakeVisible (rateUpdateKnob); rateUpdateKnob.attachToAPVTS (apvts, Param::RateUpdate);
    rateUpdateKnob.setTooltip ("Rate update frequency");
    addAndMakeVisible (quantizeTgl);    quantizeTgl.attachToAPVTS (apvts, Param::UpdateQuantize);
    quantizeTgl.setTooltip ("Quantize update frequency to tempo");

    addAndMakeVisible (updateUnitDrop);
    {
        auto& box = updateUnitDrop.getBox();
        box.addItem ("ms", 1); box.addItem ("10 ms", 2); box.addItem ("sec", 3);
        box.addItem ("32nds", 4); box.addItem ("16ths", 5);
        box.addItem ("8ths", 6); box.addItem ("Quarters", 7);
    }
    updateUnitDrop.attachToAPVTS (apvts, Param::UpdateUnit);
    updateUnitDrop.setTooltip ("Time unit for rate updates");
    addAndMakeVisible (updateQuantTgl);
    updateQuantTgl.attachToAPVTS (apvts, Param::UpdateQuantize);

    // Delay section
    addAndMakeVisible (delaySection);
    addAndMakeVisible (minDelayKnob);    minDelayKnob.attachToAPVTS (apvts, Param::MinDelay);
    minDelayKnob.setTooltip ("Minimum delay time");
    addAndMakeVisible (delayRangeKnob);  delayRangeKnob.attachToAPVTS (apvts, Param::DelayRange);
    delayRangeKnob.setTooltip ("Randomized delay range");

    // Filter section
    addAndMakeVisible (filterSection);
    addAndMakeVisible (lowpassKnob);   lowpassKnob.attachToAPVTS (apvts, Param::Lowpass);
    lowpassKnob.setTooltip ("Low-pass filter cutoff");
    addAndMakeVisible (lpResKnob);     lpResKnob.attachToAPVTS (apvts, Param::LPRes);
    lpResKnob.setTooltip ("Low-pass filter resonance");
    addAndMakeVisible (highpassKnob);  highpassKnob.attachToAPVTS (apvts, Param::Highpass);
    highpassKnob.setTooltip ("High-pass filter cutoff");
    addAndMakeVisible (hpResKnob);     hpResKnob.attachToAPVTS (apvts, Param::HPRes);
    hpResKnob.setTooltip ("High-pass filter resonance");

    // Step Sequencer section
    addAndMakeVisible (seqSection);
    addAndMakeVisible (stepSeqDisplay);
    stepSeqDisplay.attachToAPVTS (apvts);
    addAndMakeVisible (seqStepKnob);  seqStepKnob.attachToAPVTS (apvts, Param::SeqStep);
    seqStepKnob.setTooltip ("Sequencer step rate");
    addAndMakeVisible (seqQuantTgl);  seqQuantTgl.attachToAPVTS (apvts, Param::SeqQuant);
    seqQuantTgl.setTooltip ("Quantize sequencer steps to tempo");
    addAndMakeVisible (seqUnitDrop);
    {
        auto& box = seqUnitDrop.getBox();
        box.addItem ("ms", 1); box.addItem ("10 ms", 2); box.addItem ("sec", 3);
        box.addItem ("32nds", 4); box.addItem ("16ths", 5);
        box.addItem ("8ths", 6); box.addItem ("Quarters", 7);
    }
    seqUnitDrop.attachToAPVTS (apvts, Param::SeqUnit);
    seqUnitDrop.setTooltip ("Time unit for sequencer steps");

    // Seq mod depths
    addAndMakeVisible (seqLPKnob);     seqLPKnob.attachToAPVTS (apvts, Param::SeqLP);
    seqLPKnob.setTooltip ("Sequencer to Low-pass depth");
    addAndMakeVisible (seqHPKnob);     seqHPKnob.attachToAPVTS (apvts, Param::SeqHP);
    seqHPKnob.setTooltip ("Sequencer to High-pass depth");
    addAndMakeVisible (seqDelayKnob);  seqDelayKnob.attachToAPVTS (apvts, Param::SeqDelay);
    seqDelayKnob.setTooltip ("Sequencer to Delay depth");

    // Envelope section
    addAndMakeVisible (envSection);
    addAndMakeVisible (envLPKnob);     envLPKnob.attachToAPVTS (apvts, Param::EnvLP);
    envLPKnob.setTooltip ("Envelope to Low-pass depth");
    addAndMakeVisible (envHPKnob);     envHPKnob.attachToAPVTS (apvts, Param::EnvHP);
    envHPKnob.setTooltip ("Envelope to High-pass depth");
    addAndMakeVisible (envDelayKnob);  envDelayKnob.attachToAPVTS (apvts, Param::EnvDelay);
    envDelayKnob.setTooltip ("Envelope to Delay depth");
    addAndMakeVisible (envMeter);
    envMeter.setTooltip ("Envelope follower level");

    // LFO section
    addAndMakeVisible (lfoSection);
    addAndMakeVisible (lfoRateKnob);   lfoRateKnob.attachToAPVTS (apvts, Param::LFORate);
    lfoRateKnob.setTooltip ("LFO frequency");
    addAndMakeVisible (lfoDisplay);
    lfoDisplay.setTooltip ("LFO waveform preview");
    addAndMakeVisible (lfoQuantTgl);   lfoQuantTgl.attachToAPVTS (apvts, Param::LFOQuant);
    lfoQuantTgl.setTooltip ("Quantize LFO frequency to tempo");

    addAndMakeVisible (lfoShapeDrop);
    {
        auto& box = lfoShapeDrop.getBox();
        box.addItem ("Sine", 1); box.addItem ("Ramp", 2); box.addItem ("Triangle", 3);
        box.addItem ("Square", 4); box.addItem ("Step Rnd", 5);
        box.addItem ("Smooth Rnd", 6); box.addItem ("User", 7);
    }
    lfoShapeDrop.attachToAPVTS (apvts, Param::LFOShape);
    lfoShapeDrop.setTooltip ("LFO waveform shape");

    addAndMakeVisible (lfoUnitDrop);
    {
        auto& box = lfoUnitDrop.getBox();
        box.addItem ("ms", 1); box.addItem ("10 ms", 2); box.addItem ("sec", 3);
        box.addItem ("32nds", 4); box.addItem ("16ths", 5);
        box.addItem ("8ths", 6); box.addItem ("Quarters", 7);
    }
    lfoUnitDrop.attachToAPVTS (apvts, Param::LFOUnit);
    lfoUnitDrop.setTooltip ("Time unit for LFO rate");

    // LFO mod depths
    addAndMakeVisible (lfoLPKnob);     lfoLPKnob.attachToAPVTS (apvts, Param::LFOLP);
    lfoLPKnob.setTooltip ("LFO to Low-pass depth");
    addAndMakeVisible (lfoHPKnob);     lfoHPKnob.attachToAPVTS (apvts, Param::LFOHP);
    lfoHPKnob.setTooltip ("LFO to High-pass depth");
    addAndMakeVisible (lfoDelayKnob);  lfoDelayKnob.attachToAPVTS (apvts, Param::LFODelay);
    lfoDelayKnob.setTooltip ("LFO to Delay depth");

    // Gain controls
    addAndMakeVisible (pregainKnob);   pregainKnob.attachToAPVTS (apvts, Param::Pregain);
    pregainKnob.setTooltip ("Input gain");
    addAndMakeVisible (postgainKnob);  postgainKnob.attachToAPVTS (apvts, Param::Postgain);
    postgainKnob.setTooltip ("Output gain");

    //==========================================================================
    // Dynamic Suffixes for Tempo Sync
    //==========================================================================
    auto getUnitString = [this](const char* paramId) {
        auto* param = dynamic_cast<juce::AudioParameterChoice*> (processor.apvts.getParameter (paramId));
        if (param != nullptr) return param->getCurrentChoiceName();
        return juce::String();
    };

    auto isSyncOn = [this](const char* paramId) {
        return *processor.apvts.getRawParameterValue (paramId) > 0.5f;
    };

    rateUpdateKnob.setSuffixLambda ([this, getUnitString, isSyncOn]() {
        return isSyncOn (Param::UpdateQuantize) ? getUnitString (Param::UpdateUnit) : juce::String();
    });

    lfoRateKnob.setSuffixLambda ([this, getUnitString, isSyncOn]() {
        return isSyncOn (Param::LFOQuant) ? getUnitString (Param::LFOUnit) : juce::String();
    });

    seqStepKnob.setSuffixLambda ([this, getUnitString, isSyncOn]() {
        return isSyncOn (Param::SeqQuant) ? getUnitString (Param::SeqUnit) : juce::String();
    });

    //==========================================================================
    // Window setup
    //==========================================================================
    setResizable (true, true);
    setResizeLimits (baseWidth / 4, baseHeight / 4, baseWidth * 4, baseHeight * 4);
    setSize (baseWidth, baseHeight);

    startTimerHz (15);
}

CHORwerkEditor::~CHORwerkEditor()
{
    setLookAndFeel (nullptr);
    stopTimer();
}

void CHORwerkEditor::setScale (float newScale)
{
    currentScale = newScale;
    setSize (static_cast<int> (baseWidth * currentScale), 
             static_cast<int> (baseHeight * currentScale));
}

//==============================================================================
void CHORwerkEditor::paint (juce::Graphics& g)
{
    // Background gradient
    auto bounds = getLocalBounds().toFloat();
    g.setGradientFill (juce::ColourGradient (
        juce::Colour (0xff0d1825), 0, 0,
        juce::Colour (0xff0a1420), 0, bounds.getHeight(), false));
    g.fillAll();

    // Header bar
    float scaleY = static_cast<float> (getHeight()) / static_cast<float> (baseHeight);
    auto header = bounds.removeFromTop (44.0f * scaleY);
    g.setColour (juce::Colour (0xff0a1420));
    g.fillRect (header);
    g.setColour (juce::Colour (Colors::border));
    g.drawLine (0, header.getBottom(), bounds.getWidth(), header.getBottom(), 1.0f);

    // Title
    g.setColour (juce::Colour (Colors::textPrimary));
    g.setFont (juce::Font ("JetBrains Mono", 18.0f * scaleY, juce::Font::bold));
    g.drawText ("CHORwerk", header.reduced (14 * scaleY, 0), juce::Justification::centredLeft);

    // Version tag
    g.setColour (juce::Colour (Colors::textDim));
    g.setFont (juce::Font ("JetBrains Mono", 9.0f * scaleY, juce::Font::plain));
    g.drawText ("v1.0.0", header.reduced (130 * scaleY, 0), juce::Justification::centredLeft);

    // Footer
    float footerH = 36.0f * scaleY;
    auto footer = getLocalBounds().toFloat().removeFromBottom (footerH);
    g.setColour (juce::Colour (0xff0a1018));
    g.fillRect (footer);
    g.setColour (juce::Colour (Colors::border));
    g.drawLine (0, footer.getY(), bounds.getWidth(), footer.getY(), 1.0f);

    g.setColour (juce::Colour (Colors::textDim));
    g.setFont (juce::Font ("JetBrains Mono", 11.5f * scaleY, juce::Font::plain));
    g.drawText ("flarkAUDIO", footer.reduced (30 * scaleY, 0), juce::Justification::centredLeft);
    g.drawText ("VST3 / CLAP / AU", footer.reduced (30 * scaleY, 0), juce::Justification::centredRight);
}

//==============================================================================
void CHORwerkEditor::resized()
{
    auto b = getLocalBounds();
    
    // Scale is determined by window dimensions relative to base
    float scaleX = static_cast<float> (b.getWidth()) / static_cast<float> (baseWidth);
    float scaleY = static_cast<float> (b.getHeight()) / static_cast<float> (baseHeight);
    float scale = std::min (scaleX, scaleY);

    // Update LookAndFeel global scale for fonts
    lookAndFeel.setScale (scale);

    // Apply scale to all sections for consistent header tabs
    voicesSection.setScale (scale);
    rateSection.setScale (scale);
    delaySection.setScale (scale);
    filterSection.setScale (scale);
    seqSection.setScale (scale);
    envSection.setScale (scale);
    lfoSection.setScale (scale);

    int pad = static_cast<int> (14 * scale); 
    int knobW = static_cast<int> (56 * scale);
    int knobH = static_cast<int> (72 * scale);
    int dropH = static_cast<int> (34 * scale);

    //==========================================================================
    // Header
    //==========================================================================
    auto header = b.removeFromTop (static_cast<int> (52 * scaleY));
    auto headerContent = header.reduced (pad * 2, 0);
    
    // Title + version
    headerContent.removeFromLeft (static_cast<int> (170 * scaleX));
    
    // Pre/Post toggles (Smaller and more compact)
    int phaseBtnW = static_cast<int> (54 * scaleX);
    auto rightHeader = headerContent.removeFromRight (phaseBtnW * 2 + pad);
    postTgl.setBounds (rightHeader.removeFromRight (phaseBtnW).reduced(0, 10));
    rightHeader.removeFromRight (pad);
    preTgl.setBounds (rightHeader.removeFromRight (phaseBtnW).reduced(0, 10));

    // Preset browser - use remaining flexible space instead of hardcoded width
    presetBrowser.setBounds (headerContent.reduced(pad, 6));

    //==========================================================================
    // Footer
    //==========================================================================
    auto footer = b.removeFromBottom (static_cast<int> (36 * scaleY));

    //==========================================================================
    // Content area
    //==========================================================================
    auto content = b.reduced (pad);
    int totalH = content.getHeight();
    int totalW = content.getWidth();

    // TOP ROW: Waveform + Voices (~38% height)
    int topH = static_cast<int> (totalH * 0.38f);
    auto topRow = content.removeFromTop (topH);
    content.removeFromTop (pad);

    // Waveform (left 52%)
    int waveW = static_cast<int> (topRow.getWidth() * 0.52f);
    waveformDisplay.setBounds (topRow.removeFromLeft (waveW).reduced (2));
    topRow.removeFromLeft (pad);

    // Voices section (right 48%)
    voicesSection.setBounds (topRow);
    {
        auto vs = voicesSection.getContentBounds() + voicesSection.getPosition();
        vs = vs.reduced (6, 2);

        // Top: 4 knobs
        auto knobRow = vs.removeFromTop (knobH);
        int itemW = knobRow.getWidth() / 4;
        voicesKnob.setBounds   (knobRow.removeFromLeft (itemW).withSizeKeepingCentre (knobW, knobH));
        mixKnob.setBounds      (knobRow.removeFromLeft (itemW).withSizeKeepingCentre (knobW, knobH));
        feedbackKnob.setBounds (knobRow.removeFromLeft (itemW).withSizeKeepingCentre (knobW, knobH));
        rotationKnob.setBounds (knobRow.removeFromLeft (itemW).withSizeKeepingCentre (knobW, knobH));

        vs.removeFromTop (pad / 2);

        // Middle Row: Gain knobs + Stereo/Collision (Using the empty space!)
        auto midRowControls = vs.removeFromTop (knobH);
        pregainKnob.setBounds  (midRowControls.removeFromLeft (itemW).withSizeKeepingCentre (knobW, knobH));
        postgainKnob.setBounds (midRowControls.removeFromLeft (itemW).withSizeKeepingCentre (knobW, knobH));
        
        // Use the remaining 2 columns for Stereo and Collision
        auto stereoArea = midRowControls.removeFromLeft (itemW).reduced(4, 0);
        stereoModeDrop.setBounds (stereoArea.withSizeKeepingCentre (stereoArea.getWidth(), dropH));
        
        auto collisionArea = midRowControls.removeFromLeft (itemW).reduced(4, 0);
        collisionTgl.setBounds (collisionArea.withSizeKeepingCentre (collisionArea.getWidth(), dropH));
    }

    //==========================================================================
    // MIDDLE ROW: Rate, Delay, Filter (~24% height)
    //==========================================================================
    int midH = static_cast<int> (totalH * 0.25f);
    auto midRow = content.removeFromTop (midH);
    content.removeFromTop (pad);

    // Rate section (35% width)
    auto rateArea = midRow.removeFromLeft (static_cast<int> (totalW * 0.35f)).reduced (2);
    rateSection.setBounds (rateArea);
    {
        auto rs = rateSection.getContentBounds() + rateSection.getPosition();
        auto knobRow = rs.removeFromTop (knobH);
        int kw = knobRow.getWidth() / 3;
        minRateKnob.setBounds   (knobRow.removeFromLeft (kw).withSizeKeepingCentre (knobW, knobH));
        rateRangeKnob.setBounds (knobRow.removeFromLeft (kw).withSizeKeepingCentre (knobW, knobH));
        rateUpdateKnob.setBounds (knobRow.removeFromLeft (kw).withSizeKeepingCentre (knobW, knobH));

        auto ctrlRow = rs.removeFromBottom (dropH);
        quantizeTgl.setBounds (ctrlRow.removeFromLeft (static_cast<int> (100 * scale)));
        ctrlRow.removeFromLeft (pad / 2);
        updateUnitDrop.setBounds (ctrlRow.removeFromLeft (static_cast<int> (85 * scale)));
    }

    // Delay section (20% width)
    auto delayArea = midRow.removeFromLeft (static_cast<int> (totalW * 0.22f)).reduced (2);
    delaySection.setBounds (delayArea);
    {
        auto ds = delaySection.getContentBounds() + delaySection.getPosition();
        auto knobRow = ds.removeFromTop (knobH);
        int kw = knobRow.getWidth() / 2;
        minDelayKnob.setBounds   (knobRow.removeFromLeft (kw).withSizeKeepingCentre (knobW, knobH));
        delayRangeKnob.setBounds (knobRow.removeFromLeft (kw).withSizeKeepingCentre (knobW, knobH));
    }

    // Filter section (remaining)
    auto filterArea = midRow.reduced (2);
    filterSection.setBounds (filterArea);
    {
        auto fs = filterSection.getContentBounds() + filterSection.getPosition();
        auto knobRow = fs.removeFromTop (knobH);
        int kw = knobRow.getWidth() / 4;
        lowpassKnob.setBounds  (knobRow.removeFromLeft (kw).withSizeKeepingCentre (knobW, knobH));
        lpResKnob.setBounds    (knobRow.removeFromLeft (kw).withSizeKeepingCentre (knobW, knobH));
        highpassKnob.setBounds (knobRow.removeFromLeft (kw).withSizeKeepingCentre (knobW, knobH));
        hpResKnob.setBounds    (knobRow.removeFromLeft (kw).withSizeKeepingCentre (knobW, knobH));
    }

    //==========================================================================
    // BOTTOM ROW: Step Seq, Envelope, LFO (remaining ~38%)
    //==========================================================================
    auto botRow = content;
    
    // Step Sequencer (38% width)
    auto seqArea = botRow.removeFromLeft (static_cast<int> (totalW * 0.38f)).reduced (2);
    seqSection.setBounds (seqArea);
    {
        auto ss = seqSection.getContentBounds() + seqSection.getPosition();
        ss = ss.reduced (6, 0);

        auto modRow = ss.removeFromBottom (knobH);
        int kw = modRow.getWidth() / 4;
        seqStepKnob.setBounds  (modRow.removeFromLeft (kw).withSizeKeepingCentre (knobW, knobH));
        seqLPKnob.setBounds    (modRow.removeFromLeft (kw).withSizeKeepingCentre (knobW, knobH));
        seqHPKnob.setBounds    (modRow.removeFromLeft (kw).withSizeKeepingCentre (knobW, knobH));
        seqDelayKnob.setBounds (modRow.removeFromLeft (kw).withSizeKeepingCentre (knobW, knobH));

        auto ctrlRow = ss.removeFromBottom (dropH);
        seqUnitDrop.setBounds (ctrlRow.removeFromLeft (static_cast<int> (85 * scale)));
        ctrlRow.removeFromLeft (pad);
        seqQuantTgl.setBounds (ctrlRow.removeFromLeft (static_cast<int> (95 * scale)));

        stepSeqDisplay.setBounds (ss.reduced (0, 6));
    }

    // Envelope (24% width)
    auto envArea = botRow.removeFromLeft (static_cast<int> (totalW * 0.24f)).reduced (2);
    envSection.setBounds (envArea);
    {
        auto es = envSection.getContentBounds() + envSection.getPosition();
        es = es.reduced (6, 0);

        auto modRow = es.removeFromBottom (knobH);
        int kw = modRow.getWidth() / 3;
        envLPKnob.setBounds    (modRow.removeFromLeft (kw).withSizeKeepingCentre (knobW, knobH));
        envHPKnob.setBounds    (modRow.removeFromLeft (kw).withSizeKeepingCentre (knobW, knobH));
        envDelayKnob.setBounds (modRow.removeFromLeft (kw).withSizeKeepingCentre (knobW, knobH));

        auto ctrlRow = es.removeFromBottom (dropH);
        followModeDrop.setBounds (ctrlRow.reduced(pad, 0));
        
        envMeter.setBounds (es.reduced (pad, 6));
    }

    // LFO (remaining)
    auto lfoArea = botRow.reduced (2);
    lfoSection.setBounds (lfoArea);
    {
        auto ls = lfoSection.getContentBounds() + lfoSection.getPosition();
        ls = ls.reduced (6, 0);

        auto modRow = ls.removeFromBottom (knobH);
        int kw = modRow.getWidth() / 4;
        lfoRateKnob.setBounds  (modRow.removeFromLeft (kw).withSizeKeepingCentre (knobW, knobH));
        lfoLPKnob.setBounds    (modRow.removeFromLeft (kw).withSizeKeepingCentre (knobW, knobH));
        lfoHPKnob.setBounds    (modRow.removeFromLeft (kw).withSizeKeepingCentre (knobW, knobH));
        lfoDelayKnob.setBounds (modRow.removeFromLeft (kw).withSizeKeepingCentre (knobW, knobH));

        auto ctrlRow = ls.removeFromBottom (dropH);
        lfoShapeDrop.setBounds (ctrlRow.removeFromLeft (static_cast<int> (95 * scale)));
        ctrlRow.removeFromLeft (pad / 2);
        lfoUnitDrop.setBounds (ctrlRow.removeFromLeft (static_cast<int> (85 * scale)));
        ctrlRow.removeFromLeft (pad / 2);
        lfoQuantTgl.setBounds (ctrlRow.removeFromLeft (static_cast<int> (85 * scale)));

        lfoDisplay.setBounds (ls.reduced (pad, 6));
    }
}

//==============================================================================
void CHORwerkEditor::timerCallback()
{
    auto shapeVal = processor.apvts.getRawParameterValue (Param::LFOShape);
    if (shapeVal != nullptr)
        lfoDisplay.setShape (static_cast<int> (*shapeVal));
}
