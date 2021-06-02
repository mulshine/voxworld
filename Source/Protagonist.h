/*
  ==============================================================================

    Character.h
    Created: 31 May 2021 10:22:25pm
    Author:  airship

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================

class Protagonist:
public Component
{
public:
    Protagonist();

    ~Protagonist();

    void paint (Graphics& g) override;

    void resized() override;
    
    inline void setShake(float sh) { shake = sh;};
    inline float getShake(void) {return shake;}

    void clone(void);
    void declone(void);
    void setNumClones(int newNumClones);
    
    void setLeft(void);
    void setRight(void);
    
    void mouseDown(const MouseEvent& e) override;
    void mouseDrag(const MouseEvent& e) override;
    
    float getCenterBoxSize(void);
private:
    
    float shake = 0.0;
    int dir = 1;
    float phasor = 0.0;
    float width,height;
    float X{0};
    float Y{0};
    float size{100};
    Colour fill;
    
    int numClones = 0;
    
    ComponentDragger dragger;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Protagonist)
};


