/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#ifndef PLUGINPROCESSOR_H_INCLUDED
#define PLUGINPROCESSOR_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"

#include "VoxWorld.h"
#include "ConstantQEqualizer.hpp"

//==============================================================================
/**
*/
class VoxWorldAudioProcessor  : public AudioProcessor
{
public:
    //==============================================================================
    VoxWorldAudioProcessor();
    ~VoxWorldAudioProcessor();
    
    inline float getInputRMS(void) {return rms;}
    
    inline void VW_setDelayFeedback(int chan, float fb) { VoxWorld_setDelayFeedback(chan, fb);};
    inline void VW_setDelayTime(int chan, float time) { VoxWorld_setDelayTime(chan, time);};
    
    inline float VW_getDelayFeedback(int chan) { return VoxWorld_getDelayFeedback(chan); }
    inline float VW_getDelayTime(int chan) { return VoxWorld_getDelayTime(chan); }

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (AudioSampleBuffer&, MidiBuffer&) override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
   const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
   const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
    
    ConstantQEqualizer* eq;

    ConstantQEqualizer* getEQ(void){return eq;};
private:
    float rms;
    uint64 timer;
    float gain;
    bool start,ramp;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VoxWorldAudioProcessor)
};


#endif  // PLUGINPROCESSOR_H_INCLUDED
