/*
  ==============================================================================

    SynthVoice.cpp
    Created: 28 Mar 2020 7:41:18pm
    Author:  Sam

  ==============================================================================
*/


#include "SynthVoice.h"
#include "WavetableOscillatorManager.h"


// ======================
// === JUCE OVERRIDES ===
// ======================

// Constructor takes in the tree containing the parameters for its oscillator manager
SynthVoice::SynthVoice(ValueTree oscMgrParams, SynthFrameworkAudioProcessor& p)
    : processor(p)
{
    // Init oscillator manager
    oscillatorManager = std::make_unique<WavetableOscillatorManager>(oscMgrParams, processor, *this);
}

SynthVoice::~SynthVoice()
{
    oscillatorManager.reset();
}

bool SynthVoice::canPlaySound(SynthesiserSound* sound)
{
    return dynamic_cast<SynthSound*>(sound) != nullptr;
}

void SynthVoice::startNote(int midiNoteNumber, float velocity, SynthesiserSound* sound, int currentPitchWheelPosition)
{
    oscillatorManager->setSampleRate(getSampleRate());

    // Pass control to oscillator manager
    oscillatorManager->startNote(midiNoteNumber, velocity, currentPitchWheelPosition);
}

void SynthVoice::stopNote(float velocity, bool allowTailOff)
{
    // Pass control to oscillator manager
    oscillatorManager->stopNote(velocity, allowTailOff);
}

void SynthVoice::pitchWheelMoved(int newPitchWheelValue)
{

}

void SynthVoice::controllerMoved(int controllerNumber, int newControllerValue)
{

}

void SynthVoice::renderNextBlock(AudioBuffer<float>& outputBuffer, int startSample, int numSamples)
{
    // Check if the oscillators have a note to play
    if (oscillatorManager->getCurrentNote() != -1)
    {
        // Store to avoid repeated calls
        int numChannelsOut = outputBuffer.getNumChannels();

        // Write pointers for each output channel
        float** channelWritePtrs = outputBuffer.getArrayOfWritePointers();

        // Index of last sample to fill in buffer
        int stopSample = startSample + numSamples;

        for (int sample = startSample; sample < stopSample; ++sample)
        {
            // Get next sample from oscillatorManager
            float nextSampleLeveled = oscillatorManager->getNextSample();

            // Write sample to every output channel
            for (int channel = 0; channel < numChannelsOut; ++channel)
            {
                channelWritePtrs[channel][sample] += nextSampleLeveled;
            }
        }
    }
}

//==================================================================================

void SynthVoice::clear()
{
    clearCurrentNote();
}

