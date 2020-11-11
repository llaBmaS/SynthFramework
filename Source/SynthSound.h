/*
  ==============================================================================

    SynthSound.h
    Created: 25 Mar 2020 4:32:38pm
    Author:  Sam

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

// Container object for a wavetable to be played by a voice
class SynthSound : public SynthesiserSound
{
public:
    SynthSound(std::shared_ptr<AudioBuffer<float>> table);

    //==============================================================================

    bool appliesToNote(int midiNoteNumber);

    bool appliesToChannel(int midiChannel);

    //==============================================================================

    std::shared_ptr<AudioBuffer<float>> getWavetable();

private:
    std::shared_ptr<AudioBuffer<float>> wavetable;
};