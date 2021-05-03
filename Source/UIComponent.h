/*
  ==============================================================================

    UIComponent.h
    Created: 5 Dec 2016 2:23:38pm
    Author:  Michael R Mulshine

  ==============================================================================
*/

#ifndef UICOMPONENT_H_INCLUDED
#define UICOMPONENT_H_INCLUDED

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



class UIComponent:
public Component,
public Slider::Listener,
public TextButton::Listener,
public ComboBox::Listener,
public TextEditor::Listener,
private Timer
{
public:
    UIComponent();

    ~UIComponent();

    void paint (Graphics& g) override;

    void resized() override;
    
    void sliderValueChanged(Slider* s) override;
    
    void buttonClicked(Button*b) override;
    void buttonStateChanged(Button* b) override;
    
    void textEditorTextChanged (TextEditor&) override;
    
    void comboBoxChanged (ComboBox* cb) override;
    
    void timerCallback() override;
    
private:
    
    static const int cLeftOffset = 30;
    static const int cTopOffset = 30;
    
    static const int cXSpacing = 60;
    static const int cYSpacing = 5;
    
    static const int cSliderHeight = 200;
    static const int cSliderWidth = 20;
    
    static const int cLabelHeight = 20;
    static const int cLabelWidth  = cSliderWidth + cXSpacing;
    
    static const int cButtonHeight = 30;
    static const int cButtonWidth  = 75;
    
    static const int cBoxHeight = 20;
    static const int cBoxWidth  =  100;
    
    OwnedArray<Slider>      sliders;
    OwnedArray<Label>       sliderLabels;
    OwnedArray<TextButton>  buttons;
    OwnedArray<TextEditor>  textFields;
    OwnedArray<ComboBox>    comboBoxes;
    
    juce::AudioFormatManager formatManager;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UIComponent)
};


#endif  // UICOMPONENT_H_INCLUDED
