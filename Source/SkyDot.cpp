/*
  ==============================================================================

    SkyDot.cpp
    Created: 10 May 2021 11:53:18am
    Author:  airship

  ==============================================================================
*/

#include "SkyDot.h"
#include "LEAFLink.h"

SkyDot::SkyDot(float startX, float startY):
X(startX),
Y(startY)
{
    size = 100.0 + (getRandomFloat()-0.5) * 50;
    fill = Colour(getRandomFloat()*256, getRandomFloat()*256, getRandomFloat()*256);
    setSize(size*2,size*2);
    width = getWidth();
    height = getHeight();
}

SkyDot::~SkyDot()
{
    
}


void SkyDot::paint (Graphics& g)
{
    g.fillAll(Colours::transparentWhite);
    
    size *= 0.99;
    if (size < 5) size = 95;
    
    g.setColour(fill);
    float shakeX = (getRandomFloat()-0.5)*shake*25;
    float shakeY = (getRandomFloat()-0.5)*shake*25;
    g.fillEllipse(width-width/2-size/2+shakeX,height-height/2-size/2+shakeY,size,size);
}

void SkyDot::resized()
{
    
}
