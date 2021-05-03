/*
 ==============================================================================
 FM.c
 Created: 23 Jan 2017 9:39:38am
 Author:  Michael R Mulshine
 ==============================================================================
 */

#include "VoxWorld.h"

#define C 0
#define Db 1
#define D 2
#define Eb 3
#define E 4
#define F 5
#define Gb 6
#define G 7
#define Ab 8
#define A 9
#define Bb 10
#define B 11
#define Cs Db
#define Ds Eb
#define Fs Gb
#define Gs Ab
#define As Bb

#define MAJOR 0
#define MINOR 1
#define PENTATONIC 2


static void leaf_pool_report(void);
static void leaf_pool_dump(void);
static void run_pool_test(void);

float sampleRate = 44100;
int blockSize = 128;

float rms = 0.0;

tTapeDelay delay;
float delayLast = 0.0;
float delayFeedback = 0.1;
float delayTime = 0.1;

tSVF delayLPF;
float delayLPF_cutoff;

float VoxWorld_getDelayTime(void) { return delayTime; }
float VoxWorld_getDelayFeedback(void)  { return delayFeedback; }

void VoxWorld_setDelayTime(float time)
{
    delayTime = time;
    tTapeDelay_setDelay(&delay, sampleRate * time);
    
}
void VoxWorld_setDelayFeedback(float fb)
{
    delayFeedback = fb;
}

LEAF leaf;

tSimpleRetune retune;


#define NUM_BANDS 10
tSVF eq[NUM_BANDS];





float freq;

#define MSIZE 4048000
char memory[MSIZE];

int lastLoadedAudioSize = 0;

int root = C, scale = MAJOR;

float inputFreq = 0.0;
float inputMidi = 0.0;
float lastInputMidi = 0.0;
float tuneTo = 0.0;

void setRootAndScale(int newRoot, int newScale)
{
    root = newRoot;
    scale = newScale;
}

// Could make this more modern C++ or shorter with some arrays and looping, but hey...
// it works and is just as efficient
float tune(float note)
{
    float pitch = fmod(note-(float)root, 12.0);
    int octave = (int)((note-(float)root) / 12.0);
    float transp = 0.0;

    //float* transitions = (scale == MAJOR) ? major_transitions : minor_transitions;
    if (scale == MAJOR)
    {
        if (pitch < 1.0) transp = 0.0;
        else if (pitch < 3.0) transp = 2.0;
        else if (pitch < 4.5) transp = 4.0;
        else if (pitch < 6.0) transp = 5.0;
        else if (pitch < 8.0) transp = 7.0;
        else if (pitch < 10.0) transp = 9.0;
        else if (pitch < 11.5) transp = 11.0;
        else transp = 12.0;
    }
    else if (scale == MINOR)
    {
        if (pitch < 1.0) transp = 0.0;
        else if (pitch < 2.5) transp = 2.0;
        else if (pitch < 4.5) transp = 3.0;
        else if (pitch < 6.0) transp = 5.0;
        else if (pitch < 7.5) transp = 7.0;
        else if (pitch < 9.0) transp = 8.0;
        else if (pitch < 10.5) transp = 10.0; // natural 7
        else if (pitch < 11.5) transp = 11.0; // raised 7 (leading tone)
        else transp = 12.0;
    }
    else if (scale == PENTATONIC)
    {
        if (pitch < 1.5) transp = 0.0;
        else if (pitch < 4.0) transp = 3.0;
        else if (pitch < 5.5) transp = 5.0;
        else if (pitch < 6.5) transp = 6.0; // blues note
        else if (pitch < 8.5) transp = 7.0;
        else if (pitch < 11.0) transp = 10.0;
        else transp = 12.0;
    }
    
    return 12.0 * octave + (float) root + transp;
}

float bandHz[20] =
{
    50,
    100,
    200,
    400,
    800,
    
    1600,
    2400,
    3200,
    4000,
    4800,
    
    5600,
    6600,
    7600,
    9600,
    11600,
    
    13600,
    15600,
    17600,
    19600,
    21000
};

void    VoxWorld_init            (float sr, int bs)
{
    sampleRate = sr;
    blockSize = bs;
    LEAF_init(&leaf, sampleRate, memory, MSIZE, &getRandomFloat);
    
    tSimpleRetune_init(&retune, 1, 100, 500, 512, &leaf);
    tSimpleRetune_setMode(&retune, 1);
    tSimpleRetune_setPickiness(&retune, 0.9);
    
    tTapeDelay_init(&delay, 0.1*sampleRate, 2.0*sampleRate, &leaf);
    
    tSVF_init(&delayLPF, SVFTypeLowpass, 2500, 1.0, &leaf);
    
    for (int i = 0; i < NUM_BANDS; i++)
    {
        tSVF_init(&eq[i], SVFTypePeak, bandHz[i], 1.0, &leaf);
    }

    setRootAndScale(C, MAJOR);

}
#define RETUNE 0

float samp = 0.0;
float   VoxWorld_tick            (float input)
{
#if RETUNE
    inputFreq = tSimpleRetune_getInputFrequency(&retune);
    inputMidi = ftom(inputFreq);
    
    //if (fabsf(inputMidi-lastInputMidi) > 0.00)
    {
        tuneTo = mtof(tune(inputMidi));
    }
    lastInputMidi = inputMidi;
    
    tSimpleRetune_tuneVoice(&retune, 0, tuneTo);
    
    float retune = tSimpleRetune_tick(&retune, input);
#endif
    
    samp = input;
    /*
    for (int i = 0; i < NUM_BANDS; i++)
    {
        samp = tSVF_tick(&eq[i], samp);
    }
    */
    delayLast = tTapeDelay_tick(&delay, input + tSVF_tick(&delayLPF, delayLast*delayFeedback));
    
    return input*0.8 + delayLast*0.2;
}

int firstFrame = 1;
bool lastState = false, lastPlayState = false;
void    VoxWorld_block           (AudioSampleBuffer& buffer)
{
}

void    VoxWorld_controllerInput (int cnum, float cval)
{
    
}

void    VoxWorld_pitchBendInput  (int pitchBend)
{
    
}

int lastNote;
void    VoxWorld_noteOn          (int note, float velocity)
{
}


void    VoxWorld_noteOff         (int note)
{
}



void    VoxWorld_end             (void)
{
    
}











// LEAF POOL UTILITIES

void leaf_pool_report(void)
{
    DBG(String(leaf_pool_get_used(&leaf)) + " of  " + String(leaf_pool_get_size(&leaf)));
}

void leaf_pool_dump(void)
{
    float* buff = (float*)leaf_pool_get_pool(&leaf);
    unsigned long siz = leaf_pool_get_size(&leaf);
    siz /= sizeof(float);
    for (int i = 0; i < siz; i++)
    {
        DBG(String(buff[i]));
    }
}

static void run_pool_test(void)
{
    leaf_pool_report();

    DBG("ALLOC BUFFER 1");
    int size = 50;
    float* buffer;
    buffer = (float*) leaf_alloc(&leaf, sizeof(float) * size);

    for (int i = 0; i < size; i++)
    {
        buffer[i] = (float)i;

    }

    leaf_pool_report();

    DBG("ALLOC BUFFER 2");
    size = 25;

    buffer = (float*) leaf_alloc(&leaf, sizeof(float) * size);

    leaf_pool_report();

    for (int i = 0; i < size; i++)
    {
        buffer[i] = (float)(i*2);
    }
    leaf_free(&leaf, (char*)buffer);

    leaf_pool_report();

    DBG("ALLOC BUFFER 3");
    size = 15;

    buffer = (float*) leaf_alloc(&leaf, sizeof(float) * size);

    for (int i = 0; i < size; i++)
    {
        buffer[i] = (float)(i*3);
    }

    leaf_pool_report();

    leaf_pool_dump();
}
