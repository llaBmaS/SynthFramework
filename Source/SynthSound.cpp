/*
  ==============================================================================

    SynthSound.cpp
    Created: 28 Mar 2020 7:41:25pm
    Author:  Sam

  ==============================================================================
*/

#include "SynthSound.h"


//==============================================================================

SynthSound::SynthSound(std::shared_ptr<AudioBuffer<float>> table)
    : wavetable (table)
{

}

bool SynthSound::appliesToNote(int midiNoteNumber)
{
    return true;
}

bool SynthSound::appliesToChannel(int midiChannel)
{
    return true;
}

std::shared_ptr<AudioBuffer<float>> SynthSound::getWavetable()
{
    return wavetable;
}