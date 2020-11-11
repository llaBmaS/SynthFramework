/*
  ==============================================================================

    WavetableOscillator.h
    Created: 28 Mar 2020 11:43:17pm
    Author:  Sam

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Common.h"


/** A class containing a wavetable reference and methods to parse through it.


*/
class WavetableOscillator : public ValueTree::Listener
{
public:
    /** Creates a new WavetableOscillator given an oscillatorParameters tree and a pointer to the wavetable.

    */
    WavetableOscillator(ValueTree params, std::shared_ptr<AudioBuffer<float>> wavetableToUse)
        : oscParams (params)
    {
        oscWavetable = wavetableToUse;
        tableSize = oscWavetable->getNumSamples() - 1;

        // Initialise the cached detune parameter values
        initDetune();

        // Ensure mono wavetable
        jassert(oscWavetable->getNumChannels() == 1);

        oscParams.addListener(this);
    }

    ~WavetableOscillator()
    {
        oscWavetable.reset();
        oscParams.removeListener(this);
    }

    //==============================================================================
    /** Returns a unique pointer to a clone of the oscillator.

    */
    std::unique_ptr<WavetableOscillator> clone()
    {
        std::unique_ptr<WavetableOscillator> oscClone = std::make_unique<WavetableOscillator>(oscParams, oscWavetable);

        // Copy internal parameters
        oscClone->currentSampleRate = currentSampleRate;
        oscClone->currentNote = currentNote;
        oscClone->currentFrequency = currentFrequency;
        oscClone->currentIndex = currentIndex;

        return std::move(oscClone);
    }

    //==============================================================================
    /** Returns true if there is a tableDelta set, as otherwise the oscillator can't play.

        There will only be a tableDelta when both sample rate & frequency are set.
    */
    bool hasDelta()
    {
        return tableDelta != 0.0f;
    }

    /** Returns true if the oscillator is enabled

    */
    bool isEnabled()
    {
        return oscParams.getProperty(IDs::enabled);
    }

    // ##############################
    // ###### PARAMETER ACCESS ######
    // ##############################

    void setOscNumber(int newOscNumber)
    {
        oscNumber = newOscNumber;
    }

    // =======================
    // ====== WAVETABLE ======
    // =======================
    /** Set a new wavetable for the oscillator to walk through.

    */
    void setWavetable(std::shared_ptr<AudioBuffer<float>> newWavetable)
    {
        oscWavetable = newWavetable;

        // Store old table size for comparison
        int oldSize = tableSize;
        // Get new size
        tableSize = oscWavetable->getNumSamples() - 1;

        // If size change
        if (tableSize != oldSize)
        {
            // Update position in wavetable
            float ratio = (float)tableSize / (float)oldSize;
            currentIndex *= ratio;

            // Update delta for new size
            updateTableDelta();
        }
    }

    // =========================
    // ====== SAMPLE RATE ======
    // =========================
    /** Sets the sample rate for this oscillator to render at.

        Updates the tableDelta of the oscillator.
    */
    void setSampleRate(double newSampleRate)
    {
        currentSampleRate = newSampleRate;
        updateTableDelta();
    }


    // ===========================
    // ====== NOTE & DETUNE ======
    // ===========================
    /** Sets the midi note to be played by the oscillator.
        
        Updates the frequency and tableDelta of the oscillator.
    */
    void setNote(int note)
    {
        currentNote = note;
        updateFrequency();
    }


    // ===========================
    // ====== SAMPLE OUTPUT ======
    // ===========================
    /** Returns the next sample this oscillator should play.

    */
    forcedinline float getNextSample() noexcept
    {
        if (!isEnabled())
        {
            return 0.0f;
        }
        else
        {
            if (hasDelta())
            {
                int index0 = (unsigned int)currentIndex;
                int index1 = index0 + 1;

                float frac = currentIndex - (float)index0;

                auto* table = oscWavetable->getReadPointer(0);
                float value0 = table[index0];
                float value1 = table[index1];

                float currentSample = value0 + frac * (value1 - value0);

                if ((currentIndex += tableDelta) > tableSize)
                {
                    currentIndex -= tableSize;
                }

                return currentSample;
            }
        }
    } 

    float getCurrentIndex()
    {
        return currentIndex;
    }

    void setCurrentIndex(float newIndex)
    {
        currentIndex = newIndex;
    }

    /** Resets the currentIndex to 0.0 to play a new note
        
    */
    void resetIndex()
    {
        currentIndex = 0.0f;
    }


    // =======================
    // ====== LISTENERS ======
    // =======================
    /**
    
    */
    void valueTreePropertyChanged(ValueTree& treeWhosePropertyHasChanged, const Identifier& property) override
    {
        // Detune parameters
        if (treeWhosePropertyHasChanged == oscParams.getChildWithName(IDs::DETUNE))
        {
            updateFrequency();
        }
    }

private:
    // The value tree containing parameters for this oscillator
    ValueTree oscParams;

    // Oscillator index. Stored locally for consistency
    int oscNumber;

    // Current sample rate to be played at. Stored locally for efficiency
    double currentSampleRate = -1.0;

    // =======================
    // ====== WAVETABLE ======
    // =======================
    std::shared_ptr<AudioBuffer<float>> oscWavetable;
    int tableSize = 0;
    float currentIndex = 0.0f;


    // ===========================
    // ====== NOTE & DETUNE ======
    // ===========================
    // The note being played.
    int currentNote = -1;

    // 12st per step
    CachedValue<int> octaveDetuneSteps;
    // 1st per step
    CachedValue<int> coarseDetuneSteps;
    // 1/100st per step
    CachedValue<int> fineDetuneSteps;

    // The frequency being played. This will be updated based on currentNote & detune
    double currentFrequency = -1.0;

    // The rate through which the wavetable will be stepped to produce samples.
    // This will be updated when any changes to frequency or sample rate occur.
    float tableDelta = 0.0;

    //==============================================================================
    /** Called to initialise cached detune values.
    
    */
    void initDetune()
    {
        ValueTree& detune = oscParams.getChildWithName(IDs::DETUNE);

        octaveDetuneSteps.referTo(detune, IDs::detuneOctave, nullptr);
        coarseDetuneSteps.referTo(detune, IDs::detuneCoarse, nullptr);
        fineDetuneSteps.referTo(detune, IDs::detuneFine, nullptr);

        if (currentNote != -1)
        {
            updateFrequency();
        }
    }

    //==============================================================================
    /** Updates or resets currentFrequency based on the note being played & the oscillator's detune settings.

        Calls updateTableDelta to propogate frequency update.
    */
    void updateFrequency()
    {
        // Reset frequency
        if (currentNote == -1)
        {
            currentFrequency = -1.0;
        }
        // Calculate frequency from note
        else
        {
            // Adjust note by octave and coarse detunes
            int coarseAdjustedNote = currentNote + (octaveDetuneSteps * 12) + coarseDetuneSteps;

            // Get frequency for coarse adjusted note
            double noteFreq = MidiMessage::getMidiNoteInHertz(coarseAdjustedNote);

            // TODO pitch by cents for fine adjustment

            currentFrequency = noteFreq;
        }

        updateTableDelta();
    }

    /** Updates or resets the tableDelta given the current sample rate and frequency.
        
        Called by setSampleRate and setFrequency.
    */
    void updateTableDelta()
    {
        // Reset delta
        if (currentSampleRate == -1.0 || currentFrequency == -1.0)
        {
            tableDelta = 0.0;
        }
        // Calculate new delta
        else
        {
            double tableSizeOverSampleRate = (double)tableSize / currentSampleRate;
            tableDelta = currentFrequency * tableSizeOverSampleRate;
        }
    }


    //==============================================================================
};