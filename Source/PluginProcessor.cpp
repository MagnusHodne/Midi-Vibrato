#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================

//Even though we aren't using
MidiPluginProcessor::MidiPluginProcessor()
        : AudioProcessor(BusesProperties()
                                 .withInput("Input", juce::AudioChannelSet::stereo(), true)
                                 .withOutput("Output", juce::AudioChannelSet::stereo(), true)
), parameters(*this, nullptr, "MidiVibrato", juce::AudioProcessorValueTreeState::ParameterLayout{
        std::make_unique<juce::AudioParameterFloat>("numSec", "Number of seconds to hold internally", 0.1f, 3.f, 1.f),
        std::make_unique<juce::AudioParameterInt>("inputCC", "CC to use as input signal", 1, 127, 2),
        std::make_unique<juce::AudioParameterInt>("ampCC", "CC to use as amplitude/depth signal", 1, 127, 21),
        std::make_unique<juce::AudioParameterInt>("rateCC", "CC to use for rate output", 1, 127, 20),
        std::make_unique<juce::AudioParameterFloat>("freqAttack", "Attack smoothing time for frequency", 0.1f, 2.f,
                                                    0.5f),
        std::make_unique<juce::AudioParameterFloat>("freqRelease", "Release smoothing time for frequency", 0.1f, 2.f,
                                                    0.5f),
        std::make_unique<juce::AudioParameterFloat>("rmsAttack", "Attack smoothing time for rms", 0.1f, 2.f, 0.5f),
        std::make_unique<juce::AudioParameterFloat>("rmsRelease", "Release smoothing time for rms", 0.1f, 2.f, 0.5f),
}),
          detector(std::make_unique<VibratoDetector>(parameters.getParameterAsValue("numSec").getValue())) {
    parameters.addParameterListener("numSec", this);
    parameters.addParameterListener("inputCC", this);
    parameters.addParameterListener("ampCC", this);
    parameters.addParameterListener("rateCC", this);
    parameters.addParameterListener("freqAttack", this);
    parameters.addParameterListener("freqRelease", this);
    parameters.addParameterListener("rmsAttack", this);
    parameters.addParameterListener("rmsRelease", this);
}

MidiPluginProcessor::~MidiPluginProcessor() {
    parameters.removeParameterListener("numSec", this);
    parameters.removeParameterListener("inputCC", this);
    parameters.removeParameterListener("ampCC", this);
    parameters.removeParameterListener("rateCC", this);
    parameters.removeParameterListener("freqAttack", this);
    parameters.removeParameterListener("freqRelease", this);
    parameters.removeParameterListener("rmsAttack", this);
    parameters.removeParameterListener("rmsRelease", this);
}

void MidiPluginProcessor::parameterChanged(const juce::String &parameterID, float newValue) {
    if (parameterID.equalsIgnoreCase("numSec")) detector->setNumSecondsToHold(newValue);
    if (parameterID.equalsIgnoreCase("inputCC")) detector->setInputController(static_cast<int>(newValue));
    if (parameterID.equalsIgnoreCase("ampCC")) detector->setRmsController(static_cast<int>(newValue));
    if (parameterID.equalsIgnoreCase("rateCC")) detector->setFrequencyController(static_cast<int>(newValue));
    if (parameterID.equalsIgnoreCase("freqAttack")) detector->setFrequencyAttack(newValue);
    if (parameterID.equalsIgnoreCase("freqRelease")) detector->setFrequencyRelease(newValue);
    if (parameterID.equalsIgnoreCase("rmsAttack")) detector->setRmsAttack(newValue);
    if (parameterID.equalsIgnoreCase("rmsRelease")) detector->setRmsRelease(newValue);

}

//==============================================================================
void MidiPluginProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    sr = sampleRate;
    spb = samplesPerBlock;
    // Use this method as the place to do any pre-playback
    // initialisation that you need...

    //Note that this is called whenever the user changes device settings, but not before they press play!
    detector->resetValues(sr, spb);
}

void MidiPluginProcessor::processBlock(juce::AudioBuffer<float> &buffer,
                                       juce::MidiBuffer &midiMessages) {
    detector->processMidi(midiMessages, buffer.getNumSamples());
    buffer.clear(); //Clearing the audio buffer...
}

void MidiPluginProcessor::releaseResources() {
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

//==============================================================================
const juce::String MidiPluginProcessor::getName() const {
    return JucePlugin_Name;
}

bool MidiPluginProcessor::acceptsMidi() const {
#if JucePlugin_WantsMidiInput
    return true;

#else
    return false;
#endif
}

bool MidiPluginProcessor::producesMidi() const {
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool MidiPluginProcessor::isMidiEffect() const {
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double MidiPluginProcessor::getTailLengthSeconds() const {
    return 0.0;
}

int MidiPluginProcessor::getNumPrograms() {
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
    // so this should be at least 1, even if you're not really implementing programs.
}

int MidiPluginProcessor::getCurrentProgram() {
    return 0;
}

void MidiPluginProcessor::setCurrentProgram(int index) {
    juce::ignoreUnused(index);
}

const juce::String MidiPluginProcessor::getProgramName(int index) {
    juce::ignoreUnused(index);
    return {};
}

void MidiPluginProcessor::changeProgramName(int index, const juce::String &newName) {
    juce::ignoreUnused(index, newName);
}

bool MidiPluginProcessor::isBusesLayoutSupported(const BusesLayout &layouts) const {
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
        && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
#if !JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}

//==============================================================================
bool MidiPluginProcessor::hasEditor() const {
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor *MidiPluginProcessor::createEditor() {
    return new MidiPluginEditor(*this);
}

//==============================================================================
void MidiPluginProcessor::getStateInformation(juce::MemoryBlock &destData) {
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    juce::ignoreUnused(destData);
}

void MidiPluginProcessor::setStateInformation(const void *data, int sizeInBytes) {
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    juce::ignoreUnused(data, sizeInBytes);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() {
    return new MidiPluginProcessor();
}
