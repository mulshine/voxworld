/*
  ==============================================================================

    Platform.h
    Created: 1 Jun 2021 10:07:32pm
    Author:  airship

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================

class Platform:
public Component
{
public:
    Platform();

    ~Platform();

    void paint (Graphics& g) override;

    void resized() override;
    
    void mouseDown(const MouseEvent& e) override;
    void mouseDrag(const MouseEvent& e) override;
    
private:
    
    float width,height;
    float size{100};
    
    Image platformImage;
    
    ComponentDragger dragger;
    

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Platform)
};
