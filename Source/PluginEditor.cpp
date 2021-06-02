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
    
    addKeyListener(this);
    setWantsKeyboardFocus(true);

    setInterceptsMouseClicks(true, false);
    
    // initialize eq path
    initializeEQPath(40);
    
    arrowKeyDown[LEFT_KEY] = false;
    arrowKeyDown[RIGHT_KEY] = false;
    arrowKeyDown[UP_KEY] = false;
    arrowKeyDown[DOWN_KEY] = false;
    
    
    mike.addMouseListener(this, true);
    
    addAndMakeVisible(mike);
    mike.setTopLeftPosition(0, height-mike.getHeight());
    
    for (int i = 0; i < 1; i++)
    {
        platforms.add(new Platform());
        Platform* newPlatform = platforms.getLast();
        newPlatform->setBounds(width*0.5, height*0.5, (width/8.0), (width/8.0)*0.2);
        newPlatform->addMouseListener(this, true);
        newPlatform->toFront(false);
        
        addAndMakeVisible(newPlatform);
    }
    
    gradientY.addColour(0.0, Colour(0, 0, 0));
    gradientY.addColour(0.61, Colour(63, 182, 241));
    gradientY.addColour(1.0, Colour(244,200,216));
    
    
    
}

VoxWorldAudioProcessorEditor::~VoxWorldAudioProcessorEditor()
{
    
}

void VoxWorldAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (gradientY.getColourAtPosition(mikeY));   // clear the background
    float rms = getProcessor().getInputRMS();
    for (auto dot : dots)
    {
        dot->setShake(rms);
        int newX = (int)((dot->getX() - 2 * getRandomFloat()*1)+getWidth()) % getWidth();
        int newY = (int)((dot->getY() + 3 * getRandomFloat()*1)+getHeight()*0.5) % (int)(getHeight()*0.5);
        
        dot->setTopLeftPosition(newX, newY);
        dot->repaint();
    }
    
    mike.setShake(rms);
    mike.repaint();
    
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
    
    g.setColour(Colour::fromRGBA(100, 100, 100, 150));
    g.strokePath(myPath, PathStrokeType(15,
                PathStrokeType::JointStyle::curved,
                PathStrokeType::EndCapStyle::rounded));
}

void VoxWorldAudioProcessorEditor::resized()
{
    
}

void VoxWorldAudioProcessorEditor::timerCallback(void)
{
    if (!hasKeyboardFocus(false)) grabKeyboardFocus();
    
    // detect if on platform
    bool onPlatform = false;
    for (auto platform : platforms)
    {
        Rectangle<int> bounds = platform->getBounds();
        
        if (((mike.getX()+mike.getWidth()/2+mike.getCenterBoxSize()/2) >= bounds.getX()) &&
            ((mike.getX()+mike.getWidth()/2-mike.getCenterBoxSize()/2) <= (bounds.getX()+bounds.getWidth())))
        {
            if ((abs((mike.getY()+mike.getHeight()*0.5)-bounds.getY()) < 30))
            {
                onPlatform = true;
            }
        }
        
    }
    
    
    float newY = mike.getY() + (onPlatform ? 0.0 : ((height - mike.getY()) * 0.004)); // better model for gravity (currently slew-based)
    newY = LEAF_clip(-mike.getHeight()*0.5, newY+((moveY>0)?-5:(!onPlatform && (moveY<0)?5:0)), height-mike.getHeight()*0.5);
    
    
    mikeY = (height-(newY+mike.getHeight()*0.5))/height;
    VoxWorld_setY(mikeY);
    
    float newX = LEAF_clip(-mike.getWidth()*0.5, mike.getX()+((moveX>0)?5:((moveX<0)?-5:0)), width-mike.getWidth()*0.5);
    VoxWorld_setX((newX+mike.getWidth()*0.5)/getWidth());
    

    mike.setTopLeftPosition(newX, newY);
    
    
    if (moveX>0) mike.setRight();
    else if (moveX<0) mike.setLeft();
        
    repaint();
}

void VoxWorldAudioProcessorEditor::mouseDown (const MouseEvent& event)
{
    if (event.originalComponent == &mike)
    {
        DBG("mouse down on PRO");
    }
    else if (platforms.contains((Platform*)event.originalComponent))
    {
       
    }
    else
    {
        mouseIsDown = true;
    }
}

void VoxWorldAudioProcessorEditor::mouseUp (const MouseEvent& event)
{
    mouseIsDown = false;
    
    if (event.originalComponent == &mike)
    {
        if (event.getDistanceFromDragStart() < 50 )
        {
            DBG("mouse up on PRO");
            if (ModifierKeys::getCurrentModifiers().isShiftDown())
            {
                mike.declone();
                VoxWorld_declone();
            }
            else
            {
                mike.clone();
                VoxWorld_clone();
            }
        }
    }
    else
    {
        if (event.y < (getHeight()*0.65))
        {
            if (ModifierKeys::getCurrentModifiers().isShiftDown())
            {
                if (dots.size() > 0)
                {
                    removeChildComponent(dots.getLast());
                    dots.removeLast();
                    
                    VoxWorld_decreaseDelayFeedback();
                }
            }
            else
            {
                if (dots.size() < 20)
                {
                    dots.add(new SkyDot(event.x, event.y));
                    SkyDot* dot = dots.getLast();
                    
                    dot->setCentrePosition(event.x, event.y);
                    addAndMakeVisible(dot);
                
                    dot->toBehind(&mike);
                    VoxWorld_increaseDelayFeedback();
                }
            }
            
        }
    }
}


bool VoxWorldAudioProcessorEditor::keyPressed (const KeyPress& key, Component* const originatingComponent)
{
    int keyCode = key.getKeyCode();
    
    if (keyCode == KeyPress::rightKey)
    {
        moveX = 1;
        arrowKeyDown[RIGHT_KEY] = true;
    }
    else if (keyCode == KeyPress::leftKey)
    {
        moveX = -1;
        arrowKeyDown[LEFT_KEY] = true;
    }
    else if (keyCode == KeyPress::upKey)
    {
        moveY = 1;
        arrowKeyDown[UP_KEY] = true;
    }
    else if (keyCode == KeyPress::downKey)
    {
        moveY = -1;
        arrowKeyDown[DOWN_KEY] = true;
    }
    return false;
}

bool VoxWorldAudioProcessorEditor::keyStateChanged (bool isKeyDown, Component* originatingComponent)
{
    if (!isKeyDown)
    {
        if (!KeyPress::isKeyCurrentlyDown(KeyPress::rightKey))
        {
            if (!KeyPress::isKeyCurrentlyDown(KeyPress::leftKey))
            {
                moveX = 0;
                arrowKeyDown[LEFT_KEY] = false;
            }
            else
            {
                moveX = -1;
            }
            arrowKeyDown[RIGHT_KEY] = false;
        }
        
        if (!KeyPress::isKeyCurrentlyDown(KeyPress::leftKey))
        {
            if (!KeyPress::isKeyCurrentlyDown(KeyPress::rightKey))
            {
                moveX = 0;
                arrowKeyDown[RIGHT_KEY] = false;
            }
            else
            {
                moveX = 1;
            }
            arrowKeyDown[LEFT_KEY] = false;
        }
        
        if (!KeyPress::isKeyCurrentlyDown(KeyPress::upKey))
        {
            DBG("up released");
            if (!KeyPress::isKeyCurrentlyDown(KeyPress::downKey))
            {
                moveY = 0;
                arrowKeyDown[DOWN_KEY] = false;
            }
            else
            {
                moveY = -1;
            }
            arrowKeyDown[UP_KEY] = false;
        }
       
        if (!KeyPress::isKeyCurrentlyDown(KeyPress::downKey))
        {
            DBG("down released");
            if (!KeyPress::isKeyCurrentlyDown(KeyPress::upKey))
            {
                moveY = 0;
                arrowKeyDown[UP_KEY] = false;
            }
            else
            {
                moveY = 1;
            }
            arrowKeyDown[DOWN_KEY] = false;
        }
    }
    
    return false;
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




