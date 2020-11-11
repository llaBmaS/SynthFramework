/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "SynthVoice.h"
#include "SynthSound.h"
#include "WavetableCreator.h"

//==============================================================================
SynthFrameworkAudioProcessor::SynthFrameworkAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
    // Create wavetables that can be used by oscillators
    initBaseWavetables(wavetableSize);

    // Create and initialise the main PARAMETER tree 
    initValueTrees();

    // Parameters used to initialise each Voice
    ValueTree& oscMgrParams = PARAMETERS.getChildWithName(IDs::OSC_MGR);
    // Grab polyphony setting from tree
    numVoices = PARAMETERS.getChild(0).getProperty(IDs::polyphony);

    mySynth.clearVoices();
    for (int i = 0; i < numVoices; ++i)
    {
        mySynth.addVoice(new SynthVoice(oscMgrParams, *this));
    }

    // Manually set some gain envelope parameters
    ValueTree& gainEnvelopeParameters = PARAMETERS.getChildWithName(IDs::OSC_MGR).getChildWithName(IDs::ENVELOPE);
    gainEnvelopeParameters.setProperty(IDs::attack, 0.1f, nullptr);
    gainEnvelopeParameters.setProperty(IDs::decay, 2.0f, nullptr);
    gainEnvelopeParameters.setProperty(IDs::sustain, 0.6f, nullptr);
    gainEnvelopeParameters.setProperty(IDs::release, 2.0f, nullptr);

    
    mySynth.clearSounds();
    mySynth.addSound(new SynthSound(SineTable));

    // Add an oscillator
    TREE_addOscillatorNode("SINE");
}

SynthFrameworkAudioProcessor::~SynthFrameworkAudioProcessor()
{
    mySynth.clearVoices();
    mySynth.clearSounds();

    clearWavetables();
}

//==============================================================================
const String SynthFrameworkAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool SynthFrameworkAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool SynthFrameworkAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool SynthFrameworkAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double SynthFrameworkAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int SynthFrameworkAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int SynthFrameworkAudioProcessor::getCurrentProgram()
{
    return 0;
}

void SynthFrameworkAudioProcessor::setCurrentProgram (int index)
{
}

const String SynthFrameworkAudioProcessor::getProgramName (int index)
{
    return {};
}

void SynthFrameworkAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void SynthFrameworkAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    lastSampleRate = sampleRate;

    mySynth.setCurrentPlaybackSampleRate(lastSampleRate);
}

void SynthFrameworkAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool SynthFrameworkAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void SynthFrameworkAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
    // Clears all channels of buffer because there should be no audio input
    // TODO change if implementing audio-input channels
    buffer.clear();


    // calls on synth to render a full block of multi-channel audio with the current voices and sounds given the midi input
    mySynth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());
}

//==============================================================================
bool SynthFrameworkAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* SynthFrameworkAudioProcessor::createEditor()
{
    return new SynthFrameworkAudioProcessorEditor (*this);
}

//==============================================================================
void SynthFrameworkAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void SynthFrameworkAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
void SynthFrameworkAudioProcessor::TREE_addOscillatorNode(var waveTypeToUse)
{
    // The tree being added to
    ValueTree oscGroup = PARAMETERS.getChildWithName(IDs::OSC_MGR).getChildWithName(IDs::OSC_GROUP);
    int numOsc = oscGroup.getNumChildren();

    ValueTree newOsc = OscillatorParameters.createCopy();
    newOsc.setProperty(IDs::waveType, waveTypeToUse, nullptr);

    // Add node to the oscillator group, which should trigger the listeners in
    // oscillatorManager objects and create a new oscillator
    oscGroup.addChild(newOsc, numOsc, nullptr);
}

void SynthFrameworkAudioProcessor::TREE_removeOscillatorNode(int index)
{
    ValueTree oscGroup = PARAMETERS.getChildWithName(IDs::OSC_MGR).getChildWithName(IDs::OSC_GROUP);

    // Check validity of index
    jassert(index < oscGroup.getNumChildren());

    // Remove node
    oscGroup.removeChild(oscGroup.getChild(index), nullptr);
}

std::shared_ptr<AudioBuffer<float>> SynthFrameworkAudioProcessor::getWavetablePtrFromType(var waveType)
{
    if (waveType == "SINE")
    {
        return SineTable;
    }
    else if (waveType == "SAW")
    {
        return SawTable;
    }
    else if (waveType == "RAMP")
    {
        return RampTable;
    }
    else if (waveType == "TRIANGLE")
    {
        return TriangleTable;
    }
    else if (waveType == "SQUARE")
    {
        return SquareTable;
    }
}

void SynthFrameworkAudioProcessor::initBaseWavetables(int tableSize)
{
    // Store wavetables in shared_ptrs
    SineTable = WavetableCreator::createSineTable(wavetableSize);
    SawTable = WavetableCreator::createSawTable(wavetableSize);
    RampTable = WavetableCreator::createRampTable(wavetableSize);
    TriangleTable = WavetableCreator::createTriangleTable(wavetableSize);
    SquareTable = WavetableCreator::createSquareTable(wavetableSize);
}

void SynthFrameworkAudioProcessor::clearWavetables()
{
    SineTable.reset();
    SawTable.reset();
    RampTable.reset();
    TriangleTable.reset();
    SquareTable.reset();
}

void SynthFrameworkAudioProcessor::initValueTrees()
{
    //==============================================================================
    // A node containing a set of default envelope parameters
    EnvelopeParameters = ValueTree(IDs::ENVELOPE);
    jassert(EnvelopeParameters.isValid());
    // Default parameters:
    EnvelopeParameters.setProperty(IDs::enabled, 1, nullptr);
    EnvelopeParameters.setProperty(IDs::attack, 0.8f, nullptr);
    EnvelopeParameters.setProperty(IDs::decay, 0.5f, nullptr);
    EnvelopeParameters.setProperty(IDs::sustain, 1.0f, nullptr);
    EnvelopeParameters.setProperty(IDs::release, 1.0f, nullptr);

    // Copies must define: target

    //==============================================================================
    // A node containing a set of default detune parameters
    DetuneParameters = ValueTree(IDs::DETUNE);
    jassert(DetuneParameters.isValid());
    // Default parameters:
    DetuneParameters.setProperty(IDs::enabled, 1, nullptr);
    DetuneParameters.setProperty(IDs::detuneOctave, 0, nullptr);
    DetuneParameters.setProperty(IDs::detuneCoarse, 0, nullptr);
    DetuneParameters.setProperty(IDs::detuneFine, 0, nullptr);

    //==============================================================================
    // A node containing a set of default oscillator parameters
    OscillatorParameters = ValueTree(IDs::OSC);
    jassert(OscillatorParameters.isValid());
    // Default parameters:
    OscillatorParameters.setProperty(IDs::enabled, 1, nullptr);
    OscillatorParameters.setProperty(IDs::waveType, "SINE", nullptr);
    OscillatorParameters.addChild(DetuneParameters.createCopy(), -1, nullptr);

    //==============================================================================
    // Create a node for all settings managed by a WavetableOscillatorManager
    ValueTree oscillatorManagerParameters(IDs::OSC_MGR);
    oscillatorManagerParameters.setProperty(IDs::enabled, 1, nullptr);
    oscillatorManagerParameters.setProperty(IDs::voiceStealMode, "PORTAMENTO", nullptr);

    // Create a container node for the Oscillators
    ValueTree oscillators(IDs::OSC_GROUP);

    // Add group to manager
    oscillatorManagerParameters.addChild(oscillators, 0, nullptr);

    //==============================================================================
    // Add gainEnvelope to the manager
    oscillatorManagerParameters.addChild(EnvelopeParameters.createCopy(), 1, nullptr);
    oscillatorManagerParameters.getChild(1).setProperty(IDs::target, "GAIN", nullptr);

    //==============================================================================
    // A node containing general synth parameters used by multiple classes
    ValueTree parameters("Parameters");
    
    parameters.setProperty(IDs::polyphony, 5, nullptr);


    // =============================================================================
    // Assemble the main PARAMETER tree
    PARAMETERS = ValueTree(IDs::ROOT);
    PARAMETERS.addChild(parameters, 0, nullptr);
    PARAMETERS.addChild(oscillatorManagerParameters, 1, nullptr);

}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SynthFrameworkAudioProcessor();
}
