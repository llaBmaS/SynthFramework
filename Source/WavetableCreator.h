/*
  ==============================================================================

    WavetableCreator.h
    Created: 28 Mar 2020 4:52:40pm
    Author:  Sam

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

// Class for creating wavetables for base oscillator types
class WavetableCreator
{
public:
    // =====================================
    // ====== BASE WAVETABLE CREATION ======
    // =====================================
    /** Creates a sine wavetable stored in an AudioBuffer and returns a shared pointer to it.
    
        The pointer is returned via move, so it must be assigned to another shared_ptr.
    */
    static std::shared_ptr<AudioBuffer<float>> createSineTable(const unsigned int tableSize);

    /** Creates a saw wavetable stored in an AudioBuffer and returns a shared pointer to it.

        The pointer is returned via move, so it must be assigned to another shared_ptr.
    */
    static std::shared_ptr<AudioBuffer<float>> createSawTable(const unsigned int tableSize);

    /** Creates a ramp wavetable stored in an AudioBuffer and returns a shared pointer to it.

        The pointer is returned via move, so it must be assigned to another shared_ptr.
    */
    static std::shared_ptr<AudioBuffer<float>> createRampTable(const unsigned int tableSize);

    /** Creates a triangle wavetable stored in an AudioBuffer and returns a shared pointer to it.

        The pointer is returned via move, so it must be assigned to another shared_ptr.
    */
    static std::shared_ptr<AudioBuffer<float>> createTriangleTable(const unsigned int tableSize);

    /** Creates a square wavetable stored in an AudioBuffer and returns a shared pointer to it.

        The pointer is returned via move, so it must be assigned to another shared_ptr.
    */
    static std::shared_ptr<AudioBuffer<float>> createSquareTable(const unsigned int tableSize);
};