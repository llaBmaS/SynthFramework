/*
  ==============================================================================

    WavetableOscillatorManager.h
    Created: 6 Apr 2020 2:39:45pm
    Author:  Sam

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Common.h"
#include "PluginProcessor.h"
#include "SynthVoice.h"
#include "WavetableOscillator.h"


//==================================================================================
/** Manages a set of Oscillators.

*/
class WavetableOscillatorManager : public ValueTree::Listener
{
public:
    // Manager constructed from tree containing its parameters, the processor controlling the synth, and the voice that owns it
    WavetableOscillatorManager(ValueTree oscMgrParams, SynthFrameworkAudioProcessor& p, SynthVoice& v)
        : oscManagerParams (oscMgrParams),
          oscTree (oscManagerParams.getChildWithName(IDs::OSC_GROUP)),
          processor (p),
          voice (v)
    {
        oscManagerParams.addListener(this);

        gainEnv = std::make_unique<ADSR>();
        filterEnv = std::make_unique<ADSR>();

        tempGainEnv = std::make_unique<ADSR>();
        tempFilterEnv = std::make_unique<ADSR>();

        setSampleRate(voice.getSampleRate());
    }

    ~WavetableOscillatorManager()
    {
        oscManagerParams.removeListener(this);

        oscillators.clear();
        tempOscillators.clear();

        gainEnv.reset(nullptr);

        filterEnv.reset(nullptr);
    }

    // ======================================
    // ====== PROCESSOR & VOICE ACCESS ======
    // ======================================

    /** Sets the midi note to be played by the manager.
    
        Passing -1 as an argument resets the note, frequency, and tableDelta of all oscillators.
    */
    void setNote(int note)
    {
        currentNote = note;

        int numOsc = oscillators.size();
        jassert(numOsc == oscTree.getNumChildren());

        for (int i = 0; i < numOsc; ++i)
        {
            oscillators[i]->setNote(currentNote);
        }
    }

    /** Returns the midi note number currently being played by this manager.
    
    */
    int getCurrentNote()
    {
        return currentNote;
    }

    /** Sets the sample rate of all oscillators.

    */
    void setSampleRate(double sampleRate)
    {
        // Ignore if unchanged
        if (currentSampleRate != sampleRate)
        {
            // Update local variable
            currentSampleRate = sampleRate;

            // Update oscillators
            int numOsc = oscillators.size();

            //jassert(numOsc == oscTree.getNumChildren());

            for (int i = 0; i < numOsc; ++i)
            {
                oscillators[i]->setSampleRate(currentSampleRate);
                tempOscillators[i]->setSampleRate(currentSampleRate);
            }

            // Protects against invalid calls to ADSR.setSampleRate
            if (currentSampleRate > 0.0)
            {
                // Update envelopes
                gainEnv->setSampleRate(currentSampleRate);
                filterEnv->setSampleRate(currentSampleRate);
            }

        }
    }

    /** Returns the sum of next samples of all oscillators.

    */
    float getNextSample()
    {
        float value = 0.0f;

        if (isEnabled() && currentNote != -1)
        {
            // ==========================
            // ====== CURRENT NOTE ======
            // ==========================
            if (!gainEnv->isActive())
            {
                resetOscillators(oscillators);
                gainEnv->reset();

                releasing = false;
                setNote(-1);
                voice.clear();
            }
            else
            {
                int numOsc = oscillators.size();
                jassert(numOsc == oscTree.getNumChildren());

                // Sum each active oscillator's next sample
                for (int i = 0; i < numOsc; ++i)
                {
                    if (oscTree.getChild(i).getProperty(IDs::enabled))
                    {
                        value += oscillators[i]->getNextSample() * vLevel;
                    }
                }

                currentGainEnvLevel = gainEnv->getNextSample();

                // Scale by envelope value
                value *= currentGainEnvLevel;
            }

            // =========================
            // ====== FADING NOTE ======
            // =========================
            if (fading)
            {
                if (!tempGainEnv->isActive())
                {
                    // Reset temp members for new fade
                    clearFade();

                    // End fade
                    fading = false;
                }
                else
                {
                    int numOsc = tempOscillators.size();

                    // Sum each active tempOscillator's next sample
                    float value2 = 0.0f;
                    for (int i = 0; i < numOsc; ++i)
                    {
                        if (oscTree.getChild(i).getProperty(IDs::enabled))
                        {
                            value2 += tempOscillators[i]->getNextSample() * tempVLevel;
                        }
                    }

                    // Scale by tempEnvelope value
                    value2 *= tempGainEnv->getNextSample();

                    // Add to return value
                    value += value2;
                }
            }
        }

        return value;
    }

    /** Resets all oscillators to play a new note.

    */
    void resetOscillators(std::vector<std::unique_ptr<WavetableOscillator>>& oscArray)
    {
        int numOsc = oscArray.size();

        for (int i = 0; i < numOsc; ++i)
        {
            oscArray[i]->resetIndex();
        }
    }

    /** Returns true if the oscillator manager is enabled.
    
    */
    bool isEnabled()
    {
        return oscManagerParams.getProperty(IDs::enabled);
    }


    // ============================
    // ====== EVENT HANDLING ======
    // ============================
    /** Starts a new note and handles transition from current note.
    
    */
    void startNote(int midiNoteNumber, float velocity, int currentPitchWheelPosition)
    {
        // Currently playing a note
        if (currentNote != -1)
        {
            // For handling portamento and legato, no fade is necessary
            if (smoothSteal)
            {
                var stealMode = oscManagerParams.getProperty(IDs::voiceStealMode);

                if (stealMode == "PORTAMENTO")
                {
                    setNote(midiNoteNumber);
                }
                else if (stealMode == "LEGATO")
                {
                    // TODO gradually shift frequency
                }

                // End smooth steal
                smoothSteal = false;
            }
            // Fade between notes quickly
            else
            {
                // Move main parameters to temp
                initFade();

                // Set note to fade into
                setNote(midiNoteNumber);

                // Begin envelope of new note
                gainEnv->noteOn();

                fading = true;
            }
        }
        // Not currently playing a note
        else
        {
            vLevel = velocity * 0.5f;

            setNote(midiNoteNumber);

            gainEnv->noteOn();
        }
    }

    /** Releases the current note.
    
    */
    void stopNote(float velocity, bool allowTailOff)
    {
        // Only stop if currently playing
        if (currentNote != -1)
        {
            if (!releasing)
            {
                // Voice is being stolen and transition can be handled smoothly
                if (!allowTailOff && oscManagerParams.getProperty(IDs::voiceStealMode) != "NORMAL")
                {
                    smoothSteal = true;
                }
                // Voice is signalling for the note to be released
                else
                {
                    // Begin release
                    gainEnv->noteOff();
                    releasing = true;
                }
            }

            // If arrived here, voice is being stolen while in the release phase.
            // Transition cannot be handled smoothly, and must be crossfaded, which is handled by startNote.
        }
    }

    // =======================
    // ====== LISTENERS ======
    // =======================
    void valueTreePropertyChanged(ValueTree& treeWhosePropertyHasChanged, const Identifier& property) override
    {
        // OscillatorGroup parameters
        if (treeWhosePropertyHasChanged == oscManagerParams.getChildWithName(IDs::OSC_GROUP))
        {

        }
        // One of the oscillators' parameters has changed
        else if (treeWhosePropertyHasChanged.getParent() == oscTree)
        {
            // TODO make oscillator self-sufficient and update wavetable through its own listener,
            // TODO or remove wavetable change listeners altogether and only pass through synthsound

            // In the manager, we only care about changes to oscillator wavetables.
            // All other parameters are monitored by the oscillators themselves
            if (property == IDs::waveType)
            {
                ValueTree& osc = treeWhosePropertyHasChanged;
                int oscIndex = osc.getParent().indexOf(osc);

                std::shared_ptr<AudioBuffer<float>> newWavetable = processor.getWavetablePtrFromType(osc[IDs::waveType]);

                oscillators[oscIndex]->setWavetable(newWavetable);
                tempOscillators[oscIndex]->setWavetable(std::move(newWavetable));
            }
        }
        // Gain envelope parameters if the parameter value tree contains them
        else if (treeWhosePropertyHasChanged == oscManagerParams.getChildWithProperty(IDs::target, "GAIN"))
        {
            jassert(currentSampleRate != -1.0);

            if (property == IDs::attack)
            {
                gainEnvParameters.attack = treeWhosePropertyHasChanged.getProperty(IDs::attack);
            }
            else if (property == IDs::decay)
            {
                gainEnvParameters.decay = treeWhosePropertyHasChanged.getProperty(IDs::decay);
            }
            else if (property == IDs::sustain)
            {
                gainEnvParameters.sustain = treeWhosePropertyHasChanged.getProperty(IDs::sustain);
            }
            else if (property == IDs::release)
            {
                gainEnvParameters.release = treeWhosePropertyHasChanged.getProperty(IDs::release);
            }

            // Set new parameters on the envelope
            gainEnv->setParameters(gainEnvParameters);
        }
        // Filter envelope parameters if the parameter value tree contains them
        else if (treeWhosePropertyHasChanged == oscManagerParams.getChildWithProperty(IDs::target, "FILTER"))
        {
            jassert(currentSampleRate != -1.0);

            if (property == IDs::attack)
            {
                filterEnvParameters.attack = treeWhosePropertyHasChanged.getProperty(IDs::attack);
            }
            else if (property == IDs::decay)
            {
                filterEnvParameters.decay = treeWhosePropertyHasChanged.getProperty(IDs::decay);
            }
            else if (property == IDs::sustain)
            {
                filterEnvParameters.sustain = treeWhosePropertyHasChanged.getProperty(IDs::sustain);
            }
            else if (property == IDs::release)
            {
                filterEnvParameters.release = treeWhosePropertyHasChanged.getProperty(IDs::release);
            }

            // Set new properties on the envelope
            filterEnv->setParameters(filterEnvParameters);
        }
    }

    void valueTreeChildAdded(ValueTree& parentTree, ValueTree& childWhichHasBeenAdded)
    {
        if (parentTree == oscManagerParams)
        {
            // New envelope
            if (childWhichHasBeenAdded.getType() == IDs::ENVELOPE)
            {
                var targ = childWhichHasBeenAdded.getProperty(IDs::target);

                if (targ == "GAIN")
                {
                    // Add a gain envelope to the scope of the manager
                    // TODO
                    
                }
                else if (targ == "FILTER")
                {
                    // Add a filter envelope to the scope of the manager
                    // TODO

                }
            }
        }
        // New oscillator
        else if (parentTree == oscManagerParams.getChildWithName(IDs::OSC_GROUP))
        {
            var wavetype = childWhichHasBeenAdded[IDs::waveType];

            // Get a shared pointer to the wavetable the new oscillator should use
            std::shared_ptr<AudioBuffer<float>> wavetableToPass = processor.getWavetablePtrFromType(wavetype);

            // Create new unique ptrs to two identical oscillators (one main, one for fade)
            auto newOsc = std::make_unique<WavetableOscillator>(childWhichHasBeenAdded, wavetableToPass);
            auto newTempOsc = std::make_unique<WavetableOscillator>(childWhichHasBeenAdded, std::move(wavetableToPass));

            // Init sample rates of new oscillators
            newOsc->setSampleRate(currentSampleRate);
            newTempOsc->setSampleRate(currentSampleRate);

            // Ignore note of tempOsc, as that will be handled during a fade
            newOsc->setNote(currentNote);

            // Move oscillators into vectors
            oscillators.push_back(std::move(newOsc));
            tempOscillators.push_back(std::move(newTempOsc));
        }
    }

    void valueTreeChildRemoved(ValueTree& parentTree, ValueTree& childWhichHasBeenRemoved, int indexFromWhichChildWasRemoved)
    {
        if (parentTree == oscManagerParams)
        {
            // Envelope removed
            if (childWhichHasBeenRemoved.getType() == IDs::ENVELOPE)
            {
                if (childWhichHasBeenRemoved.getProperty(IDs::target) == "GAIN")
                {
                    // Remove gain envelope from the scope of the manager
                    // TODO

                }
                else if (childWhichHasBeenRemoved.getProperty(IDs::target) == "FILTER")
                {
                    // Remove filter envelope from the scope of the manager
                    // TODO

                }
            }
        }
        // Oscillator removed
        else if (parentTree == oscTree)
        {
            oscillators.erase(oscillators.begin() + indexFromWhichChildWasRemoved);
            tempOscillators.erase(tempOscillators.begin() + indexFromWhichChildWasRemoved);
        }
    }


private:
    //==============================================================================
    // A reference to the processor
    SynthFrameworkAudioProcessor& processor;
    

    // A reference to the voice that owns this oscillator manager
    SynthVoice& voice;

    //==============================================================================
    // The parameters associated with this manager
    ValueTree oscManagerParams;
    ValueTree oscTree;

    double currentSampleRate = -1.0;

    //==============================================================================
    // =====================================
    // ====== OSCILLATORS & ENVELOPES ======
    // =====================================
    std::vector<std::unique_ptr<WavetableOscillator>> oscillators;
    // The current note being played
    int currentNote = -1;
    // The current vLevel set by the velocity of the note press
    float vLevel = 0.0f;

    // Envelopes
    std::unique_ptr<ADSR> gainEnv = nullptr;
    ADSR::Parameters gainEnvParameters;
    std::unique_ptr<ADSR> filterEnv = nullptr;
    ADSR::Parameters filterEnvParameters;

    // A stored copy of the last gain envelope sample, for initiating fades
    float currentGainEnvLevel = 0.0f;

    // The time in seconds for a note to fade quickly
    float fastReleaseTime = 0.01f;

    // ============================================
    // ====== TEMPORARY VARIABLES FOR FADING ======
    // ============================================
    std::vector<std::unique_ptr<WavetableOscillator>> tempOscillators;
    
    float tempVLevel = 0.0f;

    std::unique_ptr<ADSR> tempGainEnv = nullptr;
    ADSR::Parameters tempGainEnvParameters;
    std::unique_ptr<ADSR> tempFilterEnv = nullptr;

    // Flag: current note is releasing
    bool releasing = false;

    // Flag: voice steal requires no fade
    bool smoothSteal = false;

    // Flag: fading between notes, sum both the main and temporary oscillators' samples
    bool fading = false;


    //==============================================================================
    /** Initiates a fade between the current note and the new note.

        Fills the temporary variables with copies of the main ones.
    */
    void initFade()
    {
        // =========================================
        // ====== COPY MAIN VARIABLES TO TEMP ======
        // =========================================
        int numOsc = oscillators.size();
        jassert(numOsc == oscTree.getNumChildren());

        // Swap oscillators with temp ones
        for (int i = 0; i < numOsc; ++i)
        {
            oscillators[i].swap(tempOscillators[i]);
        }
        
        // Swap envelopes with temp ones
        tempGainEnv.swap(gainEnv);
        tempFilterEnv.swap(filterEnv);

        // Set env params for new note
        gainEnv->setParameters(gainEnvParameters);
        gainEnv->reset();

        // Whether or not the current note is releasing, set flag to false for the new note
        releasing = false;

        // Create new parameters for an immediate fast release starting at the currentGainEnvlevel
        tempGainEnvParameters.attack = 0.0f;
        tempGainEnvParameters.decay = 0.0f;
        tempGainEnvParameters.sustain = currentGainEnvLevel;
        tempGainEnvParameters.release = fastReleaseTime;



        // Set new parameters for fade, immediately release
        tempGainEnv->setParameters(tempGainEnvParameters);
        tempGainEnv->noteOff();

        // Store previous vLevel for fade
        tempVLevel = vLevel;
    }

    // Clears the temporary variables after a fade is complete.
    void clearFade()
    {
        // Reset temporary oscillators for a new fade
        resetOscillators(tempOscillators);

        // Reset temporary envelopes for a new fade
        tempGainEnv->reset();
        tempFilterEnv->reset();

        // Reset tempVLevel TODO might be unecessary
        tempVLevel = -1.0f;
    }
    
    //==============================================================================

    // Helper for updating oscillator indices after a wavetable has been removed
    void updateOscillatorIndices(int indexRemovedFrom)
    {
        int numOsc = oscillators.size();
        jassert(numOsc == oscTree.getNumChildren());

        for (int i = indexRemovedFrom; i < numOsc; ++i)
        {
            oscillators[i]->setOscNumber(i);
        }
    }

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WavetableOscillatorManager)
};