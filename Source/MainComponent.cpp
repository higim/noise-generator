#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    // Make sure you set the size of the component after
    // you add any child components.
    setSize (800, 600);

    // Some platforms require permissions to open input channels so request that here
    if (juce::RuntimePermissions::isRequired (juce::RuntimePermissions::recordAudio)
        && ! juce::RuntimePermissions::isGranted (juce::RuntimePermissions::recordAudio))
    {
        juce::RuntimePermissions::request (juce::RuntimePermissions::recordAudio,
                                           [&] (bool granted) { setAudioChannels (granted ? 2 : 0, 2); });
    }
    else
    {
        // Specify the number of input and output channels that we want to open
        setAudioChannels (0, 2);
    }
}

MainComponent::~MainComponent()
{
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
}

//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    // This function will be called when the audio device is started, or when
    // its settings (i.e. sample rate, block size, etc) are changed.

    // You can use this function to initialise any resources you might need,
    // but be careful - it will be called on the audio thread, not the GUI thread.

    // For more details, see the help for AudioProcessor::prepareToPlay()
    juce::String message;
    message << "Preparing to play audio...\n";
    message << " samplesPerBlockExpected = " << samplesPerBlockExpected << "\n";
    message << " sampleRate = " << sampleRate;
    juce::Logger::getCurrentLogger()->writeToLog(message);

    matrix.resize(NUMROWS);
    for (auto& row: matrix)
        row.resize(samplesPerBlockExpected, 0.0f);
    
    pinkBuffer.resize(samplesPerBlockExpected);
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    // Your audio-processing code goes here!

    // For more details, see the help for AudioProcessor::getNextAudioBlock()

    for (auto channel = 0; channel < bufferToFill.buffer->getNumChannels(); channel++)
    {
        auto* buffer = bufferToFill.buffer->getWritePointer(channel, bufferToFill.startSample);

        pinkNoiseGenerator(bufferToFill.numSamples, 16);

        
        for (int i = 0; i < bufferToFill.numSamples; ++i) {
            pinkBuffer[i] = pinkBuffer[i] * 0.25f - 0.125f;
        }

        // Copy only the valid number of samples
        std::copy(pinkBuffer.begin(), pinkBuffer.begin() + bufferToFill.numSamples, buffer);
    }
}

void MainComponent::releaseResources()
{
    // This will be called when the audio device stops, or when it is being
    // restarted due to a setting change.

    // For more details, see the help for AudioProcessor::releaseResources()
    juce::Logger::getCurrentLogger()->writeToLog("Releasing audio resouces");
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    // You can add your drawing code here!
}

void MainComponent::resized()
{
    // This is called when the MainContentComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
}

void MainComponent::pinkNoiseGenerator (int numSamples, int numRows) {
    for (auto sample = 0; sample < numSamples; ++sample)
        matrix[0][sample] = random.nextFloat();

    for (auto row = 1; row < numRows; ++row) {
        int stepSize = 1 << row;
        int numSteps = static_cast<int>(std::ceil(static_cast<float>(numSamples) / stepSize));
        std::vector<float> randomValues(numSteps, 0.0);
        for (int i = 0; i < numSteps; ++i) {
            float value = random.nextFloat();
            int start = i * stepSize;
            int end = std::min((i + 1) * stepSize, numSamples);
            for (int j = start; j < end; ++j)
                matrix[row][j] = value;
        }
    }

    // Clear and accumulate pinkBuffer
    std::fill(pinkBuffer.begin(), pinkBuffer.begin() + numSamples, 0.0f);

    for (auto i = 0; i < numSamples; ++i)
        for (int row = 0; row < numRows; ++row)
            pinkBuffer[i] += matrix[row][i];

    float max_abs = 0.0f;
    for (int i = 0; i < numSamples; ++i)
        max_abs = std::max(max_abs, std::abs(pinkBuffer[i]));

    if (max_abs > 0.0f) {
        for (int i = 0; i < numSamples; ++i)
            pinkBuffer[i] /= max_abs;
    }
}
