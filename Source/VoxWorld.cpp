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
float delayFeedback = 0.1;
float delayTime = 0.1;

tSVF delayLPF;
float delayLPF_cutoff;

tSimpleRetune retune;
float freq;
int root = C, scale = MAJOR;
float inputFreq = 0.0;
float inputMidi = 0.0;
float lastInputMidi = 0.0;
float tuneTo = 0.0;
float tuneBy = 0.0;

tFormantShifter formant;


#define NUM_VOICES 3
tFormantShifter formants[NUM_VOICES];
tRetune retunes;


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





#define MSIZE 0
char memory[MSIZE];

int lastLoadedAudioSize = 0;



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

void formant_transpose(float transp)
{
    tFormantShifter_setShiftFactor(&formant, mtof(60+transp)/mtof(60));
}
#define FORMANT_ORDER 15

void    VoxWorld_init            (float sr, int bs)
{
    sampleRate = sr;
    blockSize = bs;
    LEAF_init(&leaf, sampleRate, memory, MSIZE, &getRandomFloat);
    
    tSimpleRetune_init(&retune, 1, 90, 1000, 512, &leaf);
    tSimpleRetune_setMode(&retune, 0);
    tSimpleRetune_setPickiness(&retune, 0.9);
    
    tFormantShifter_init(&formant, FORMANT_ORDER, &leaf);
    formant_transpose(3.0);
    //tFormantShifter_setIntensity(&formant, 3.0);
    
    tTapeDelay_init(&delay, 0.1*sampleRate, 2.0*sampleRate, &leaf);
    
    tSVF_init(&delayLPF, SVFTypeLowpass, 2500, 1.0, &leaf);

    setRootAndScale(C, MAJOR);

}
#define RETUNE 0
#define FORMANT 1
#define DELAY 0

float samp = 0.0;
float retune_out = 0.0;
float formant_out = 0.0;
float delay_out = 0.0;
float mix = 1.0;
float   VoxWorld_tick            (float input)
{
#if RETUNE
    inputFreq = tSimpleRetune_getInputFrequency(&retune);
    inputMidi = ftom(inputFreq);
    
    tuneTo = mtof(tune(inputMidi));

    tSimpleRetune_tuneVoice(&retune, 0, tuneTo);
    
    retune_out = tSimpleRetune_tick(&retune, input);
#endif
#if FORMANT

    tuneBy = mtof(67)/mtof(60);
    
    tSimpleRetune_tuneVoice(&retune, 0, tuneBy);
    float retuned = tSimpleRetune_tick(&retune, input);
    formant_out = tFormantShifter_tick(&formant, retuned);

#endif
#if DELAY
    delay_out = tTapeDelay_tick(&delay, input + tSVF_tick(&delayLPF, delayOut*delayFeedback));
#endif
    
    return mix * ((delay_out + formant_out + retune_out) / (RETUNE + FORMANT + DELAY)) + (1.0-mix) * input;
}

int firstFrame = 1;
bool lastState = false, lastPlayState = false;
void    VoxWorld_block           (AudioSampleBuffer& buffer)
{
    //DBG(tuneBy);
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
