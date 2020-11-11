/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Common.h"

//==============================================================================
namespace
{
    // ======================
    // === COPYABLE TREES ===
    // ======================
    // A node containing a set of default envelope parameters
    ValueTree EnvelopeParameters;
    // A node containing a set of default detune parameters
    ValueTree DetuneParameters;
    // A node containing a set of default oscillator parameters
    ValueTree OscillatorParameters;
}

/**
*/
class SynthFrameworkAudioProcessor  : public AudioProcessor,
                                      public ValueTree::Listener
{
public:
    //==============================================================================
    SynthFrameworkAudioProcessor();
    ~SynthFrameworkAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (AudioBuffer<float>&, MidiBuffer&) override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    /** Add a new oscillator node to the value tree.

        - Passes a wavetable reference as a CachedValue<> which can be accessed by the oscillator.

        - Triggers a listener in WavetableOscillatorManager to create a new oscillator.
    */
    void TREE_addOscillatorNode(var waveTypeToUse);

    /** Removes an oscillator node from the tree by its index.

        - Triggers a listener in WavetableOscillatorManager to delete the oscillator.
    */
    void TREE_removeOscillatorNode(int index);

    //==============================================================================
    /** Returns a shared pointer to the wavetable buffer associated with a given waveType.

    */
    std::shared_ptr<AudioBuffer<float>> getWavetablePtrFromType(var waveType);

    //==============================================================================

private:
    // The global parameter tree, which contains all settings
    ValueTree PARAMETERS;

    Synthesiser mySynth;
    int numVoices;

    double lastSampleRate;

    // ==================
    // === WAVETABLES ===
    // ==================
    // Wavetables to be referenced by oscillators
    std::shared_ptr<AudioBuffer<float>> SineTable;
    std::shared_ptr<AudioBuffer<float>> SawTable;
    std::shared_ptr<AudioBuffer<float>> RampTable;
    std::shared_ptr<AudioBuffer<float>> TriangleTable;
    std::shared_ptr<AudioBuffer<float>> SquareTable;
    
    // TODO implement different wavetables for different frequencies
    const int wavetableSize = 1024;


    // Initializes the wavetables
    void initBaseWavetables(int tableSize);

    // Clears the memory of the base wavetables
    // TODO store this externally instead
    void clearWavetables();

    // Initializes the ValueTrees
    void initValueTrees();


    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SynthFrameworkAudioProcessor)
};
