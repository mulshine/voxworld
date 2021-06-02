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

float sampleRate = 44100; // sample rate overwritten on init
int blockSize = 128; // block size overwritten on init

LEAF leaf;
#define LEAF_MEMSIZE 1500000
char memory[LEAF_MEMSIZE];

#define LEFT 0
#define RIGHT 1
#define NUM_CHANNELS 2

VoxWorldMode mode = VoxWorldMode1;

// ~ ~ ~ AUTOTUNE parts ~ ~ ~
tRetune retune;

tRamp inputSmooth;
tRamp outputSmooth;
float inputSmoothTime = 0.020; // seconds
float outputSmoothTime = 0.005; // seconds

float freq;
int root = A, scale = MINOR;
float inputFreq = 0.0;
float inputMidi = 0.0;
float lastInputMidi = 0.0;
float tuneTo = 0.0;
float tuneBy = 0.0;

static float tune(float note);
static float getFrequencyRatio(float midiTranspose);


// ~ ~ ~ DELAY parts ~ ~ ~
tTapeDelay delay[NUM_CHANNELS];
float delayTime[NUM_CHANNELS] = {0.25, 0.225};
float delayFeedback[NUM_CHANNELS] = {0.2, 0.15};

// ~ ~ ~ VOCODER parts ~ ~ ~
tVocoder vocoder;
tSawtooth saw[3];
tRamp sawFreq;
tRamp gain;

// ~ ~ ~ CHOIR parts ~ ~ ~
#define NUM_VOICES 7 // 4 really for choir, first voice is reserved for normal autotune
tFormantShifter formant[NUM_VOICES];
float pitchShifts[NUM_VOICES] =     {  0.0,  0.2,   -5.0,  7.0, -12.0,  12.0, 2.0};
float formantShifts[NUM_VOICES] =   {  0.0,  5.0,   5.0,   -9.0,  9.0, -14.0, -4.0};
float gains[NUM_VOICES] =           {  0.0,  0.95,   0.85,   1.0,  0.9, 0.85, 0.85};
float pans[NUM_VOICES] =            {  0.0,  -0.1, -0.68,  0.65, -0.3,  0.3, 0.1};

float pitchRatios[NUM_VOICES];

static void setFormantTransposition(int which, float transp);
static void setPitchTransposition(int which, float transp);

// ~ ~ ~ ~ ~ MIXING parts ~ ~ ~ ~ ~
float out[2] = {0.0,0.0};


float autotuneOut[NUM_CHANNELS];
float vocoderOut[NUM_CHANNELS];
float choirOut[NUM_CHANNELS];
float formantOut[NUM_CHANNELS];
float delayOut[NUM_CHANNELS];

tRamp mix;
tRamp autotuneMix;
tRamp choirMix;
tRamp formantMix;
tRamp vocoderMix;

tRamp delayMix;
float delayMixTarget = 0.0;


int numClones = 0;
int numSkyDots = 0;
float X = 0.0;
float Y = 0.0;

float platformX[MAX_NUM_PLATFORMS];
float platformY[MAX_NUM_PLATFORMS];
int numPlatforms = 0;

void    VoxWorld_init            (float sr, int bs)
{
    sampleRate = sr;
    blockSize = bs;
    LEAF_init(&leaf, sampleRate, memory, LEAF_MEMSIZE, &getRandomFloat);
    
    for (int i = 0; i < MAX_NUM_PLATFORMS; i++)
    {
        platformX[i] = -1.0;
        platformY[i] = -1.0;
    }
    
    // ~ ~ ~ AUTOTUNE init ~ ~ ~
    tRetune_init(&retune, NUM_VOICES, 60, 1000, 1024, &leaf);
    tRetune_setMode(&retune, 0); // "shift by" mode
    tRetune_setPickiness(&retune, 0.92);
    
    tRamp_init(&inputSmooth, inputSmoothTime, 1, &leaf);
    tRamp_init(&outputSmooth, outputSmoothTime, 1, &leaf);
    
    // ~ ~ ~ VOCODER init ~ ~ ~
    tVocoder_init(&vocoder, &leaf);
    
    tSawtooth_init(&saw[0], &leaf);
    tSawtooth_init(&saw[1], &leaf);
    tSawtooth_init(&saw[2], &leaf);
    
    tSawtooth_setFreq(&saw[0], 110);
    tSawtooth_setFreq(&saw[1], 220);
    tSawtooth_setFreq(&saw[2], 330);
    
    tRamp_init(&sawFreq, 0.005, 1, &leaf);
    tRamp_init(&gain, 0.020, 1, &leaf);
    
    // ~ ~ ~ CHOIR init ~ ~ ~
    for (int i = 1; i < NUM_VOICES; i++)
    {
        tFormantShifter_init(&formant[i], 20, &leaf);
        
        setFormantTransposition(i, formantShifts[i]);
        setPitchTransposition(i, pitchShifts[i]);
    }
    
    // ~ ~ ~ DELAY init ~ ~ ~
    tTapeDelay_init(&delay[LEFT], delayTime[LEFT]*sampleRate, 0.5*sampleRate, &leaf);
    tTapeDelay_init(&delay[RIGHT], delayTime[RIGHT]*sampleRate, 0.5*sampleRate, &leaf);
    
    // ~ ~ ~ MIXING init ~ ~ ~
    tRamp_init(&mix, 1.0, 1, &leaf);
    tRamp_init(&autotuneMix, 2.0, 1, &leaf);
    tRamp_init(&vocoderMix, 2.0, 1, &leaf);
    tRamp_init(&choirMix, 2.0, 1, &leaf);
    tRamp_init(&delayMix, 2.0, 1, &leaf);
    tRamp_init(&formantMix, 2.0, 1, &leaf);
    
    tRamp_setDest(&choirMix, 1.0);
    //tRamp_setDest(&autotuneMix, 1.0);
    tRamp_setDest(&mix, 0.6);
    
}


// ~ ~ ~ ~ ~ RMS stuff ~ ~ ~ ~ ~
float squaresum = 0.01;
float thresh = 0.2;
float rms = 0.0;
float RMS_tick(float sample)
{
    squaresum = squaresum * 0.999 + sample * sample;
    rms = sqrt(squaresum);
    return rms;
}



// ~ ~ ~ ~ ~ VOXWORLD TICK !! ~ ~ ~ ~ ~
float*   VoxWorld_tick            (float input)
{
    inputFreq = tRetune_getInputFrequency(&retune);

    
    // ~ ~ ~ ~ ~ AUTOTUNE ~ ~ ~ ~ ~ ~
    inputMidi = ftom(inputFreq);
            
    tRamp_setDest(&inputSmooth, inputMidi);
    tRamp_setDest(&outputSmooth, mtof(tune(tRamp_tick(&inputSmooth))));

    tuneTo = tRamp_tick(&outputSmooth);
    pitchRatios[0] = tuneTo / inputFreq;
    
    tRetune_tuneVoices(&retune, pitchRatios); // performs shifting for choir and autotune
    
    float* tunedVoices = tRetune_tick(&retune, input);
            
    autotuneOut[0] = tunedVoices[0];
    autotuneOut[1] = autotuneOut[0];
    
    // ~ ~ ~ ~ ~ VOCODER ~ ~ ~ ~ ~
    tRamp_setDest(&sawFreq, inputFreq);

    float sawFreqHz = tRamp_tick(&sawFreq);
    
    tSawtooth_setFreq(&saw[0], sawFreqHz);
    tSawtooth_setFreq(&saw[1], sawFreqHz * 0.5);
    tSawtooth_setFreq(&saw[2], sawFreqHz * 1.5);
    
    float carrier = (tSawtooth_tick(&saw[0]) * 0.6 + tSawtooth_tick(&saw[1]) * 0.1 + tSawtooth_tick(&saw[2]) * 0.3);
    float vocoded = tVocoder_tick(&vocoder, carrier, input);
    
    tRamp_setDest(&gain, (RMS_tick(vocoded)>thresh) ? 1.0 : 0.0);
    
    vocoderOut[0] = tRamp_tick(&gain) * vocoded;
    vocoderOut[1] = vocoderOut[0];
    
    // ~ ~ ~ ~ ~ CHOIR ~ ~ ~ ~ ~
    float l = 0.0;
    float r = 0.0;
    float voices[NUM_VOICES];
    
    for (int i = 1; i < (1+numClones); i++)
    {
        voices[i] = tFormantShifter_tick(&formant[i], (tunedVoices[i]*gains[i]));
        
        float pan = (pans[i]+1.0)*0.5;
        l += voices[i] * (1.0-pan);
        r += voices[i] * pan;
    }
    
    choirOut[0] = l;
    choirOut[1] = r;
    
    float mx = tRamp_tick(&mix);
    // ~ ~ ~ ~ ~ MIXING ~ ~ ~ ~ ~
    l =     input * (1.0-mx) +
                ((vocoderOut[0]*tRamp_tick(&vocoderMix)) +
                 (autotuneOut[0]*tRamp_tick(&autotuneMix)) +
                 (choirOut[0]*tRamp_tick(&choirMix)) +
                 (formantOut[0]+tRamp_tick(&formantMix))) * mx;
    
    r =     input * (1.0-mx) +
                ((vocoderOut[1]*tRamp_sample(&vocoderMix)) +
                 (autotuneOut[1]*tRamp_sample(&autotuneMix)) +
                 (choirOut[1]*tRamp_sample(&choirMix)) +
                 (formantOut[1]+tRamp_sample(&formantMix))) * mx;
    
    float dmix = tRamp_tick(&delayMix);
    l = l * (1.0-dmix) + tTapeDelay_tick(&delay[LEFT], l + out[LEFT] * delayFeedback[LEFT]) * dmix;
    r = r * (1.0-dmix) + tTapeDelay_tick(&delay[RIGHT], r + out[RIGHT] * delayFeedback[RIGHT]) * dmix;
    
    out[LEFT] = l;
    out[RIGHT] = r;
    
    return out;
}

int firstFrame = 1;
bool lastState = false, lastPlayState = false;
void    VoxWorld_block           (AudioSampleBuffer& buffer)
{
   // DBG("rms: " + String(rms));
}

#define NUM_PEAKS 3

float dryPeak = 0.0;
float autotunePeak = 0.5;
float choirPeak = 1.0;

void VoxWorld_setX(float newX)
{
    X = LEAF_clip(0.0, newX, 1.0);
}

void VoxWorld_setY(float newY)
{
    Y = LEAF_clip(0.0, (newY-0.2)*1.25, 1.0);
    
    delayMixTarget = Y*0.5;
    tRamp_setDest(&delayMix, delayMixTarget);
    
}

float VoxWorld_getX(void)
{
    return X;
}


float VoxWorld_getY(void)
{
    return Y;
}

void VoxWorld_setNumPlatforms(int newNumPlatforms)
{
    numPlatforms = newNumPlatforms;
}

int VoxWorld_getNumPlatforms(void)
{
    return numPlatforms;
}

void VoxWorld_removePlatform(int which)
{
    for (int i = which; i < (MAX_NUM_PLATFORMS-1); i++)
    {
        platformX[i] = platformX[i+1];
        platformY[i] = platformY[i+1];
    }
}

void VoxWorld_setPlatformX(int which, float newX)
{
    DBG("new platform " + String(which) + " X: " + String(newX));
    platformX[LEAF_clipInt(0, which, MAX_NUM_PLATFORMS)] = newX;
}

void VoxWorld_setPlatformY(int which, float newY)
{
    DBG("new platform " + String(which) + " Y: " + String(newY));
    platformY[LEAF_clipInt(0, which, MAX_NUM_PLATFORMS)] = newY;
}

float VoxWorld_getPlatformX(int which)
{
    return platformX[which];
}


float VoxWorld_getPlatformY(int which)
{
    return platformY[which];
}

void VoxWorld_setNumSkyDot(int newNumSkyDots)
{
    DBG("new num sky dots: " + String(newNumSkyDots));
    numSkyDots = newNumSkyDots;
    
    VoxWorld_setDelayFeedback(0, LEAF_clip(0.4, 0.4 + newNumSkyDots * (1.0/20.0)*0.5999, 0.9999));
    VoxWorld_setDelayFeedback(1, VoxWorld_getDelayFeedback(0));
    DBG("new delay feedback: " + String(VoxWorld_getDelayFeedback(0)));
    
}

int VoxWorld_getNumSkyDot(void)
{
    return numSkyDots;
}

void VoxWorld_setNumClones(int newNumClones)
{
    DBG("new num clones: " + String(newNumClones));
    numClones = LEAF_clipInt(0, newNumClones, 6);
    
}

int VoxWorld_getNumClones(void)
{
    return numClones;
}


void VoxWorld_clone(void)
{
    numClones += 1;
    
    if (numClones > 6) numClones = 6;
}

void VoxWorld_declone(void)
{
    numClones -= 1;
    
    if (numClones < 0) numClones = 0;
}
void setRootAndScale(int newRoot, int newScale)
{
    root = newRoot;
    scale = newScale;
}

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

float getFrequencyRatio(float midiTranspose)
{
    return mtof(60.0+midiTranspose)/mtof(60);
}

void setFormantTransposition(int which, float transp){
    tFormantShifter_setShiftFactor(&formant[which], getFrequencyRatio(transp));
}

void setPitchTransposition(int which, float transp)
{
    pitchShifts[which] = transp;
    pitchRatios[which] = getFrequencyRatio(pitchShifts[which]);
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
    DBG("ENDINGGGGG VOX WORLDDDDD");
    
    tRamp_free(&gain);
    tRamp_free(&sawFreq);
    
    tSawtooth_free(&saw[2]);
    tSawtooth_free(&saw[1]);
    tSawtooth_free(&saw[0]);
    tRetune_free(&retune);
    tVocoder_free(&vocoder);
    
    tTapeDelay_free(&delay[LEFT]);
    tTapeDelay_free(&delay[RIGHT]);
    
    tRamp_free(&delayMix);
    tRamp_free(&autotuneMix);
    tRamp_free(&vocoderMix);
    tRamp_free(&formantMix);
    tRamp_free(&choirMix);
    
    DBG("alloc: " + String(leaf.allocCount));
    DBG("free: " + String(leaf.freeCount));
    //leaf_free(&leaf, leaf);
}

// VOX WORLD UTILITIES
float VoxWorld_getDelayTime(int chan) { return delayTime[chan]; }
float VoxWorld_getDelayFeedback(int chan)  { return delayFeedback[chan]; }

void VoxWorld_setDelayTime(int chan, float time)
{
    delayTime[chan] = time;
    tTapeDelay_setDelay(&delay[chan], sampleRate * time);
}

void VoxWorld_setDelayFeedback(int chan, float fb)
{
    delayFeedback[chan] = fb;
}

void VoxWorld_increaseDelayFeedback(void)
{
    for (int i = 0; i < NUM_CHANNELS; i++)
    {
        float fb = LEAF_clip(0.0, VoxWorld_getDelayFeedback(i)+0.05, 0.95);
        VoxWorld_setDelayFeedback(i,fb);
        
        DBG("increase feedback " + String(i) + ": " + String(fb));
    }
}

void VoxWorld_decreaseDelayFeedback(void)
{
    for (int i = 0; i < NUM_CHANNELS; i++)
    {
        float fb = LEAF_clip(0.0, VoxWorld_getDelayFeedback(i)-0.05, 0.95);
        VoxWorld_setDelayFeedback(i,fb);
        
        DBG("decrease feedback " + String(i) + ": " + String(fb));
    }
}

void VoxWorld_increaseDelayTime(void)
{
    for (int i = 0; i < NUM_CHANNELS; i++)
    {
        float time = LEAF_clip(0.0, VoxWorld_getDelayTime(i)+0.05, 1.0);
        VoxWorld_setDelayTime(i,time);
        
        DBG("increase delay time " + String(i) + ": " + String(time));
    }
}

void VoxWorld_decreaseDelayTime(void)
{
    for (int i = 0; i < NUM_CHANNELS; i++)
    {
        float time = LEAF_clip(0.0, VoxWorld_getDelayTime(i)-0.05, 1.0);
        VoxWorld_setDelayTime(i,time);
        
        DBG("decrease delay time " + String(i) + ": " + String(time));
    }
}

void VoxWorld_increaseDelayMix(void)
{
    delayMixTarget = LEAF_clip(0.0, delayMixTarget + 0.05, 0.9);
    tRamp_setDest(&delayMix, delayMixTarget);
    
    DBG("increase delay mix: " + String(delayMixTarget));
}

void VoxWorld_decreaseDelayMix(void)
{
    delayMixTarget = LEAF_clip(0.0, delayMixTarget - 0.05, 0.9);
    tRamp_setDest(&delayMix, delayMixTarget);
    
    DBG("decrease delay mix: " + String(delayMixTarget));
}

void VoxWorld_setMode(VoxWorldMode newMode)
{
    mode = newMode;
    if (mode == VoxWorldMode1)
    {
        tRetune_setMode(&retune, 1); // tune to
        
        tRamp_setDest(&autotuneMix, 1.0);
        tRamp_setDest(&vocoderMix, 0.0);
        tRamp_setDest(&choirMix, 0.0);
        tRamp_setDest(&delayMix, 0.0);
    }
    else if (mode == VoxWorldMode2)
    {
        tRamp_setDest(&autotuneMix, 0.0);
        tRamp_setDest(&vocoderMix, 1.0);
        tRamp_setDest(&choirMix, 0.0);
        tRamp_setDest(&delayMix, 1.0);
    }
    else if (mode == VoxWorldMode3)
    {
        tRetune_setMode(&retune, 0); // tune by
        
        tRamp_setDest(&autotuneMix, 0.0);
        tRamp_setDest(&vocoderMix, 0.0);
        tRamp_setDest(&choirMix, 1.0);
        tRamp_setDest(&delayMix, 1.0);
    }
}

