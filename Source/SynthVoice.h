/*
  ==============================================================================

    SynthVoice.h
    Created: 25 Mar 2020 4:36:53pm
    Author:  Sam

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Common.h"
#include "PluginProcessor.h"
#include "SynthSound.h"

class WavetableOscillatorManager;
// Plays a wavetable described by SynthSound
class SynthVoice : public SynthesiserVoice
{
public:
    SynthVoice(ValueTree oscMgrParams, SynthFrameworkAudioProcessor& p);

    ~SynthVoice();

    bool canPlaySound(SynthesiserSound* sound);

    void startNote(int midiNoteNumber, float velocity, SynthesiserSound* sound, int currentPitchWheelPosition);

    void stopNote(float velocity, bool allowTailOff);

    void pitchWheelMoved(int newPitchWheelValue);

    void controllerMoved(int controllerNumber, int newControllerValue);

    void renderNextBlock(AudioBuffer<float>& outputBuffer, int startSample, int numSamples);

    //==============================================================================

    /** Call to clear a synth voice's current note externally.
    
    */
    void clear();

private:
    // Reference to the processor that owns the synth
    SynthFrameworkAudioProcessor& processor;

    // A set of oscillators managed by its containing class. This class does most of the work processing
    // the output of multiple oscillators, including fading between notes when necessary
    std::unique_ptr<WavetableOscillatorManager> oscillatorManager;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SynthVoice)
};