#include <atomic>

#include <juce_audio_utils/juce_audio_utils.h>

#include "../DSP/BloomVerbEngine.h"

namespace
{
class LoopTesterComponent final : public juce::AudioAppComponent,
                                  private juce::Timer
{
public:
    LoopTesterComponent()
    {
        setOpaque(true);

        addAndMakeVisible(loadButton);
        loadButton.setButtonText("Load Audio File");
        loadButton.onClick = [this] { openFileChooser(); };

        addAndMakeVisible(playButton);
        playButton.setButtonText("Play");
        playButton.onClick = [this] { togglePlayback(); };

        addAndMakeVisible(loopToggle);
        loopToggle.setButtonText("Loop");
        loopToggle.setToggleState(true, juce::dontSendNotification);
        loopToggle.onClick = [this]
        {
            shouldLoop.store(loopToggle.getToggleState());
            if (readerSource != nullptr)
                readerSource->setLooping(shouldLoop.load());
        };

        addAndMakeVisible(fileLabel);
        fileLabel.setText("No file loaded", juce::dontSendNotification);
        fileLabel.setColour(juce::Label::textColourId, juce::Colours::white);

        typeLabel.setText("Type", juce::dontSendNotification);
        addAndMakeVisible(typeLabel);
        addAndMakeVisible(typeBox);
        typeBox.addItemList({"Plate", "Hall", "Room", "Cloud", "Bloom", "Grain", "Chamber", "Dream"}, 1);
        typeBox.setSelectedId(2, juce::dontSendNotification);
        typeBox.onChange = [this] { typeChoice.store(typeBox.getSelectedId() - 1); };

        addSlider(mixSlider, mixLabel, "Mix", 0.0, 1.0, 0.25, [this] { mix.store(static_cast<float>(mixSlider.getValue())); });
        addSlider(decaySlider, decayLabel, "Decay", 0.1, 20.0, 2.8, [this] { decay.store(static_cast<float>(decaySlider.getValue())); });
        addSlider(sizeSlider, sizeLabel, "Size", 0.0, 1.0, 0.5, [this] { size.store(static_cast<float>(sizeSlider.getValue())); });
        addSlider(motionSlider, motionLabel, "Motion", 0.0, 1.0, 0.3, [this] { motion.store(static_cast<float>(motionSlider.getValue())); });
        addSlider(harmonicSlider, harmonicLabel, "Harmonic", 0.0, 1.0, 0.2, [this] { harmonic.store(static_cast<float>(harmonicSlider.getValue())); });
        addSlider(warpSlider, warpLabel, "Warp", 0.0, 1.0, 0.3, [this] { warp.store(static_cast<float>(warpSlider.getValue())); });

        formatManager.registerBasicFormats();
        setAudioChannels(0, 2);
        startTimerHz(10);
    }

    ~LoopTesterComponent() override
    {
        transportSource.stop();
        transportSource.setSource(nullptr);
        readerSource.reset();
        shutdownAudio();
    }

    void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override
    {
        engine.prepare(sampleRate, samplesPerBlockExpected, 2);
        transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
    }

    void getNextAudioBlock(const juce::AudioSourceChannelInfo& bufferToFill) override
    {
        bufferToFill.clearActiveBufferRegion();

        if (readerSource == nullptr)
            return;

        juce::AudioSourceChannelInfo tempInfo(&tempBuffer, 0, bufferToFill.numSamples);
        if (tempBuffer.getNumChannels() != bufferToFill.buffer->getNumChannels()
            || tempBuffer.getNumSamples() < bufferToFill.numSamples)
        {
            tempBuffer.setSize(bufferToFill.buffer->getNumChannels(), bufferToFill.numSamples, false, false, true);
            tempInfo.buffer = &tempBuffer;
        }

        tempInfo.clearActiveBufferRegion();
        transportSource.getNextAudioBlock(tempInfo);

        auto params = bloomverb::RuntimeParameters{};
        params.type = juce::jlimit(0, 7, typeChoice.load());
        params.mix = mix.load();
        params.decaySeconds = decay.load();
        params.size = size.load();
        params.motion = motion.load();
        params.harmonic = harmonic.load();
        params.warp = warp.load();

        engine.process(tempBuffer, params);

        for (int channel = 0; channel < bufferToFill.buffer->getNumChannels(); ++channel)
            bufferToFill.buffer->copyFrom(channel, bufferToFill.startSample, tempBuffer, channel, 0, bufferToFill.numSamples);
    }

    void releaseResources() override
    {
        transportSource.releaseResources();
        engine.reset();
    }

    void paint(juce::Graphics& g) override
    {
        juce::ColourGradient background(juce::Colour(0xff1a1630), 0.0f, 0.0f,
                                        juce::Colour(0xff080a16), 0.0f, static_cast<float>(getHeight()), false);
        g.setGradientFill(background);
        g.fillAll();
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced(14);
        auto top = area.removeFromTop(42);

        loadButton.setBounds(top.removeFromLeft(150));
        top.removeFromLeft(8);
        playButton.setBounds(top.removeFromLeft(100));
        top.removeFromLeft(8);
        loopToggle.setBounds(top.removeFromLeft(90));
        top.removeFromLeft(12);
        typeLabel.setBounds(top.removeFromLeft(36));
        typeBox.setBounds(top.removeFromLeft(140));

        area.removeFromTop(10);
        fileLabel.setBounds(area.removeFromTop(28));

        area.removeFromTop(10);
        auto row = area.removeFromTop(180);
        const int cellWidth = row.getWidth() / 6;
        layoutSliderCell(row, cellWidth, mixLabel, mixSlider);
        layoutSliderCell(row, cellWidth, decayLabel, decaySlider);
        layoutSliderCell(row, cellWidth, sizeLabel, sizeSlider);
        layoutSliderCell(row, cellWidth, motionLabel, motionSlider);
        layoutSliderCell(row, cellWidth, harmonicLabel, harmonicSlider);
        layoutSliderCell(row, cellWidth, warpLabel, warpSlider);
    }

    void tryLoadFileFromArgument(const juce::String& commandLine)
    {
        auto args = juce::StringArray::fromTokens(commandLine, true);
        args.trim();
        args.removeEmptyStrings();

        if (args.isEmpty())
            return;

        loadFile(juce::File(args[0]));
    }

private:
    void timerCallback() override
    {
        if (transportSource.isPlaying() && playButton.getButtonText() != "Stop")
            playButton.setButtonText("Stop");
        else if (!transportSource.isPlaying() && playButton.getButtonText() != "Play")
            playButton.setButtonText("Play");
    }

    void addSlider(juce::Slider& slider,
                   juce::Label& label,
                   const juce::String& name,
                   double min,
                   double max,
                   double value,
                   std::function<void()> onChange)
    {
        label.setText(name, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        label.setColour(juce::Label::textColourId, juce::Colours::white);
        addAndMakeVisible(label);

        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 66, 18);
        slider.setRange(min, max, 0.001);
        slider.setValue(value, juce::dontSendNotification);
        slider.onValueChange = std::move(onChange);
        addAndMakeVisible(slider);
    }

    static void layoutSliderCell(juce::Rectangle<int>& row, int cellWidth, juce::Label& label, juce::Slider& slider)
    {
        auto cell = row.removeFromLeft(cellWidth).reduced(6);
        label.setBounds(cell.removeFromTop(20));
        slider.setBounds(cell);
    }

    void openFileChooser()
    {
        chooser = std::make_unique<juce::FileChooser>("Select audio file to loop",
                                                      juce::File{},
                                                      "*.wav;*.aif;*.aiff;*.flac;*.mp3;*.ogg");

        const auto chooserFlags = juce::FileBrowserComponent::openMode
                                | juce::FileBrowserComponent::canSelectFiles;

        chooser->launchAsync(chooserFlags, [this](const juce::FileChooser& fc)
        {
            loadFile(fc.getResult());
        });
    }

    void loadFile(const juce::File& file)
    {
        if (!file.existsAsFile())
            return;

        std::unique_ptr<juce::AudioFormatReader> reader(formatManager.createReaderFor(file));
        if (reader == nullptr)
        {
            fileLabel.setText("Unsupported file: " + file.getFileName(), juce::dontSendNotification);
            return;
        }

        transportSource.stop();
        transportSource.setSource(nullptr);
        readerSource.reset();

        const auto sourceSampleRate = reader->sampleRate;
        const auto sourceNumChannels = static_cast<int>(reader->numChannels);

        readerSource = std::make_unique<juce::AudioFormatReaderSource>(reader.release(), true);
        readerSource->setLooping(shouldLoop.load());
        transportSource.setSource(readerSource.get(),
                                  32768,
                                  nullptr,
                                  sourceSampleRate,
                                  sourceNumChannels);
        transportSource.setPosition(0.0);
        transportSource.start();

        fileLabel.setText("Loaded: " + file.getFullPathName(), juce::dontSendNotification);
    }

    void togglePlayback()
    {
        if (readerSource == nullptr)
            return;

        if (transportSource.isPlaying())
            transportSource.stop();
        else
            transportSource.start();
    }

    bloomverb::BloomVerbEngine engine;
    juce::AudioFormatManager formatManager;
    juce::AudioTransportSource transportSource;
    std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
    std::unique_ptr<juce::FileChooser> chooser;
    juce::AudioBuffer<float> tempBuffer;

    juce::TextButton loadButton;
    juce::TextButton playButton;
    juce::ToggleButton loopToggle;
    juce::Label fileLabel;
    juce::Label typeLabel;
    juce::ComboBox typeBox;

    juce::Label mixLabel, decayLabel, sizeLabel, motionLabel, harmonicLabel, warpLabel;
    juce::Slider mixSlider, decaySlider, sizeSlider, motionSlider, harmonicSlider, warpSlider;

    std::atomic<int> typeChoice { 1 };
    std::atomic<float> mix { 0.25f };
    std::atomic<float> decay { 2.8f };
    std::atomic<float> size { 0.5f };
    std::atomic<float> motion { 0.3f };
    std::atomic<float> harmonic { 0.2f };
    std::atomic<float> warp { 0.3f };
    std::atomic<bool> shouldLoop { true };
};

class LoopTesterWindow final : public juce::DocumentWindow
{
public:
    explicit LoopTesterWindow(juce::String commandLine)
        : DocumentWindow("BloomVerb Loop Tester",
                         juce::Colour(0xff0f1222),
                         juce::DocumentWindow::allButtons)
    {
        auto content = std::make_unique<LoopTesterComponent>();
        content->tryLoadFileFromArgument(commandLine);
        setUsingNativeTitleBar(true);
        setResizable(true, true);
        setContentOwned(content.release(), true);
        centreWithSize(1080, 420);
        setVisible(true);
    }

    void closeButtonPressed() override
    {
        juce::JUCEApplication::getInstance()->systemRequestedQuit();
    }
};

class LoopTesterApplication final : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override { return "BloomVerb Loop Tester"; }
    const juce::String getApplicationVersion() override { return "0.1.0"; }
    bool moreThanOneInstanceAllowed() override { return true; }

    void initialise(const juce::String& commandLine) override
    {
        mainWindow = std::make_unique<LoopTesterWindow>(commandLine);
    }

    void shutdown() override
    {
        mainWindow.reset();
    }

    void systemRequestedQuit() override
    {
        quit();
    }

    void anotherInstanceStarted(const juce::String&) override {}

private:
    std::unique_ptr<LoopTesterWindow> mainWindow;
};
} // namespace

START_JUCE_APPLICATION(LoopTesterApplication)
