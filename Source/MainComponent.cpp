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
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    // Your audio-processing code goes here!

    // For more details, see the help for AudioProcessor::getNextAudioBlock()

    // Right now we are not producing any data, in which case we need to clear the buffer
    // (to prevent the output of random noise)
    for (auto channel = 0; channel < bufferToFill.buffer->getNumChannels(); channel++)
    {
        auto* buffer = bufferToFill.buffer->getWritePointer(channel, bufferToFill.startSample);

        auto pink = pinkNoiseGenerator(bufferToFill.numSamples, 16);

        for (float&x : pink) {
            x = x * 0.25f - 0.125f;
        }
        
        std::copy(pink.begin(), pink.end(), buffer);
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

std::vector<float> MainComponent::pinkNoiseGenerator (int numSamples, int numRows) {
    std::vector<std::vector<float>> matrix(numRows, std::vector<float>(numSamples, 0.0));

    for (auto sample = 0; sample < numSamples; ++sample)
        matrix[0][sample] = random.nextFloat();

    for (auto row = 1; row < numRows; ++row) {
        int stepSize = std::pow(2, row); // Alternative: 1 << row
        int numSteps = static_cast<int>(std::ceil(static_cast<float>(numSamples) / stepSize));
        std::vector<float> randomValues(numSteps, 0.0);
        for (auto sample = 0; sample < numSteps; ++sample)
            randomValues[sample] = random.nextFloat();
        for (auto i = 0; i < numSteps; ++i)
            for (auto j = i * stepSize; j < std::min((i + 1) * stepSize, numSamples); ++j)
                matrix[row][j] = randomValues[i];
    }

    std::vector<float> pink(numSamples, 0.0);
    for (auto i = 0; i < numSamples; ++i)
        for (int row = 0; row < numRows; ++row)
            pink[i] += matrix[row][i];

    float max_abs = 0.0f;
    for (float x : pink) {
        if (std::abs(x) > max_abs) max_abs = std::abs(x);
    }
    if (max_abs > 0.0f) {
        for (float& x: pink) {
            x /= max_abs;
        }
    }

    return pink;
}
