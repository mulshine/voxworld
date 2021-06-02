/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginEditor.h"

#include "LEAFLink.h"	


VoxWorldAudioProcessorEditor::VoxWorldAudioProcessorEditor(VoxWorldAudioProcessor& processor, ValueTree& params):
AudioProcessorEditor (processor),
state(params)
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
    
    bool onPlatform = false;
    for (auto platform : platforms)
    {
        int which = platforms.indexOf(platform);
        if (!platform->isMouseOverOrDragging())
        {
            platform->setTopLeftPosition(VoxWorld_getPlatformX(which), VoxWorld_getPlatformY(which));
        }
        
        // detect if on platform
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
    
    // Y processing
    float newY = mike.getY() + (onPlatform ? 0.0 : ((height - mike.getY()) * 0.004)); // better model for gravity (currently slew-based)
    newY = LEAF_clip(-mike.getHeight()*0.5, newY+((moveY>0)?-5:(!onPlatform && (moveY<0)?5:0)), height-mike.getHeight()*0.5);
    mikeY = (height-(newY+mike.getHeight()*0.5))/height;
    VoxWorld_setY(mikeY);
    
    // X processing
    float newX = LEAF_clip(-mike.getWidth()*0.5, mike.getX()+((moveX>0)?5:((moveX<0)?-5:0)), width-mike.getWidth()*0.5);
    mikeX = (newX+mike.getWidth()*0.5)/getWidth();
    VoxWorld_setX(mikeX);

    mike.setTopLeftPosition(newX, newY);
    
    if (moveX>0) mike.setRight();
    else if (moveX<0) mike.setLeft();
    
    mike.setNumClones(VoxWorld_getNumClones());
    
    int numDots = VoxWorld_getNumSkyDot();
    
    if (dots.size() < LEAF_clip(0, numDots, 25))
    {
        for (int i = 0; i < numDots-dots.size(); i++)
        {
            int x = getRandomFloat()*width, y = getRandomFloat()*height;
            dots.add(new SkyDot(x, y));
            SkyDot* dot = dots.getLast();
            
            dot->setCentrePosition(x, y);
            addAndMakeVisible(dot);
        
            dot->toBehind(&mike);
        }
    }
    
    int numPlatforms = VoxWorld_getNumPlatforms();
    
    if (platforms.size() != LEAF_clip(0, numPlatforms, MAX_NUM_PLATFORMS))
    {
        platforms.clear();
        for (int i = 0; i < numPlatforms; i++)
        {
            platforms.add(new Platform());
            Platform* newPlatform = platforms.getLast();
            
            newPlatform->setBounds(VoxWorld_getPlatformX(i), VoxWorld_getPlatformY(i), (width/8.0), (width/8.0)*0.2);
            newPlatform->addMouseListener(this, true);
            newPlatform->toFront(false);
            
            addAndMakeVisible(newPlatform);
        }
    }
    
        
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
                VoxWorld_declone();
            }
            else
            {
                VoxWorld_clone();
            }
            
            state.setProperty("numclones", VoxWorld_getNumClones(), nullptr);
        }
    }
    else if (platforms.contains((Platform*)event.originalComponent))
    {
        int which = platforms.indexOf((Platform*)event.originalComponent);
        
        if (ModifierKeys::getCurrentModifiers().isShiftDown())
        {
            VoxWorld_removePlatform(which);
            platforms.remove(which);
            
            VoxWorld_setNumPlatforms(platforms.size());
            
            for (int i = 0 ; i < MAX_NUM_PLATFORMS; i++)
            {
                state.removeProperty("platformX_"+String(i), nullptr);
                state.removeProperty("platformY_"+String(i), nullptr);
                float x = VoxWorld_getPlatformX(i);
                float y = VoxWorld_getPlatformY(i);
                if ((x >= 0.0) && (y >= 0.0))
                {
                    state.setProperty("platformX_"+String(i), x, nullptr);
                    state.setProperty("platformY_"+String(i), y, nullptr);
                }
            }
        }
        else
        {
            Platform* platform = platforms[which];
            VoxWorld_setPlatformX(which, platform->getX());
            VoxWorld_setPlatformY(which, platform->getY());
            
            state.setProperty("platformX_"+String(which), VoxWorld_getPlatformX(which), nullptr);
            state.setProperty("platformY_"+String(which), VoxWorld_getPlatformY(which), nullptr);
        }
        
        VoxWorld_setNumPlatforms(platforms.size());
        state.setProperty("numplatforms", VoxWorld_getNumPlatforms(), nullptr);
        
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
                }
            }
            
            VoxWorld_setNumSkyDot(dots.size());
            state.setProperty("numskydot", VoxWorld_getNumSkyDot(), nullptr);
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
    else if (keyCode == KeyPress::spaceKey)
    {
        platforms.add(new Platform());
        Platform* newPlatform = platforms.getLast();
        newPlatform->setBounds(width*0.5-(width/8.0)*0.5, height*0.5, (width/8.0), (width/8.0)*0.2);
        newPlatform->addMouseListener(this, true);
        newPlatform->toFront(false);
        
        VoxWorld_setNumPlatforms(platforms.size());
        
        int which = platforms.indexOf(newPlatform);
        VoxWorld_setPlatformX(which, newPlatform->getX());
        VoxWorld_setPlatformY(which, newPlatform->getY());
        
        
        addAndMakeVisible(newPlatform);
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




