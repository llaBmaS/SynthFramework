/*
  ==============================================================================

    Common.cpp
    Created: 14 Apr 2020 9:27:46pm
    Author:  Sam

  ==============================================================================
*/

#include "Common.h"
#include "WavetableCreator.h"

namespace IDs
{
    // =========================
    // ====== IDENTIFIERS ======
    // =========================
    
    Identifier ROOT("Root");

    Identifier enabled("Enabled");

    Identifier OSC_MGR("OscillatorManager");
        Identifier voiceStealMode("VoiceStealMode");
        Identifier OSC_GROUP("OscillatorGroup");
            Identifier OSC("Oscillator");
                Identifier waveType("WaveType");
                Identifier DETUNE("Detune");
                    Identifier detuneOctave("Octave");
                    Identifier detuneCoarse("Coarse");
                    Identifier detuneFine("Fine");
        Identifier ENVELOPE("Envelope");
            Identifier target("Target");
            Identifier attack("Attack");
            Identifier decay("Decay");
            Identifier sustain("Sustain");
            Identifier release("Release");

    Identifier polyphony("Polyphony");

}