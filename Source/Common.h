/*
  ==============================================================================

    Common.h
    Created: 6 Apr 2020 11:39:26pm
    Author:  Sam

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

/** A namespace containing juce:Identifier objects for use with reading/writing the value tree.

*/
namespace IDs
{
    // =========================
    // ====== IDENTIFIERS ======
    // =========================
    extern Identifier ROOT;

    // Whether a node is enabled or not
    extern Identifier enabled;

    // Default tree structure
    extern Identifier OSC_MGR;
        extern Identifier voiceStealMode;
        extern Identifier OSC_GROUP;
            extern Identifier OSC;
                extern Identifier waveType;
                extern Identifier DETUNE;
                    extern Identifier detuneOctave;
                    extern Identifier detuneCoarse;
                    extern Identifier detuneFine;
        extern Identifier ENVELOPE;
            // The paramater to which a node is applied (ie gain envelope vs filter envelope)
            extern Identifier target;
            extern Identifier attack;
            extern Identifier decay;
            extern Identifier sustain;
            extern Identifier release;

    extern Identifier polyphony;
}


