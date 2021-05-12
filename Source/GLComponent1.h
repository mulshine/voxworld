/*
  ==============================================================================

    GLComponent1.h
    Created: 10 May 2021 11:53:35am
    Author:  airship

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class GLComponent1 : juce::OpenGLAppComponent
{
public:
    
    GLComponent1();
    ~GLComponent1();
    
    void resized() override;
    void render() override;
    
    void initialise() override;
    void shutdown() override;

private:
    
    std::unique_ptr<juce::OpenGLShaderProgram> shader;
    //std::unique_ptr<juce::Shape> shape;
    //std::unique_ptr<juce::Attributes> attributes;
    //std::unique_ptr<juce::Uniforms> uniforms;
     
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GLComponent1)
};
