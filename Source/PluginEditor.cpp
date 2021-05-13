/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginEditor.h"

#include "LEAFLink.h"



VoxWorldAudioProcessorEditor::VoxWorldAudioProcessorEditor(VoxWorldAudioProcessor& processor):
AudioProcessorEditor (processor)
{
    setSize(width, height);
    startTimerHz(60);
    
    // initialize eq path
    initializeEQPath(40);
}

VoxWorldAudioProcessorEditor::~VoxWorldAudioProcessorEditor()
{
    
}

void VoxWorldAudioProcessorEditor::timerCallback(void)
{
    repaint();
}

void VoxWorldAudioProcessorEditor::mouseDown (const MouseEvent& event)
{
    mouseIsDown = true;
    
    if (event.y < (getHeight()*0.65))
    {
        dots.add(new SkyDot(event.x, event.y));
        SkyDot* dot = dots.getLast();
        
        dot->setCentrePosition(event.x, event.y);
        addAndMakeVisible(dot);

        for (int i = 0; i < 2; i++)
        {
            float fb = VoxWorld_getDelayFeedback(i)*1.2;
            if (fb > 0.95) fb = 0.95;
            DBG("feedback " + String(i) + ": " + String(fb));
            VoxWorld_setDelayFeedback(i,fb);
            
            float time = VoxWorld_getDelayTime(i)*1.05;
            if (time > 1.0) time = 1.0;
            DBG("time " + String(i) + ": " + String(time));
            VoxWorld_setDelayTime(i,fb);
        }
        
    }
   
}

void VoxWorldAudioProcessorEditor::mouseUp (const MouseEvent& event)
{
    mouseIsDown = false;
}

void VoxWorldAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (Colours::slategrey);   // clear the background
    float rms = getProcessor().getInputRMS();
    for (auto dot : dots)
    {
        dot->setShake(rms);
        dot->repaint();
    }
    
    auto position = getMouseXYRelative();
    if (mouseIsDown && position.y > (getHeight()*0.65))
    {
        Point<float> mousePosition = {
            static_cast<float>(position.getX()),
            static_cast<float>(position.getY()) };

        addToEQPath(mousePosition);
    }
    
    Path myPath;
    myPath.startNewSubPath(Point<float>(0,eqPath.front().getY()));
   
    for (int i = 0; i < eqPath.size(); i++)
    {
        myPath.lineTo(eqPath[i].x,
                      eqPath[i].y+(rms*(getRandomFloat()-0.5)*100));
    }
    
    myPath.lineTo(Point<float>(getWidth(),eqPath.back().getY()));
    
    g.strokePath(myPath, PathStrokeType(15,
                PathStrokeType::JointStyle::curved,
                PathStrokeType::EndCapStyle::rounded));
}

void VoxWorldAudioProcessorEditor::resized()
{

}


// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ EQ PATH STUFF ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
void VoxWorldAudioProcessorEditor::initializeEQPath(int size)
{
    eqPath.resize(size);
    for (int i = 0; i < eqPath.size(); i++)
    {
        eqPath[i] = Point<float>(i*getWidth()/eqPath.size(), getHeight()*0.8);
    }
}

void VoxWorldAudioProcessorEditor::addToEQPath(const Point<float>& point)
{
    int index = point.getX()/getWidth() * eqPath.size();
    index = LEAF_clipInt(0, index, eqPath.size()-1);
    
    eqPath[index] = point;
    
    int group = (index/2);
    float avg = eqPath[group*2].y;
    float diff = avg-getHeight()*0.8;
    float linearGain = 0.0;
    float dB = 0.0;
    if (diff < 0)
    {
        linearGain = fabs(diff) / (getHeight()*0.15);
        dB = 12.0 * linearGain;
    }
    else
    {
        linearGain = -diff/(getHeight()*0.20);
        dB = 24.0 * linearGain;
    }
    
    DBG("band: " +String(group) + " gain: " + String(dB));
    getProcessor().getEQ()->setBand(group, dB);
}
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~




