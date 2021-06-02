/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#ifndef PLUGINEDITOR_H_INCLUDED
#define PLUGINEDITOR_H_INCLUDED

#include "PluginProcessor.h"

#include "SkyDot.h"
#include "Protagonist.h"
#include "Platform.h"

//==============================================================================
/**
*/
#define LEFT_KEY 0
#define RIGHT_KEY 1
#define UP_KEY 2
#define DOWN_KEY 3
class VoxWorldAudioProcessorEditor  : public AudioProcessorEditor, public KeyListener, private Timer
{
public:
    VoxWorldAudioProcessorEditor (VoxWorldAudioProcessor&, ValueTree&);
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
    
    bool keyPressed (const KeyPress& key, Component* originatingComponent) override;
    bool keyStateChanged (bool isKeyDown, Component* originatingComponent) override;
    
    VoxWorldAudioProcessor& getProcessor() const
    {
        return static_cast<VoxWorldAudioProcessor&> (processor);
    }
    
    ValueTree& state;
    
    inline float getX() {return X;};
    inline float getY() {return Y;};
    
    int width{1024};
    int height{768};
    
    float X,Y;
    
    OwnedArray<SkyDot> dots;
    OwnedArray<Platform> platforms;
    std::vector<bool> platformMoving;
    Protagonist mike;
    float mikeY = 0.0;
    float mikeX = 0.0;
    
    bool mouseIsDown{false};
    bool shiftIsDown{false};
    bool arrowKeyDown[4];
    int moveX{0};
    int moveY{0};
    
    Point<int> mouseStart;
    Point<int> mouseEnd;
    
    std::vector<Point<float>> eqPath;

    int autotuneGridNum = 12;
    OwnedArray<Rectangle<int>> autotuneGrid;
    
    ColourGradient gradientY;
    
    
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (VoxWorldAudioProcessorEditor)
};


#endif  // PLUGINEDITOR_H_INCLUDED
