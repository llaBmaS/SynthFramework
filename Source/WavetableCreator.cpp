/*
  ==============================================================================

    WavetableCreator.cpp
    Created: 28 Mar 2020 4:52:40pm
    Author:  Sam

  ==============================================================================
*/

#include <JuceHeader.h>
#include "WavetableCreator.h"

std::shared_ptr<AudioBuffer<float>> WavetableCreator::createSineTable(const unsigned int tableSize)
{
    // Instantiate new buffer contained in a shared pointer
    std::shared_ptr<AudioBuffer<float>> table = std::make_shared<AudioBuffer<float>>(1, tableSize + 1);


    auto* samples = table->getWritePointer(0);

    auto angleDelta = MathConstants<double>::twoPi / (double)(tableSize - 1);
    auto currentAngle = 0.0;

    for (auto i = 0; i < tableSize; ++i)
    {
        auto sample = std::sin(currentAngle);
        samples[i] = (float) sample;
        currentAngle += angleDelta;
    }

    // Wraparound: last sample is equal to first
    samples[tableSize] = samples[0];

    return std::move(table);
}

std::shared_ptr<AudioBuffer<float>> WavetableCreator::createSawTable(const unsigned int tableSize)
{
    // Instantiate new buffer contained in a shared pointer
    std::shared_ptr<AudioBuffer<float>> table = std::make_shared<AudioBuffer<float>>(1, tableSize + 1);


    return std::move(table);
}

std::shared_ptr<AudioBuffer<float>> WavetableCreator::createRampTable(const unsigned int tableSize)
{
    // Instantiate new buffer contained in a shared pointer
    std::shared_ptr<AudioBuffer<float>> table = std::make_shared<AudioBuffer<float>>(1, tableSize + 1);


    return std::move(table);
}

std::shared_ptr<AudioBuffer<float>> WavetableCreator::createTriangleTable(const unsigned int tableSize)
{
    // Instantiate new buffer contained in a shared pointer
    std::shared_ptr<AudioBuffer<float>> table = std::make_shared<AudioBuffer<float>>(1, tableSize + 1);


    return std::move(table);
}

std::shared_ptr<AudioBuffer<float>> WavetableCreator::createSquareTable(const unsigned int tableSize)
{
    // Instantiate new buffer contained in a shared pointer
    std::shared_ptr<AudioBuffer<float>> table = std::make_shared<AudioBuffer<float>>(1, tableSize + 1);

    return std::move(table);
}