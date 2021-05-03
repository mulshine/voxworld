/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#ifndef PLUGINEDITOR_H_INCLUDED
#define PLUGINEDITOR_H_INCLUDED

#include "PluginProcessor.h"

#include "UIComponent.h"

//==============================================================================
/**
*/
class VoxWorldAudioProcessorEditor  : public AudioProcessorEditor, private Timer
{
public:
    VoxWorldAudioProcessorEditor (VoxWorldAudioProcessor&);
    ~VoxWorldAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;
    
    void initializeEQPath(int size);
    void addToEQPath(const Point<float>& point);

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    
    void timerCallback(void) override;
    
    void mouseDown (const MouseEvent& event) override;
    void mouseUp (const MouseEvent& event) override;
    
    VoxWorldAudioProcessor& getProcessor() const
    {
        return static_cast<VoxWorldAudioProcessor&> (processor);
    }
    
    inline float getX() {return X;};
    inline float getY() {return Y;};
    
    int width{1024};
    int height{768};
    
    float X,Y;
    
    OwnedArray<SkyDot> dots;
    
    bool mouseIsDown{false};
    
    
    std::vector<Point<float>> eqPath;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VoxWorldAudioProcessorEditor)
};


#endif  // PLUGINEDITOR_H_INCLUDED
