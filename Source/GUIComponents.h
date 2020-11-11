/*
  ==============================================================================

    GUIComponents.h
    Created: 13 Apr 2020 10:50:03pm
    Author:  Sam

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "Common.h"


/** TODO LIST

    - 



*/


/** The window which contains the content of the UI.

*/
class MainWindow : public DocumentWindow
{
public:
    MainWindow(const String& name, Component* c)
        : DocumentWindow(name, Desktop::getInstance().getDefaultLookAndFeel()
                                                     .findColour(ResizableWindow::backgroundColourId),
                               DocumentWindow::allButtons)
    {
        setUsingNativeTitleBar(true);
        setContentOwned(c, true);


        setResizable(true, false);
        setResizeLimits(300, 250, 10000, 10000);
        centreWithSize(getWidth(), getHeight());

        setVisible(true);
    }

    ~MainWindow()
    {

    }

private:

    MainContentComponent mainComponent;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
};


/** The main content component which contains all other components as children.

*/
class MainContentComponent : public Component
{
public:
    MainContentComponent()
    {

    }

    MainContentComponent(ValueTree mainParameterTree)
    {

    }

    ~MainContentComponent()
    {

    }

private:
    OscillatorsWindow oscillators;
    EnvelopesWindow envelopes;
    FiltersWindow filters;

};


/** The UI window containing all OscillatorComponents.

*/
class OscillatorsWindow : public Component
{
public:

    OscillatorsWindow()
    {

    }

    ~OscillatorsWindow()
    {

    }

    void paint(Graphics&) override
    {

    }

    void resized() override
    {
        auto area = getLocalBounds();


    }

    void updateValueTree(ValueTree oscMgrParams)
    {
        // Get number of oscillators from parameter tree
        int numOsc = params.getChildWithName(IDs::OSC_GROUP).getNumChildren();

        // TODO arrange oscillator components


        // For now, a single oscillator occupies the window


    }

private:
    ValueTree params;

    Label oscLabel;
    Viewport oscGroupView;
    OscillatorGroupComponent oscillators;
};

class OscillatorGroupComponent : public Component,
    public ValueTree::Listener
{
public:
    OscillatorGroupComponent()
    {

    }

    ~OscillatorGroupComponent()
    {

    }

    void updateValueTree(ValueTree oscParameters)
    {
        params = oscParameters;
    }

    void valueTreeChildAdded(ValueTree& parentTree, ValueTree& childWhichHasBeenAdded)
    {

    }

private:
    ValueTree params;

    std::vector<OscillatorComponent> oscComps;
};

/** The UI component displaying a single oscillator and its parameters.

*/
class OscillatorComponent : public Component,
                            public ValueTree::Listener
{
public:
    OscillatorComponent()
    {

    }


    ~OscillatorComponent()
    {

    }

    void paint(Graphics&) override
    {

    }

    void resized() override
    {
        auto area = getLocalBounds();
    }



    //==============================================================================

    void setValueTree(ValueTree oscParameters)
    {
        params = oscParameters;

        // Get index of oscillator
        ValueTree oscGroup = params.getParent();
        oscNumber = oscGroup.indexOf(params) + 1;

        // Add combobox items
        waveType.addItem("Sine", 1);
        waveType.addItem("Saw", 2);
        waveType.addItem("Ramp", 3);
        waveType.addItem("Triangle", 4);
        waveType.addItem("Square", 5);

        noiseType.addItem("White", 1);
        noiseType.addItem("Grey", 2);
        noiseType.addItem("Blue", 3);


        params.addListener(this);
    }


    //==============================================================================
    void valueTreePropertyChanged(ValueTree& treeWhosePropertyHasChanged, const Identifier& property) override
    {

    }

private:
    ValueTree params;

    int oscNumber;

    WavetableComponent wavetable;

    ComboBox waveType;

    ComboBox noiseType;
    Slider noise;
};

/** The UI visualisation of a wavetable

*/
class WavetableComponent : public Component
{
public:
    WavetableComponent()
    {
        setSize(100, 70);
    }

    ~WavetableComponent()
    {
        wavetable.reset();
    }

    void paint(Graphics& g) override
    {
        if (wavetable)
        {
            DBG("WavetableComponent.Paint: TABLE EXISTS");
            int h = getHeight();
            int w = getWidth();

            // Vertical center
            int centerV = h / 2;

            int numSamples = wavetable->getNumSamples();
            auto tableRead = wavetable->getArrayOfReadPointers()[0];

            // Size of horizontal step between samples
            int stepSize = w / numSamples;

            // Coordinates to draw lines between
            int x1, x2;
            int y1, y2;

            // Draw lines between every inc sample(s)
            g.setColour(Colours::black);
            int inc = 1;
            for (int i = 0; i < numSamples - inc; i += inc)
            {
                int yMaxDeviation = centerV - (h / 10);

                if (i == 0)
                {
                    y1 = centerV + (tableRead[i] * -1 * yMaxDeviation);
                    y2 = centerV + (tableRead[i + inc] * -1 * yMaxDeviation);

                    x1 = 0;
                    x2 = stepSize * inc;
                }
                // Use stored previous coordinates in subsequent iterations
                else
                {
                    y2 = centerV + (tableRead[i + inc] * -1 * yMaxDeviation);
                    x2 = x1 + (stepSize * inc);
                }

                // Draw line
                g.drawLine(x1, x2, y1, y2, 1);

                // Store previous coordinates
                y1 = y2;
                x1 = x2;
            }
        }
        else
        {
            DBG("WavetableComponent.Paint: TABLE DOES NOT EXIST");
        }
        
    }

    void resized() override
    {
        repaint();
    }

    void updateWavetable(std::shared_ptr<AudioBuffer<float>> newTable)
    {
        wavetable.reset();
        wavetable = newTable;

        repaint();
    }

private:
    std::shared_ptr<AudioBuffer<float>> wavetable;
};

// =======================
// ====== ENVELOPES ======
// =======================

/** The UI component displaying a single envelope and its parameters.

*/
class EnvelopeComponent : public Component,
                          public ValueTree::Listener
{
public:
    EnvelopeComponent(ValueTree envParameters)
        : params (envParameters)
    {
        params.addListener(this);
    }

    ~EnvelopeComponent()
    {

    }

    void paint(Graphics&) override
    {

    }

    void resized() override
    {

    }

private:
    ValueTree params;

    Slider attackSlider;
    Slider decaySlider;
    Slider sustainSlider;
    Slider releaseSlider;
};

/** The UI window containing all EnvelopeComponents

*/
class EnvelopesWindow : public Component
{
public:
    EnvelopesWindow()
    {

    }
    ~EnvelopesWindow()
    {

    }

    void paint(Graphics&) override
    {

    }

    void resized() override
    {

    }

private:

};


// TODO
// =====================
// ====== FILTERS ======
// =====================

/** The UI component displaying a single filter and its parameters.

*/
class FilterComponent : public Component
{
public:
    FilterComponent();
    ~FilterComponent();

    void paint(Graphics&) override;
    void resized() override;

private:

};

/** The UI window containing all FilterComponents.

*/
class FiltersWindow : public Component
{
public:
    FiltersWindow();
    ~FiltersWindow();

    void paint(Graphics&) override;
    void resized() override;

private:

};

// TODO
// ==================
// ====== LFOs ======
// ==================

/** The UI component displaying a single LFO and its parameters.

*/
class LFOComponent : public Component
{
public:
    LFOComponent();
    ~LFOComponent();

    void paint(Graphics&) override;
    void resized() override;

private:

};

/** The UI window containing all LFOComponents.

*/
class LFOsWindow : public Component
{
public:
    LFOsWindow();
    ~LFOsWindow();

    void paint(Graphics&) override;
    void resized() override;

private:

};

