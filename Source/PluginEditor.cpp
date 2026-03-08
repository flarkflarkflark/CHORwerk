#include "PluginEditor.h"

using Colors = CharsiesiLookAndFeel::Colors;

CharsiesiEditor::CharsiesiEditor (CharsiesiProcessor& p)
    : AudioProcessorEditor (p),
      processor (p),
      presetBrowser (p.getPresetManager()),
      //--- Toggles in header ---
      preTgl  ("Pre",  juce::Colour (Colors::voices)),
      postTgl ("Post", juce::Colour (Colors::voices)),
      //--- Waveform ---
      waveformDisplay (p.getChorusEngine()),
      //--- Voices section ---
      voicesSection ("Voices", juce::Colour (Colors::voices)),
      voicesKnob   ("Voices", "",   juce::Colour (Colors::voices)),
      mixKnob      ("Mix",    "%",  juce::Colour (Colors::voices)),
      feedbackKnob ("Fdbk",   "%",  juce::Colour (Colors::feedback)),
      rotationKnob ("Rotat",  "π",  juce::Colour (Colors::rate), true),
      stereoTgl    ("Stereo",       juce::Colour (Colors::voices)),
      followLevelTgl ("Follow Lvl", juce::Colour (Colors::voices)),
      collisionTgl ("Collision",    juce::Colour (Colors::voices)),
      //--- Rate section ---
      rateSection ("Rate", juce::Colour (Colors::rate)),
      minRateKnob    ("Min",    "ms/s", juce::Colour (Colors::rate)),
      rateRangeKnob  ("Range",  "ms/s", juce::Colour (Colors::rate)),
      rateUpdateKnob ("Update", "",     juce::Colour (Colors::rate)),
      quantizeTgl    ("Quantize",       juce::Colour (Colors::rate)),
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
      seqQuantTgl  ("Quant",  juce::Colour (Colors::stepSeq)),
      //--- Envelope section ---
      envSection ("Envelope", juce::Colour (Colors::envelope)),
      envLPKnob    ("→LP",  "oct", juce::Colour (Colors::envelope), true),
      envHPKnob    ("→HP",  "oct", juce::Colour (Colors::envelope), true),
      envDelayKnob ("→Dly", "ms",  juce::Colour (Colors::envelope), true),
      followModeDrop ("Follow", juce::Colour (Colors::envelope)),
      envMeter (p.getChorusEngine()),
      //--- LFO section ---
      lfoSection ("LFO", juce::Colour (Colors::lfo)),
      lfoRateKnob   ("Rate", "",    juce::Colour (Colors::lfo)),
      lfoShapeDrop   ("Shape",      juce::Colour (Colors::lfo)),
      lfoUnitDrop    ("Unit",       juce::Colour (Colors::lfo)),
      lfoQuantTgl    ("Quant",      juce::Colour (Colors::lfo)),
      lfoDisplay (p.getChorusEngine()),
      //--- Mod depths ---
      seqLPKnob    ("→LP",  "oct", juce::Colour (Colors::stepSeq), true),
      seqHPKnob    ("→HP",  "oct", juce::Colour (Colors::stepSeq), true),
      seqDelayKnob ("→Dly", "ms",  juce::Colour (Colors::stepSeq), true),
      lfoLPKnob    ("→LP",  "oct", juce::Colour (Colors::lfo), true),
      lfoHPKnob    ("→HP",  "oct", juce::Colour (Colors::lfo), true),
      lfoDelayKnob ("→Dly", "ms",  juce::Colour (Colors::lfo), true),
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
    addAndMakeVisible (preTgl);   // pregain visual cue — not a real param toggle
    addAndMakeVisible (postTgl);

    // Waveform
    addAndMakeVisible (waveformDisplay);

    // Voices section
    addAndMakeVisible (voicesSection);
    addAndMakeVisible (voicesKnob);    voicesKnob.attachToAPVTS (apvts, Param::Voices);
    addAndMakeVisible (mixKnob);       mixKnob.attachToAPVTS (apvts, Param::Mix);
    addAndMakeVisible (feedbackKnob);  feedbackKnob.attachToAPVTS (apvts, Param::Feedback);
    addAndMakeVisible (rotationKnob);  rotationKnob.attachToAPVTS (apvts, Param::Rotation);
    addAndMakeVisible (stereoTgl);
    addAndMakeVisible (followLevelTgl);
    addAndMakeVisible (collisionTgl);  collisionTgl.attachToAPVTS (apvts, Param::UpdateOnCollision);

    // Stereo mode dropdown (inside voices section)
    addAndMakeVisible (stereoModeDrop);
    stereoModeDrop.getBox().addItem ("Free", 1);
    stereoModeDrop.getBox().addItem ("Slave", 2);
    stereoModeDrop.getBox().addItem ("Anti-Slave", 3);
    stereoModeDrop.getBox().addItem ("Half", 4);
    stereoModeDrop.attachToAPVTS (apvts, Param::StereoMode);

    // Follow level dropdown
    addAndMakeVisible (followModeDrop);
    followModeDrop.getBox().addItem ("Off", 1);
    followModeDrop.getBox().addItem ("Slow", 2);
    followModeDrop.getBox().addItem ("Normal", 3);
    followModeDrop.getBox().addItem ("Fast", 4);
    followModeDrop.attachToAPVTS (apvts, Param::FollowLevel);

    // Rate section
    addAndMakeVisible (rateSection);
    addAndMakeVisible (minRateKnob);    minRateKnob.attachToAPVTS (apvts, Param::MinRate);
    addAndMakeVisible (rateRangeKnob);  rateRangeKnob.attachToAPVTS (apvts, Param::RateRange);
    addAndMakeVisible (rateUpdateKnob); rateUpdateKnob.attachToAPVTS (apvts, Param::RateUpdate);
    addAndMakeVisible (quantizeTgl);    quantizeTgl.attachToAPVTS (apvts, Param::UpdateQuantize);

    addAndMakeVisible (updateUnitDrop);
    {
        auto& box = updateUnitDrop.getBox();
        box.addItem ("ms", 1); box.addItem ("10 ms", 2); box.addItem ("sec", 3);
        box.addItem ("32nds", 4); box.addItem ("16ths", 5);
        box.addItem ("8ths", 6); box.addItem ("Quarters", 7);
    }
    updateUnitDrop.attachToAPVTS (apvts, Param::UpdateUnit);
    addAndMakeVisible (updateQuantTgl);
    updateQuantTgl.attachToAPVTS (apvts, Param::UpdateQuantize);

    // Delay section
    addAndMakeVisible (delaySection);
    addAndMakeVisible (minDelayKnob);    minDelayKnob.attachToAPVTS (apvts, Param::MinDelay);
    addAndMakeVisible (delayRangeKnob);  delayRangeKnob.attachToAPVTS (apvts, Param::DelayRange);

    // Filter section
    addAndMakeVisible (filterSection);
    addAndMakeVisible (lowpassKnob);   lowpassKnob.attachToAPVTS (apvts, Param::Lowpass);
    addAndMakeVisible (lpResKnob);     lpResKnob.attachToAPVTS (apvts, Param::LPRes);
    addAndMakeVisible (highpassKnob);  highpassKnob.attachToAPVTS (apvts, Param::Highpass);
    addAndMakeVisible (hpResKnob);     hpResKnob.attachToAPVTS (apvts, Param::HPRes);

    // Step Sequencer section
    addAndMakeVisible (seqSection);
    addAndMakeVisible (stepSeqDisplay);
    stepSeqDisplay.attachToAPVTS (apvts);
    addAndMakeVisible (seqStepKnob);  seqStepKnob.attachToAPVTS (apvts, Param::SeqStep);
    addAndMakeVisible (seqQuantTgl);  seqQuantTgl.attachToAPVTS (apvts, Param::SeqQuant);
    addAndMakeVisible (seqUnitDrop);
    {
        auto& box = seqUnitDrop.getBox();
        box.addItem ("ms", 1); box.addItem ("10 ms", 2); box.addItem ("sec", 3);
        box.addItem ("32nds", 4); box.addItem ("16ths", 5);
        box.addItem ("8ths", 6); box.addItem ("Quarters", 7);
    }
    seqUnitDrop.attachToAPVTS (apvts, Param::SeqUnit);

    // Seq mod depths
    addAndMakeVisible (seqLPKnob);     seqLPKnob.attachToAPVTS (apvts, Param::SeqLP);
    addAndMakeVisible (seqHPKnob);     seqHPKnob.attachToAPVTS (apvts, Param::SeqHP);
    addAndMakeVisible (seqDelayKnob);  seqDelayKnob.attachToAPVTS (apvts, Param::SeqDelay);

    // Envelope section
    addAndMakeVisible (envSection);
    addAndMakeVisible (envLPKnob);     envLPKnob.attachToAPVTS (apvts, Param::EnvLP);
    addAndMakeVisible (envHPKnob);     envHPKnob.attachToAPVTS (apvts, Param::EnvHP);
    addAndMakeVisible (envDelayKnob);  envDelayKnob.attachToAPVTS (apvts, Param::EnvDelay);
    addAndMakeVisible (envMeter);

    // LFO section
    addAndMakeVisible (lfoSection);
    addAndMakeVisible (lfoRateKnob);   lfoRateKnob.attachToAPVTS (apvts, Param::LFORate);
    addAndMakeVisible (lfoDisplay);
    addAndMakeVisible (lfoQuantTgl);   lfoQuantTgl.attachToAPVTS (apvts, Param::LFOQuant);

    addAndMakeVisible (lfoShapeDrop);
    {
        auto& box = lfoShapeDrop.getBox();
        box.addItem ("Sine", 1); box.addItem ("Ramp", 2); box.addItem ("Triangle", 3);
        box.addItem ("Square", 4); box.addItem ("Step Rnd", 5);
        box.addItem ("Smooth Rnd", 6); box.addItem ("User", 7);
    }
    lfoShapeDrop.attachToAPVTS (apvts, Param::LFOShape);

    addAndMakeVisible (lfoUnitDrop);
    {
        auto& box = lfoUnitDrop.getBox();
        box.addItem ("ms", 1); box.addItem ("10 ms", 2); box.addItem ("sec", 3);
        box.addItem ("32nds", 4); box.addItem ("16ths", 5);
        box.addItem ("8ths", 6); box.addItem ("Quarters", 7);
    }
    lfoUnitDrop.attachToAPVTS (apvts, Param::LFOUnit);

    // LFO mod depths
    addAndMakeVisible (lfoLPKnob);     lfoLPKnob.attachToAPVTS (apvts, Param::LFOLP);
    addAndMakeVisible (lfoHPKnob);     lfoHPKnob.attachToAPVTS (apvts, Param::LFOHP);
    addAndMakeVisible (lfoDelayKnob);  lfoDelayKnob.attachToAPVTS (apvts, Param::LFODelay);

    // Gain controls
    addAndMakeVisible (pregainKnob);   pregainKnob.attachToAPVTS (apvts, Param::Pregain);
    addAndMakeVisible (postgainKnob);  postgainKnob.attachToAPVTS (apvts, Param::Postgain);

    //==========================================================================
    // Window setup
    //==========================================================================
    setResizable (true, true);
    setResizeLimits (baseWidth / 2, baseHeight / 2, baseWidth * 3, baseHeight * 3);
    getConstrainer()->setFixedAspectRatio (
        static_cast<double> (baseWidth) / static_cast<double> (baseHeight));
    setSize (baseWidth, baseHeight);

    startTimerHz (15);
}

CharsiesiEditor::~CharsiesiEditor()
{
    setLookAndFeel (nullptr);
    stopTimer();
}

//==============================================================================
void CharsiesiEditor::paint (juce::Graphics& g)
{
    // Background gradient
    auto bounds = getLocalBounds().toFloat();
    g.setGradientFill (juce::ColourGradient (
        juce::Colour (0xff0d1825), 0, 0,
        juce::Colour (0xff0a1420), 0, bounds.getHeight(), false));
    g.fillAll();

    // Header bar
    auto header = bounds.removeFromTop (44.0f);
    g.setColour (juce::Colour (0xff0a1420));
    g.fillRect (header);
    g.setColour (juce::Colour (Colors::border));
    g.drawLine (0, header.getBottom(), bounds.getWidth(), header.getBottom(), 1.0f);

    // Title
    g.setColour (juce::Colour (Colors::textPrimary));
    g.setFont (juce::Font ("JetBrains Mono", 18.0f, juce::Font::bold));
    g.drawText ("CHARSIESIS", header.reduced (14, 0), juce::Justification::centredLeft);

    // Version tag
    g.setColour (juce::Colour (Colors::textDim));
    g.setFont (juce::Font ("JetBrains Mono", 9.0f, juce::Font::plain));
    g.drawText ("v2", header.reduced (130, 0), juce::Justification::centredLeft);

    // Footer
    auto footer = getLocalBounds().toFloat().removeFromBottom (24.0f);
    g.setColour (juce::Colour (0xff0a1018));
    g.fillRect (footer);
    g.setColour (juce::Colour (Colors::border));
    g.drawLine (0, footer.getY(), bounds.getWidth(), footer.getY(), 1.0f);

    g.setColour (juce::Colour (Colors::textDim));
    g.setFont (juce::Font ("JetBrains Mono", 9.0f, juce::Font::plain));
    g.drawText ("flarkAUDIO", footer.reduced (14, 0), juce::Justification::centredLeft);
    g.drawText ("VST3 / CLAP / AU", footer.reduced (14, 0), juce::Justification::centredRight);
}

//==============================================================================
void CharsiesiEditor::resized()
{
    auto b = getLocalBounds();
    float scale = static_cast<float> (b.getWidth()) / static_cast<float> (baseWidth);

    int pad = static_cast<int> (8 * scale);
    int knobW = static_cast<int> (56 * scale);
    int knobH = static_cast<int> (72 * scale);
    int tglH = static_cast<int> (22 * scale);
    int dropH = static_cast<int> (36 * scale);

    //==========================================================================
    // Header
    //==========================================================================
    auto header = b.removeFromTop (static_cast<int> (44 * scale));
    auto presetArea = header.withTrimmedLeft (static_cast<int> (150 * scale)).reduced (pad);

    // Pre/Post toggles at far right of header
    auto rightHeader = presetArea.removeFromRight (static_cast<int> (120 * scale));
    postTgl.setBounds (rightHeader.removeFromRight (static_cast<int> (55 * scale)));
    rightHeader.removeFromRight (4);
    preTgl.setBounds (rightHeader.removeFromRight (static_cast<int> (55 * scale)));

    presetBrowser.setBounds (presetArea);

    //==========================================================================
    // Footer
    //==========================================================================
    auto footer = b.removeFromBottom (static_cast<int> (24 * scale));

    //==========================================================================
    // Content area
    //==========================================================================
    auto content = b.reduced (pad);

    // TOP ROW: Waveform + Voices (~38% height)
    int topH = static_cast<int> (content.getHeight() * 0.35f);
    auto topRow = content.removeFromTop (topH);
    content.removeFromTop (pad);

    // Waveform (left 55%)
    int waveW = static_cast<int> (topRow.getWidth() * 0.55f);
    waveformDisplay.setBounds (topRow.removeFromLeft (waveW).reduced (2));
    topRow.removeFromLeft (pad);

    // Voices section (right 45%)
    voicesSection.setBounds (topRow);
    {
        auto vs = voicesSection.getContentBounds();

        // Top: 4 knobs
        auto knobRow = vs.removeFromTop (knobH);
        voicesKnob.setBounds   (knobRow.removeFromLeft (knobW));
        mixKnob.setBounds      (knobRow.removeFromLeft (knobW));
        feedbackKnob.setBounds (knobRow.removeFromLeft (knobW));
        rotationKnob.setBounds (knobRow.removeFromLeft (knobW));

        vs.removeFromTop (2);

        // Bottom: toggles + stereo mode dropdown
        auto ctrlRow = vs.removeFromTop (tglH);
        stereoModeDrop.setBounds (ctrlRow.removeFromLeft (static_cast<int> (80 * scale)).withHeight (dropH));
        ctrlRow.removeFromLeft (4);
        collisionTgl.setBounds (ctrlRow.removeFromLeft (static_cast<int> (75 * scale)));

        // Gain knobs tucked under
        auto gainRow = vs.removeFromTop (knobH);
        pregainKnob.setBounds  (gainRow.removeFromLeft (knobW));
        postgainKnob.setBounds (gainRow.removeFromLeft (knobW));
    }

    //==========================================================================
    // MIDDLE ROW: Rate, Delay, Filter (~30% height)
    //==========================================================================
    int midH = static_cast<int> (content.getHeight() * 0.42f);
    auto midRow = content.removeFromTop (midH);
    content.removeFromTop (pad);

    int thirdW = midRow.getWidth() / 3;

    // Rate section
    auto rateArea = midRow.removeFromLeft (thirdW).reduced (2);
    rateSection.setBounds (rateArea);
    {
        auto rs = rateSection.getContentBounds();
        auto knobRow = rs.removeFromTop (knobH);
        minRateKnob.setBounds   (knobRow.removeFromLeft (knobW));
        rateRangeKnob.setBounds (knobRow.removeFromLeft (knobW));
        rateUpdateKnob.setBounds (knobRow.removeFromLeft (knobW));

        rs.removeFromTop (2);
        auto ctrlRow = rs.removeFromTop (tglH);
        quantizeTgl.setBounds (ctrlRow.removeFromLeft (static_cast<int> (70 * scale)));
        ctrlRow.removeFromLeft (4);
        updateUnitDrop.setBounds (ctrlRow.removeFromLeft (static_cast<int> (80 * scale)).withHeight (dropH));
    }

    // Delay section
    auto delayArea = midRow.removeFromLeft (static_cast<int> (thirdW * 0.65f)).reduced (2);
    delaySection.setBounds (delayArea);
    {
        auto ds = delaySection.getContentBounds();
        auto knobRow = ds.removeFromTop (knobH);
        minDelayKnob.setBounds   (knobRow.removeFromLeft (knobW));
        delayRangeKnob.setBounds (knobRow.removeFromLeft (knobW));
    }

    // Filter section
    auto filterArea = midRow.reduced (2);
    filterSection.setBounds (filterArea);
    {
        auto fs = filterSection.getContentBounds();
        auto knobRow = fs.removeFromTop (knobH);
        lowpassKnob.setBounds  (knobRow.removeFromLeft (knobW));
        lpResKnob.setBounds    (knobRow.removeFromLeft (knobW));
        highpassKnob.setBounds (knobRow.removeFromLeft (knobW));
        hpResKnob.setBounds    (knobRow.removeFromLeft (knobW));
    }

    //==========================================================================
    // BOTTOM ROW: Step Seq, Envelope, LFO (remaining)
    //==========================================================================
    auto botRow = content;
    int botThirdW = botRow.getWidth() / 3;

    // Step Sequencer section (~38%)
    auto seqArea = botRow.removeFromLeft (static_cast<int> (botThirdW * 1.15f)).reduced (2);
    seqSection.setBounds (seqArea);
    {
        auto ss = seqSection.getContentBounds();

        // Seq bars (main area)
        int seqBarH = static_cast<int> (ss.getHeight() * 0.50f);
        stepSeqDisplay.setBounds (ss.removeFromTop (seqBarH));
        ss.removeFromTop (2);

        // Mod depth knobs
        auto modRow = ss.removeFromTop (knobH);
        seqLPKnob.setBounds    (modRow.removeFromLeft (knobW));
        seqHPKnob.setBounds    (modRow.removeFromLeft (knobW));
        seqDelayKnob.setBounds (modRow.removeFromLeft (knobW));

        // Controls
        auto ctrlRow = ss.removeFromTop (tglH);
        seqStepKnob.setBounds (ctrlRow.removeFromLeft (knobW).withHeight (knobH));
        seqUnitDrop.setBounds (ctrlRow.removeFromLeft (static_cast<int> (70 * scale)).withHeight (dropH));
        ctrlRow.removeFromLeft (4);
        seqQuantTgl.setBounds (ctrlRow.removeFromLeft (static_cast<int> (55 * scale)));
    }

    // Envelope section (~25%)
    auto envArea = botRow.removeFromLeft (static_cast<int> (botThirdW * 0.8f)).reduced (2);
    envSection.setBounds (envArea);
    {
        auto es = envSection.getContentBounds();

        // Follow mode dropdown
        auto topCtrl = es.removeFromTop (dropH);
        followModeDrop.setBounds (topCtrl.removeFromLeft (static_cast<int> (80 * scale)));
        topCtrl.removeFromLeft (4);
        envMeter.setBounds (topCtrl.removeFromLeft (static_cast<int> (12 * scale)).withHeight (knobH));

        es.removeFromTop (2);

        // Env mod depth knobs
        auto modRow = es.removeFromTop (knobH);
        envLPKnob.setBounds    (modRow.removeFromLeft (knobW));
        envHPKnob.setBounds    (modRow.removeFromLeft (knobW));
        envDelayKnob.setBounds (modRow.removeFromLeft (knobW));
    }

    // LFO section (remaining)
    auto lfoArea = botRow.reduced (2);
    lfoSection.setBounds (lfoArea);
    {
        auto ls = lfoSection.getContentBounds();

        // LFO display + shape/rate
        auto topPart = ls.removeFromTop (static_cast<int> (ls.getHeight() * 0.45f));
        auto lfoKnobs = topPart.removeFromLeft (knobW + 4);
        lfoRateKnob.setBounds (lfoKnobs.removeFromTop (knobH));

        lfoDisplay.setBounds (topPart.reduced (2));

        ls.removeFromTop (2);

        // Mod depth knobs
        auto modRow = ls.removeFromTop (knobH);
        lfoLPKnob.setBounds    (modRow.removeFromLeft (knobW));
        lfoHPKnob.setBounds    (modRow.removeFromLeft (knobW));
        lfoDelayKnob.setBounds (modRow.removeFromLeft (knobW));

        ls.removeFromTop (2);

        // Dropdowns + toggle
        auto ctrlRow = ls;
        lfoShapeDrop.setBounds (ctrlRow.removeFromLeft (static_cast<int> (80 * scale)).withHeight (dropH));
        ctrlRow.removeFromLeft (4);
        lfoUnitDrop.setBounds (ctrlRow.removeFromLeft (static_cast<int> (65 * scale)).withHeight (dropH));
        ctrlRow.removeFromLeft (4);
        lfoQuantTgl.setBounds (ctrlRow.removeFromLeft (static_cast<int> (55 * scale)).withHeight (tglH));
    }
}

//==============================================================================
void CharsiesiEditor::timerCallback()
{
    // Update LFO display shape
    auto shapeVal = processor.apvts.getRawParameterValue (Param::LFOShape);
    if (shapeVal != nullptr)
        lfoDisplay.setShape (static_cast<int> (*shapeVal));

    // Dirty flag tracking for preset manager
    // (In a full implementation, parameter changes would set dirty via listener)
}
