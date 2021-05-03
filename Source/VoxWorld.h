/*
  ==============================================================================

    VoxWorld.h
    Created: 03 May 2021 12:40:00pm
    Author:  Michael R Mulshine

  ==============================================================================
*/

#ifndef VOXWORLD_H_INCLUDED
#define VOXWORLD_H_INCLUDED

#include "LEAFLink.h"

#include "Yin.h"
#include "../LEAF/leaf/leaf.h"

// VoxWorld API

float   VoxWorld_tick            (float input);
void    VoxWorld_init            (float sampleRate, int blocksize);
void    VoxWorld_end             (void);
void    VoxWorld_block           (AudioSampleBuffer& buffer);

void    VoxWorld_noteOn          (int midiNoteNumber, float velocity);
void    VoxWorld_noteOff         (int midiNoteNumber);

float VoxWorld_getDelayTime(void);
float VoxWorld_getDelayFeedback(void);

void VoxWorld_setDelayTime(float time);
void VoxWorld_setDelayFeedback(float fb);




#endif  // VOXWORLD_H_INCLUDED
