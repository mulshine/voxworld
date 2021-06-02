/*
  ==============================================================================

    Character.cpp
    Created: 31 May 2021 10:22:25pm
    Author:  airship

  ==============================================================================
*/

#include "Protagonist.h"
#include "LEAFLink.h"

Protagonist::Protagonist()
{
    size = 80.0;
    fill = Colour::fromRGBA(138,43,226, 250);
    setSize(size*4,size*4);
    width = getWidth();
    height = getHeight();
    numClones = 0;
    phasor = 0.0;
    dir = 1;
    
    //setInterceptsMouseClicks(true, false);
}

Protagonist::~Protagonist()
{
    
}


void Protagonist::clone(void)
{
    numClones += 1;
    
    if (numClones > 6) numClones = 6;
    
    DBG("numClones: " + String(numClones));
}

void Protagonist::declone(void)
{
    numClones -= 1;
    
    if (numClones < 0) numClones = 0;
    
    DBG("numClones: " + String(numClones));
}

void Protagonist::setNumClones(int newNumClones)
{
    numClones = newNumClones;
}

void Protagonist::setRight(void)
{
    dir = -1;
}

void Protagonist::setLeft(void)
{
    dir = 1;
}

float Protagonist::getCenterBoxSize(void)
{
    return size;
}

void Protagonist::paint (Graphics& g)
{
    g.fillAll(Colours::transparentWhite);
    
    g.setColour(fill);
    float x = width-width/2-size/2;
    float y = height-height/2-size/2;

    float grow = shake*300;
    g.fillRoundedRectangle(x-grow*0.5,
                           y-grow*0.5,
                           size+grow,
                           size+grow,
                           12);
    
    g.setColour(Colours::lightgoldenrodyellow);
    
    float mouthW = size*0.2+shake*size*6;
    float mouthH = size*0.02+shake*size*7;
    g.fillEllipse(x+size/2-mouthW/2.0, y+size/2-mouthH/2.0, mouthW, mouthH);
    
    phasor += (0.01 + 0.79 * shake);
    
    if (phasor >= M_PI*1000) phasor = 0.0;
    
    
    float cloneSize = size*0.4*(1.0+shake);
    for (int i = 0; i < numClones; i++)
    {
        g.setColour(fill.withMultipliedAlpha(0.95));
        
        float cloneX = x + cos((float)i/numClones*2.0*M_PI+phasor*dir)*size*(1.0+shake)*1.25 + size/4;
        float cloneY = y + sin((float)i/numClones*2.0*M_PI+phasor*dir)*size*(1.0+shake)*1.25 + size/4;
        
        g.fillRoundedRectangle(cloneX,
                               cloneY,
                               cloneSize,
                               cloneSize,
                               6);
        
        
        // draw smile
        g.setColour(Colours::lightgoldenrodyellow.brighter().withMultipliedAlpha(0.9));
        
        mouthW = cloneSize*0.2+shake*cloneSize*6;
        mouthH = cloneSize*0.02+shake*cloneSize*7;
        g.fillEllipse(cloneX+cloneSize/2-mouthW/2.0, cloneY+cloneSize/2-mouthH/2.0, mouthW, mouthH);
    }
    
}

void Protagonist::resized()
{
    
}


void Protagonist::mouseDown(const MouseEvent& e)
{
    dragger.startDraggingComponent(this, e);
}

void Protagonist::mouseDrag(const MouseEvent& e)
{
    dragger.dragComponent(this, e, nullptr);
}

