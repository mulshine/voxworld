/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

#include "UIComponent.h"

//==============================================================================

VoxWorldAudioProcessor::VoxWorldAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
: AudioProcessor (BusesProperties()
#if ! JucePlugin_IsMidiEffect
#if ! JucePlugin_IsSynth
.withInput  ("Input",  AudioChannelSet::stereo(), true)
#endif
.withOutput ("Output", AudioChannelSet::stereo(), true)
#endif
          ),
parameters(ValueTree("VOXWORLD"))
#endif
{
    
}

VoxWorldAudioProcessor::~VoxWorldAudioProcessor()
{

}

//==============================================================================
void VoxWorldAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    gain = 0.0f;
    timer = 0;
    start = false;
    ramp = false;
    
    eq = new ConstantQEqualizer(sampleRate);
    
    deesser = new DeEsser(sampleRate);
    
    VoxWorld_init(sampleRate, samplesPerBlock);
}

//====================================BLOCK=======================================

void VoxWorldAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
   const int totalNumInputChannels  = getTotalNumInputChannels();
   const int totalNumOutputChannels = getTotalNumOutputChannels();
    
    for (int i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
    
    VoxWorld_block(buffer);
    
    MidiMessage m;
    for (MidiMessageMetadata metadata : midiMessages)
    {
        m = metadata.getMessage();
        int noteNumber = m.getNoteNumber();
        float velocity = m.getFloatVelocity();
        
        if (m.isNoteOn())
        {
            VoxWorld_noteOn(noteNumber, velocity);
        }
        else if (m.isNoteOff())
        {
            VoxWorld_noteOff(noteNumber);
        }
        else
        {
            
        }
    }
    rms = buffer.getRMSLevel(0, 0, buffer.getNumSamples());
    if (totalNumInputChannels == 2)
    {
        const float* inPointerL = buffer.getReadPointer (0);
        //const float* inPointerR = buffer.getReadPointer (1);
         
        float* outPointerL = buffer.getWritePointer( 0);
        float* outPointerR = buffer.getWritePointer( 1);

        
        eq->compute(buffer.getNumSamples(), inPointerL, outPointerL);
        //deesser->compute(buffer.getNumSamples(), inPointerL, outPointerL);

        for (int samp = 0; samp < buffer.getNumSamples(); ++samp) {
            float* tick = VoxWorld_tick( outPointerL[samp] );
            outPointerL[samp] = tick[0];
            outPointerR[samp] = tick[1];
        }
    }
    else if (totalNumInputChannels == 1)
    {
        const float* inPointerL = buffer.getReadPointer (0);
         
         float* outPointerL = buffer.getWritePointer( 0);
        
         eq->compute(buffer.getNumSamples(), inPointerL, outPointerL);
         
         for (int samp = 0; samp < buffer.getNumSamples(); ++samp) {
             float* tick = VoxWorld_tick( outPointerL[samp] );
             outPointerL[samp] = (tick[0]+tick[1]) * 0.5;
         }
    }
    else
    {
        
    }
   
}

//==============================================================================
const String VoxWorldAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool VoxWorldAudioProcessor::acceptsMidi() const
{
    return true;
}

bool VoxWorldAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

double VoxWorldAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int VoxWorldAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int VoxWorldAudioProcessor::getCurrentProgram()
{
    return 0;
}

void VoxWorldAudioProcessor::setCurrentProgram (int index)
{
}

const String VoxWorldAudioProcessor::getProgramName (int index)
{
    return String();
}

void VoxWorldAudioProcessor::changeProgramName (int index, const String& newName)
{
}

void VoxWorldAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
    VoxWorld_end();
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool VoxWorldAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif


//==============================================================================
bool VoxWorldAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* VoxWorldAudioProcessor::createEditor()
{
    return new VoxWorldAudioProcessorEditor (*this, parameters);
}

//==============================================================================
void VoxWorldAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
    auto xml = parameters.createXml();
    DBG("~ ~ ~ ~ ~ GET THIS: \n" + xml->toString());
    copyXmlToBinary(*xml, destData);
}

void VoxWorldAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
    std::unique_ptr<XmlElement> xml (getXmlFromBinary(data, sizeInBytes));
    
    if (xml != nullptr)
    {
        DBG("~ ~ ~ ~ ~ SET THIS: \n" + xml->toString());
        parameters = ValueTree::fromXml(*xml);
        
        parameters.removeProperty("platformX", nullptr);
        parameters.removeProperty("platformY", nullptr);
        
        auto numPlatforms = parameters.getProperty("numplatforms");

        VoxWorld_setNumPlatforms(numPlatforms ? (int)numPlatforms : 0);
        
        for (int i = 0; i < VoxWorld_getNumPlatforms(); i++)
        {
            auto platX = parameters.getProperty("platformX_"+String(i));
            auto platY = parameters.getProperty("platformY_"+String(i));
            
            if (platX && platY)
            {
                VoxWorld_setPlatformX(i, platX);
                VoxWorld_setPlatformY(i, platY);
            }
            else
            {
                parameters.removeProperty("platformX_"+String(i), nullptr);
                parameters.removeProperty("platformY_"+String(i), nullptr);
                VoxWorld_setPlatformX(i, -1.0);
                VoxWorld_setPlatformY(i, -1.0);
            }
        }
        
        VoxWorld_setNumClones(parameters.getProperty("numclones"));
        VoxWorld_setNumSkyDot(parameters.getProperty("numskydot"));
    }
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new VoxWorldAudioProcessor();
}
