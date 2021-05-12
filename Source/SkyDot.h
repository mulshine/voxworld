/*
  ==============================================================================

    SkyDot.h
    Created: 10 May 2021 11:53:18am
    Author:  airship

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================

class SkyDot:
public Component/*,
private Timer*/
{
public:
    SkyDot(float startX, float startY);

    ~SkyDot();

    void paint (Graphics& g) override;

    void resized() override;
    
    inline void setShake(float sh) { shake = sh;};
    inline float getShake(void) {return shake;}
    
    //void timerCallback() override;
    
private:
    
    float shake = 0.0;
    float width,height;
    float X{0};
    float Y{0};
    float size{100};
    Colour fill;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SkyDot)
};


