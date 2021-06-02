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

#include "../LEAF/leaf/leaf.h"

// VoxWorld API

#define MAX_NUM_PLATFORMS 10

typedef enum VoxWorldMode
{
    VoxWorldMode1,
    VoxWorldMode2,
    VoxWorldMode3,
    VoxWorldMode4
} VoxWorldMode;

float*   VoxWorld_tick            (float input);
void    VoxWorld_init            (float sampleRate, int blocksize);
void    VoxWorld_end             (void);
void    VoxWorld_block           (AudioSampleBuffer& buffer);

void    VoxWorld_noteOn          (int midiNoteNumber, float velocity);
void    VoxWorld_noteOff         (int midiNoteNumber);


float VoxWorld_getDelayTime(int chan);
float VoxWorld_getDelayFeedback(int chan);

void VoxWorld_setDelayTime(int chan, float time);
void VoxWorld_setDelayFeedback(int chan, float fb);


void VoxWorld_setMode(VoxWorldMode mode);

void VoxWorld_setX(float X);
void VoxWorld_setY(float Y);
float VoxWorld_getX(void);
float VoxWorld_getY(void);


void VoxWorld_setNumSkyDot(int newNumSkyDots);
int VoxWorld_getNumSkyDot(void);

void VoxWorld_setNumPlatforms(int newNumPlatforms);
int VoxWorld_getNumPlatforms(void);
void VoxWorld_removePlatform(int which);
void VoxWorld_setPlatformX(int which, float X);
void VoxWorld_setPlatformY(int which, float Y);
float VoxWorld_getPlatformX(int which);
float VoxWorld_getPlatformY(int which);

void VoxWorld_setNumClones(int newNumClones);
int VoxWorld_getNumClones(void);
void VoxWorld_clone(void);
void VoxWorld_declone(void);

void VoxWorld_increaseDelayFeedback(void);
void VoxWorld_decreaseDelayFeedback(void);

void VoxWorld_increaseDelayTime(void);
void VoxWorld_decreaseDelayTime(void);

void VoxWorld_increaseDelayMix(void);
void VoxWorld_decreaseDelayMix(void);



#endif  // VOXWORLD_H_INCLUDED
