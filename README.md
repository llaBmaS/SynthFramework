SynthFramework

This is a personal project written in C++ and using the JUCE audio framework, for the purposes of development practice and to continue developing in the future with an expanded featureset.

In its current state, the application does not compile because the User Interface is incomplete. Prior to UI development, the application was in working condition with the following featureset:
- Arbitrary number of oscillators at a time; resizes data structures dynamically
- Able to produce sound from any valid wavetable (currently only hardcoded with a sine wave for testing purposes)
- ADSR gain envelope functionality (with plans to implement individual envelopes for each oscillator)
- Monophonic or polyphonic (designed to handle note stealing with minimal space-complexity)

The most important files of the project, that most showcase my development skills and practices, are WavetableOscillator.h, WavetableOscillatorManager.h, and SynthVoice.h, which handle the production of sound given the current parameters of the program. These parameters are stored in a central "ValueTree", a data structure similar to XML format, and relevant classes extend listeners so they may automatically update when parameters are changed and translate these changes into changes in the sound produced.


Future development plans:
- 'Noise' knob for each osillator, which interpolates between the selected wavetable (0) and white noise (1). The goal of this is to be able to add any amount of grain and texture to a produced sound while retaining its original tonal qualities.
- Pulse width modulation for oscillators, to modulate the shape of the wavetable.
- Dynamic application of envelopes and filters, to allow for endless customization. (ie envelope 1 applies to oscillators 2, 4, 6 and filters 1, 3)
- Low Frequency Oscillators (LFOs) to apply modulation to filters and gain much like envelopes
- Wavetable editing and saving, to allow for even more possibilities than base wave types like sine, square, triangle, etc
