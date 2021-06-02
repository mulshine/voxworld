/*
  ==============================================================================

    Platform.cpp
    Created: 1 Jun 2021 10:07:32pm
    Author:  airship

  ==============================================================================
*/

#include "Platform.h"
#include "LEAFLink.h"

Platform::Platform()
{
    platformImage = ImageFileFormat::loadFrom(BinaryData::platform_png, BinaryData::platform_pngSize);
}

Platform::~Platform()
{
    
}

void Platform::paint (Graphics& g)
{
    g.drawImage(platformImage, getLocalBounds().toFloat());
    
}

void Platform::resized()
{
    
}

void Platform::mouseDown(const MouseEvent& e)
{
    dragger.startDraggingComponent(this, e);
}

void Platform::mouseDrag(const MouseEvent& e)
{
    dragger.dragComponent(this, e, nullptr);
}
