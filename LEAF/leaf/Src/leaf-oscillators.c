/*==============================================================================
 leaf-oscillators.c
 Created: 20 Jan 2017 12:00:58pm
 Author:  Michael R Mulshine
 ==============================================================================*/

#if _WIN32 || _WIN64

#include "..\Inc\leaf-oscillators.h"
#include "..\leaf.h"

#else

#include "../Inc/leaf-oscillators.h"
#include "../leaf.h"

#endif

#if LEAF_INCLUDE_SINE_TABLE
// Cycle
void    tCycle_init(tCycle* const cy, LEAF* const leaf)
{
    tCycle_initToPool(cy, &leaf->mempool);
}

void    tCycle_initToPool   (tCycle* const cy, tMempool* const mp)
{
    _tMempool* m = *mp;
    _tCycle* c = *cy = (_tCycle*) mpool_alloc(sizeof(_tCycle), m);
    c->mempool = m;
    LEAF* leaf = c->mempool->leaf;
    
    c->inc      =  0.0f;
    c->phase    =  0.0f;
    c->invSampleRate = leaf->invSampleRate;
}

void    tCycle_free (tCycle* const cy)
{
    _tCycle* c = *cy;
    
    mpool_free((char*)c, c->mempool);
}

void     tCycle_setFreq(tCycle* const cy, float freq)
{
    _tCycle* c = *cy;
    
    if (!isfinite(freq)) return;
    
    c->freq  = freq;

    c->inc = freq * c->invSampleRate;
    c->inc -= (int)c->inc;
}

//need to check bounds and wrap table properly to allow through-zero FM
float   tCycle_tick(tCycle* const cy)
{
    _tCycle* c = *cy;
    float temp;
    int idx;
    float frac;
    float samp0;
    float samp1;
    
    // Phasor increment
    c->phase += c->inc;
    if (c->phase >= 1.0f) c->phase -= 1.0f;
    if (c->phase < 0.0f) c->phase += 1.0f;

    // Wavetable synthesis

    temp = SINE_TABLE_SIZE * c->phase;
    idx = (int)temp;
    frac = temp - (float)idx;
    samp0 = __leaf_table_sinewave[idx];
    if (++idx >= SINE_TABLE_SIZE) idx = 0;
    samp1 = __leaf_table_sinewave[idx];

    return (samp0 + (samp1 - samp0) * frac);
}

void     tCycle_setSampleRate (tCycle* const cy, float sr)
{
    _tCycle* c = *cy;
    
    c->invSampleRate = 1.0f/sr;
    tCycle_setFreq(cy, c->freq);
}
#endif // LEAF_INCLUDE_SINE_TABLE

#if LEAF_INCLUDE_TRIANGLE_TABLE
//========================================================================
/* Triangle */
void   tTriangle_init(tTriangle* const cy, LEAF* const leaf)
{
    tTriangle_initToPool(cy, &leaf->mempool);
}

void    tTriangle_initToPool    (tTriangle* const cy, tMempool* const mp)
{
    _tMempool* m = *mp;
    _tTriangle* c = *cy = (_tTriangle*) mpool_alloc(sizeof(_tTriangle), m);
    c->mempool = m;
    LEAF* leaf = c->mempool->leaf;
    
    c->inc      =  0.0f;
    c->phase    =  0.0f;
    c->invSampleRate = leaf->invSampleRate;
    tTriangle_setFreq(cy, 220);
}

void    tTriangle_free  (tTriangle* const cy)
{
    _tTriangle* c = *cy;
    
    mpool_free((char*)c, c->mempool);
}

void tTriangle_setFreq(tTriangle* const cy, float freq)
{
    _tTriangle* c = *cy;
    
    c->freq = freq;
    
    c->inc = c->freq * c->invSampleRate;
    c->inc -= (int)c->inc;
    
    // abs for negative frequencies
    c->w = fabsf(c->freq * (TRI_TABLE_SIZE * c->invSampleRate));
    
    c->w = log2f_approx(c->w);//+ LEAF_SQRT2 - 1.0f; adding an offset here will shift our table selection upward, reducing aliasing but lower high freq fidelity. +1.0f should remove all aliasing
    if (c->w < 0.0f) c->w = 0.0f;
    c->oct = (int)c->w;
    c->w -= c->oct;
    if (c->oct >= 10) c->oct = 9;
}


float   tTriangle_tick(tTriangle* const cy)
{
    _tTriangle* c = *cy;
    
    float temp;
    int idx;
    float frac;
    float samp0;
    float samp1;
    
    // Phasor increment
    c->phase += c->inc;
    if (c->phase >= 1.0f) c->phase -= 1.0f;
    if (c->phase < 0.0f) c->phase += 1.0f;

    // Wavetable synthesis
    temp = TRI_TABLE_SIZE * c->phase;
    
    idx = (int)temp;
    frac = temp - (float)idx;
    samp0 = __leaf_table_triangle[c->oct][idx];
    if (++idx >= TRI_TABLE_SIZE) idx = 0;
    samp1 = __leaf_table_triangle[c->oct][idx];
    
    float oct0 = (samp0 + (samp1 - samp0) * frac);
    
    idx = (int)temp;
    samp0 = __leaf_table_triangle[c->oct+1][idx];
    if (++idx >= TRI_TABLE_SIZE) idx = 0;
    samp1 = __leaf_table_triangle[c->oct+1][idx];
    
    float oct1 = (samp0 + (samp1 - samp0) * frac);
    
    return oct0 + (oct1 - oct0) * c->w;
}

void     tTriangle_setSampleRate (tTriangle* const cy, float sr)
{
    _tTriangle* c = *cy;
    
    c->invSampleRate = 1.0f/sr;
    tTriangle_setFreq(cy, c->freq);
}
#endif // LEAF_INCLUDE_TRIANGLE_TABLE

#if LEAF_INCLUDE_SQUARE_TABLE
//========================================================================
/* Square */
void   tSquare_init(tSquare* const cy, LEAF* const leaf)
{
    tSquare_initToPool(cy, &leaf->mempool);
}

void    tSquare_initToPool  (tSquare* const cy, tMempool* const mp)
{
    _tMempool* m = *mp;
    _tSquare* c = *cy = (_tSquare*) mpool_alloc(sizeof(_tSquare), m);
    c->mempool = m;
    LEAF* leaf = c->mempool->leaf;
    
    c->inc      =  0.0f;
    c->phase    =  0.0f;
    c->invSampleRate = leaf->invSampleRate;
    tSquare_setFreq(cy, 220);
}

void    tSquare_free (tSquare* const cy)
{
    _tSquare* c = *cy;
    
    mpool_free((char*)c, c->mempool);
}

void    tSquare_setFreq(tSquare* const cy, float freq)
{
    _tSquare* c = *cy;

    c->freq  = freq;
    
    c->inc = c->freq * c->invSampleRate;
    c->inc -= (int)c->inc;
    
    // abs for negative frequencies
    c->w = fabsf(c->freq * (SQR_TABLE_SIZE * c->invSampleRate));
    
    c->w = log2f_approx(c->w);//+ LEAF_SQRT2 - 1.0f; adding an offset here will shift our table selection upward, reducing aliasing but lower high freq fidelity. +1.0f should remove all aliasing
    if (c->w < 0.0f) c->w = 0.0f;
    c->oct = (int)c->w;
    c->w -= c->oct;
    if (c->oct >= 10) c->oct = 9;
}

float   tSquare_tick(tSquare* const cy)
{
    _tSquare* c = *cy;
    
    float temp;
    int idx;
    float frac;
    float samp0;
    float samp1;
    
    // Phasor increment
    c->phase += c->inc;
    if (c->phase >= 1.0f) c->phase -= 1.0f;
    if (c->phase < 0.0f) c->phase += 1.0f;

    // Wavetable synthesis
    temp = SQR_TABLE_SIZE * c->phase;
    
    idx = (int)temp;
    frac = temp - (float)idx;
    samp0 = __leaf_table_squarewave[c->oct][idx];
    if (++idx >= SQR_TABLE_SIZE) idx = 0;
    samp1 = __leaf_table_squarewave[c->oct][idx];
    
    float oct0 = (samp0 + (samp1 - samp0) * frac);
    
    idx = (int)temp;
    samp0 = __leaf_table_squarewave[c->oct+1][idx];
    if (++idx >= SQR_TABLE_SIZE) idx = 0;
    samp1 = __leaf_table_squarewave[c->oct+1][idx];
    
    float oct1 = (samp0 + (samp1 - samp0) * frac);
    
    return oct0 + (oct1 - oct0) * c->w;
}

void     tSquare_setSampleRate (tSquare* const cy, float sr)
{
    _tSquare* c = *cy;
    
    c->invSampleRate = 1.0f/sr;
    tSquare_setFreq(cy, c->freq);
}
#endif // LEAF_INCLUDE_SQUARE_TABLE

#if LEAF_INCLUDE_SAWTOOTH_TABLE
//=====================================================================
// Sawtooth
void    tSawtooth_init(tSawtooth* const cy, LEAF* const leaf)
{
    tSawtooth_initToPool(cy, &leaf->mempool);
}

void    tSawtooth_initToPool    (tSawtooth* const cy, tMempool* const mp)
{
    _tMempool* m = *mp;
    _tSawtooth* c = *cy = (_tSawtooth*) mpool_alloc(sizeof(_tSawtooth), m);
    c->mempool = m;
    LEAF* leaf = c->mempool->leaf;
    
    c->inc      = 0.0f;
    c->phase    = 0.0f;
    c->invSampleRate = leaf->invSampleRate;
    tSawtooth_setFreq(cy, 220);
}

void    tSawtooth_free (tSawtooth* const cy)
{
    _tSawtooth* c = *cy;
    
    mpool_free((char*)c, c->mempool);
}

void    tSawtooth_setFreq(tSawtooth* const cy, float freq)
{
    _tSawtooth* c = *cy;
    
    c->freq  = freq;
    
    c->inc = c->freq * c->invSampleRate;
    c->inc -= (int)c->inc;
    
    // abs for negative frequencies
    c->w = fabsf(c->freq * (SAW_TABLE_SIZE * c->invSampleRate));
    
    c->w = log2f_approx(c->w);//+ LEAF_SQRT2 - 1.0f; adding an offset here will shift our table selection upward, reducing aliasing but lower high freq fidelity. +1.0f should remove all aliasing
    if (c->w < 0.0f) c->w = 0.0f; // If c->w is < 0.0f, then freq is less than our base freq
    c->oct = (int)c->w;
    c->w -= c->oct;
    if (c->oct >= 10) c->oct = 9;
}

float   tSawtooth_tick(tSawtooth* const cy)
{
    _tSawtooth* c = *cy;
    
    float temp;
    int idx;
    float frac;
    float samp0;
    float samp1;
    
    // Phasor increment
    c->phase += c->inc;
    while (c->phase >= 1.0f) c->phase -= 1.0f;
    while (c->phase < 0.0f) c->phase += 1.0f;
    
    // Wavetable synthesis
    temp = SAW_TABLE_SIZE * c->phase;
    
    idx = (int)temp;
    frac = temp - (float)idx;
    samp0 = __leaf_table_sawtooth[c->oct][idx];
    if (++idx >= SAW_TABLE_SIZE) idx = 0;
    samp1 = __leaf_table_sawtooth[c->oct][idx];
    
    float oct0 = (samp0 + (samp1 - samp0) * frac);
    
    idx = (int)temp;
    samp0 = __leaf_table_sawtooth[c->oct+1][idx];
    if (++idx >= SAW_TABLE_SIZE) idx = 0;
    samp1 = __leaf_table_sawtooth[c->oct+1][idx];
    
    float oct1 = (samp0 + (samp1 - samp0) * frac);
    
    return oct0 + (oct1 - oct0) * c->w;
}

void     tSawtooth_setSampleRate (tSawtooth* const cy, float sr)
{
    _tSawtooth* c = *cy;
    
    c->invSampleRate = 1.0f/sr;
    tSawtooth_setFreq(cy, c->freq);
}
#endif // LEAF_INCLUDE_SAWTOOTH_TABLE

//==============================================================================

/* tTri: Anti-aliased Triangle waveform. */
void    tPBTriangle_init          (tPBTriangle* const osc, LEAF* const leaf)
{
    tPBTriangle_initToPool(osc, &leaf->mempool);
}

void    tPBTriangle_initToPool    (tPBTriangle* const osc, tMempool* const mp)
{
    _tMempool* m = *mp;
    _tPBTriangle* c = *osc = (_tPBTriangle*) mpool_alloc(sizeof(_tPBTriangle), m);
    c->mempool = m;
    LEAF* leaf = c->mempool->leaf;

    c->invSampleRate = leaf->invSampleRate;
    c->inc      =  0.0f;
    c->phase    =  0.0f;
    c->skew     =  0.5f;
    c->lastOut  =  0.0f;
}

void    tPBTriangle_free (tPBTriangle* const cy)
{
    _tPBTriangle* c = *cy;
    
    mpool_free((char*)c, c->mempool);
}

float   tPBTriangle_tick          (tPBTriangle* const osc)
{
    _tPBTriangle* c = *osc;
    
    float out;
    float skew;
    
    if (c->phase < c->skew)
    {
        out = 1.0f;
        skew = (1.0f - c->skew) * 2.0f;
    }
    else
    {
        out = -1.0f;
        skew = c->skew * 2.0f;
    }
    
    out += LEAF_poly_blep(c->phase, c->inc);
    out -= LEAF_poly_blep(fmodf(c->phase + (1.0f - c->skew), 1.0f), c->inc);
    
    out = (skew * c->inc * out) + ((1 - c->inc) * c->lastOut);
    c->lastOut = out;
    
    c->phase += c->inc - (int)c->inc;
    if (c->phase >= 1.0f) c->phase -= 1.0f;
    if (c->phase < 0.0f) c->phase += 1.0f;
    
    return out;
}

void    tPBTriangle_setFreq       (tPBTriangle* const osc, float freq)
{
    _tPBTriangle* c = *osc;
    
    c->freq  = freq;
    c->inc = freq * c->invSampleRate;
}

void    tPBTriangle_setSkew       (tPBTriangle* const osc, float skew)
{
    _tPBTriangle* c = *osc;
    c->skew = (skew + 1.0f) * 0.5f;
}

void     tPBTriangle_setSampleRate (tPBTriangle* const osc, float sr)
{
    _tPBTriangle* c = *osc;
    
    c->invSampleRate = 1.0f/sr;
    tPBTriangle_setFreq(osc, c->freq);
}


//==============================================================================

/* tPulse: Anti-aliased pulse waveform. */
void    tPBPulse_init        (tPBPulse* const osc, LEAF* const leaf)
{
    tPBPulse_initToPool(osc, &leaf->mempool);
}

void    tPBPulse_initToPool  (tPBPulse* const osc, tMempool* const mp)
{
    _tMempool* m = *mp;
    _tPBPulse* c = *osc = (_tPBPulse*) mpool_alloc(sizeof(_tPBPulse), m);
    c->mempool = m;
    LEAF* leaf = c->mempool->leaf;
    
    c->invSampleRate = leaf->invSampleRate;
    c->inc      =  0.0f;
    c->phase    =  0.0f;
    c->width     =  0.5f;
}

void    tPBPulse_free (tPBPulse* const osc)
{
    _tPBPulse* c = *osc;
    
    mpool_free((char*)c, c->mempool);
}

float   tPBPulse_tick        (tPBPulse* const osc)
{
    _tPBPulse* c = *osc;
    
    float out;
    if (c->phase < c->width) out = 1.0f;
    else out = -1.0f;
    out += LEAF_poly_blep(c->phase, c->inc);
    out -= LEAF_poly_blep(fmodf(c->phase + (1.0f - c->width), 1.0f), c->inc);
    
    c->phase += c->inc - (int)c->inc;
    if (c->phase >= 1.0f) c->phase -= 1.0f;
    if (c->phase < 0.0f) c->phase += 1.0f;
    
    return out;
}

void    tPBPulse_setFreq     (tPBPulse* const osc, float freq)
{
    _tPBPulse* c = *osc;
    
    c->freq  = freq;
    c->inc = freq * c->invSampleRate;
}

void    tPBPulse_setWidth    (tPBPulse* const osc, float width)
{
    _tPBPulse* c = *osc;
    c->width = width;
}

void    tPBPulse_setSampleRate (tPBPulse* const osc, float sr)
{
    _tPBPulse* c = *osc;
    
    c->invSampleRate = 1.0f/sr;
    tPBPulse_setFreq(osc, c->freq);
}

//==============================================================================

/* tSawtooth: Anti-aliased Sawtooth waveform. */
void    tPBSaw_init          (tPBSaw* const osc, LEAF* const leaf)
{
    tPBSaw_initToPool(osc, &leaf->mempool);
}

void    tPBSaw_initToPool    (tPBSaw* const osc, tMempool* const mp)
{
    _tMempool* m = *mp;
    _tPBSaw* c = *osc = (_tPBSaw*) mpool_alloc(sizeof(_tPBSaw), m);
    c->mempool = m;
    LEAF* leaf = c->mempool->leaf;
    
    c->inc      =  0.0f;
    c->phase    =  0.0f;
    c->invSampleRate = leaf->invSampleRate;
}

void    tPBSaw_free  (tPBSaw* const osc)
{
    _tPBSaw* c = *osc;
    
    mpool_free((char*)c, c->mempool);
}

float   tPBSaw_tick          (tPBSaw* const osc)
{
    _tPBSaw* c = *osc;
    
    float out = (c->phase * 2.0f) - 1.0f;
    out -= LEAF_poly_blep(c->phase, c->inc);
    
    c->phase += c->inc - (int)c->inc;
    if (c->phase >= 1.0f) c->phase -= 1.0f;
    if (c->phase < 0.0f) c->phase += 1.0f;
    
    return out;
}

void    tPBSaw_setFreq       (tPBSaw* const osc, float freq)
{
    _tPBSaw* c = *osc;
    
    c->freq  = freq;
    c->inc = freq * c->invSampleRate;
}

void    tPBSaw_setSampleRate (tPBSaw* const osc, float sr)
{
    _tPBSaw* c = *osc;
    
    c->invSampleRate = 1.0f/sr;
    tPBSaw_setFreq(osc, c->freq);
}

//========================================================================
/* Phasor */

void    tPhasor_init(tPhasor* const ph, LEAF* const leaf)
{
    tPhasor_initToPool(ph, &leaf->mempool);
}

void    tPhasor_initToPool  (tPhasor* const ph, tMempool* const mp)
{
    _tMempool* m = *mp;
    _tPhasor* p = *ph = (_tPhasor*) mpool_alloc(sizeof(_tPhasor), m);
    p->mempool = m;
    LEAF* leaf = p->mempool->leaf;
    
    p->phase = 0.0f;
    p->inc = 0.0f;
    p->phaseDidReset = 0;
    p->invSampleRate = leaf->invSampleRate;
}

void    tPhasor_free (tPhasor* const ph)
{
    _tPhasor* p = *ph;
    
    mpool_free((char*)p, p->mempool);
}

void    tPhasor_setFreq(tPhasor* const ph, float freq)
{
    _tPhasor* p = *ph;

    p->freq  = freq;
    
    p->inc = freq * p->invSampleRate;
    p->inc -= (int)p->inc;
}

float   tPhasor_tick(tPhasor* const ph)
{
    _tPhasor* p = *ph;
    
    p->phase += p->inc;

    p->phaseDidReset = 0;
    if (p->phase >= 1.0f)
    {
        p->phaseDidReset = 1;
        p->phase -= 1.0f;
    }
    else if (p->phase < 0.0f)
    {
        p->phaseDidReset = 1;
        p->phase += 1.0f;
    }
    
    return p->phase;
}

void     tPhasor_setSampleRate (tPhasor* const ph, float sr)
{
    _tPhasor* p = *ph;
    
    p->invSampleRate = 1.0f/sr;
    tPhasor_setFreq(ph, p->freq);
};

//========================================================================
/* Noise */
void    tNoise_init(tNoise* const ns, NoiseType type, LEAF* const leaf)
{
    tNoise_initToPool(ns, type, &leaf->mempool);
}

void    tNoise_initToPool   (tNoise* const ns, NoiseType type, tMempool* const mp)
{
    _tMempool* m = *mp;
    _tNoise* n = *ns = (_tNoise*) mpool_alloc(sizeof(_tNoise), m);
    n->mempool = m;
    LEAF* leaf = n->mempool->leaf;
    
    n->type = type;
    n->rand = leaf->random;
}

void    tNoise_free (tNoise* const ns)
{
    _tNoise* n = *ns;
    
    mpool_free((char*)n, n->mempool);
}

float   tNoise_tick(tNoise* const ns)
{
    _tNoise* n = *ns;
    
    float rand = (n->rand() * 2.0f) - 1.0f;
    
    if (n->type == PinkNoise)
    {
        float tmp;
        n->pinkb0 = 0.99765f * n->pinkb0 + rand * 0.0990460f;
        n->pinkb1 = 0.96300f * n->pinkb1 + rand * 0.2965164f;
        n->pinkb2 = 0.57000f * n->pinkb2 + rand * 1.0526913f;
        tmp = n->pinkb0 + n->pinkb1 + n->pinkb2 + rand * 0.1848f;
        return (tmp * 0.05f);
    }
    else // WhiteNoise
    {
        return rand;
    }
}

//=================================================================================
/* Neuron */

void tNeuron_init(tNeuron* const nr, LEAF* const leaf)
{
    tNeuron_initToPool(nr, &leaf->mempool);
}

void tNeuron_initToPool  (tNeuron* const nr, tMempool* const mp)
{
    _tMempool* m = *mp;
    _tNeuron* n = *nr = (_tNeuron*) mpool_alloc(sizeof(_tNeuron), m);
    n->mempool = m;
    LEAF* leaf = n->mempool->leaf;

    tPoleZero_initToPool(&n->f, mp);
    
    tPoleZero_setBlockZero(&n->f, 0.99f);
    
    n->invSampleRate = leaf->invSampleRate;
    n->timeStep = (44100.0f * n->invSampleRate) / 50.0f;
    
    n->current = 0.0f; // 100.0f for sound
    n->voltage = 0.0f;
    
    n->mode = NeuronNormal;
    
    n->P[0] = 0.0f;
    n->P[1] = 0.0f;
    n->P[2] = 1.0f;
    
    n->V[0] = -12.0f;
    n->V[1] = 115.0f;
    n->V[2] = 10.613f;
    
    n->gK = 36.0f;
    n->gN = 120.0f;
    n->gL = 0.3f;
    n->C = 1.0f;
    
    n->rate[2] = n->gL/n->C;
}

void    tNeuron_free (tNeuron* const nr)
{
    _tNeuron* n = *nr;
    
    tPoleZero_free(&n->f);
    mpool_free((char*)n, n->mempool);
}

void   tNeuron_reset(tNeuron* const nr)
{
    _tNeuron* n = *nr;
    
    tPoleZero_setBlockZero(&n->f, 0.99f);
    
    n->timeStep = (44100.0f * n->invSampleRate) / 50.0f;
    
    n->current = 0.0f; // 100.0f for sound
    n->voltage = 0.0f;
    
    n->mode = NeuronNormal;
    
    n->P[0] = 0.0f;
    n->P[1] = 0.0f;
    n->P[2] = 1.0f;
    
    n->V[0] = -12.0f;
    n->V[1] = 115.0f;
    n->V[2] = 10.613f;
    
    n->gK = 36.0f;
    n->gN = 120.0f;
    n->gL = 0.3f;
    n->C = 1.0f;
    
    n->rate[2] = n->gL/n->C;
}

void tNeuron_setV1(tNeuron* const nr, float V1)
{
    _tNeuron* n = *nr;
    n->V[0] = V1;
}

void tNeuron_setV2(tNeuron* const nr, float V2)
{
    _tNeuron* n = *nr;
    n->V[1] = V2;
}

void tNeuron_setV3(tNeuron* const nr, float V3)
{
    _tNeuron* n = *nr;
    n->V[2] = V3;
}

void tNeuron_setTimeStep(tNeuron* const nr, float timeStep)
{
    _tNeuron* n = *nr;
    n->timeStep = (44100.0f * n->invSampleRate) * timeStep;
}

void tNeuron_setK(tNeuron* const nr, float K)
{
    _tNeuron* n = *nr;
    n->gK = K;
}

void tNeuron_setL(tNeuron* const nr, float L)
{
    _tNeuron* n = *nr;
    n->gL = L;
    n->rate[2] = n->gL/n->C;
}

void tNeuron_setN(tNeuron* const nr, float N)
{
    _tNeuron* n = *nr;
    n->gN = N;
}

void tNeuron_setC(tNeuron* const nr, float C)
{
    _tNeuron* n = *nr;
    n->C = C;
    n->rate[2] = n->gL/n->C;
}

float tNeuron_tick(tNeuron* const nr)
{
    _tNeuron* n = *nr;
    
    float output = 0.0f;
    float voltage = n->voltage;
    
    n->alpha[0] = (0.01f * (10.0f - voltage)) / (expf((10.0f - voltage)/10.0f) - 1.0f);
    n->alpha[1] = (0.1f * (25.0f-voltage)) / (expf((25.0f-voltage)/10.0f) - 1.0f);
    n->alpha[2] = (0.07f * expf((-1.0f * voltage)/20.0f));
    
    n->beta[0] = (0.125f * expf((-1.0f* voltage)/80.0f));
    n->beta[1] = (4.0f * expf((-1.0f * voltage)/18.0f));
    n->beta[2] = (1.0f / (expf((30.0f-voltage)/10.0f) + 1.0f));
    
    for (int i = 0; i < 3; i++)
    {
        n->P[i] = (n->alpha[i] * n->timeStep) + ((1.0f - ((n->alpha[i] + n->beta[i]) * n->timeStep)) * n->P[i]);
        
        if (n->P[i] > 1.0f)         n->P[i] = 0.0f;
        else if (n->P[i] < -1.0f)   n->P[i] = 0.0f;
    }
    // rate[0]= k ; rate[1] = Na ; rate[2] = l
    n->rate[0] = ((n->gK * powf(n->P[0], 4.0f)) / n->C);
    n->rate[1] = ((n->gN * powf(n->P[1], 3.0f) * n->P[2]) / n->C);
    
    //calculate the final membrane voltage based on the computed variables
    n->voltage = voltage +
    (n->timeStep * n->current / n->C) -
    (n->timeStep * ( n->rate[0] * (voltage - n->V[0]) + n->rate[1] * (voltage - n->V[1]) + n->rate[2] * (voltage - n->V[2])));
    
    if (n->mode == NeuronTanh)
    {
        n->voltage = 100.0f * tanhf(0.01f * n->voltage);
    }
    else if (n->mode == NeuronAaltoShaper)
    {
        float shapeVoltage = 0.01f * n->voltage;
        
        float w, c, xc, xc2, xc4;
        
        float sqrt8 = 2.82842712475f;
        
        float wscale = 1.30612244898f;
        float m_drive = 1.0f;
        
        xc = LEAF_clip(-sqrt8, shapeVoltage, sqrt8);
        
        xc2 = xc*xc;
        
        c = 0.5f * shapeVoltage * (3.0f - (xc2));
        
        xc4 = xc2 * xc2;
        
        w = (1.0f - xc2 * 0.25f + xc4 * 0.015625f) * wscale;
        
        shapeVoltage = w * (c + 0.05f * xc2) * (m_drive + 0.75f);
        
        n->voltage = 100.0f * shapeVoltage;
    }
    
    
    if (n->voltage > 100.0f)  n->voltage = 100.0f;
    else if (n->voltage < -100.) n->voltage = -100.0f;
    
    //(inputCurrent + (voltage - ((voltage * timeStep) / timeConstant)) + P[0] + P[1] + P[2]) => voltage;
    // now we should have a result
    //set the output voltage to the "step" ugen, which controls the DAC.
    output = n->voltage * 0.01f; // volts
    
    output = tPoleZero_tick(&n->f, output);
    
    return output;
    
}

void tNeuron_setMode  (tNeuron* const nr, NeuronMode mode)
{
    _tNeuron* n = *nr;
    n->mode = mode;
}

void tNeuron_setCurrent  (tNeuron* const nr, float current)
{
    _tNeuron* n = *nr;
    n->current = current;
}

void tNeuron_setSampleRate (tNeuron* const nr, float sr)
{
    _tNeuron* n = *nr;
    n->invSampleRate = 1.0f/sr;
    n->timeStep = (44100.0f * n->invSampleRate) / 50.0f;
}

//----------------------------------------------------------------------------------------------------------

void tMBPulse_init(tMBPulse* const osc, LEAF* const leaf)
{
    tMBPulse_initToPool(osc, &leaf->mempool);
}
                          
void tMBPulse_initToPool(tMBPulse* const osc, tMempool* const pool)
{
    _tMempool* m = *pool;
    _tMBPulse* c = *osc = (_tMBPulse*) mpool_alloc(sizeof(_tMBPulse), m);
    c->mempool = m;
    LEAF* leaf = c->mempool->leaf;
    
    c->invSampleRate = leaf->invSampleRate;
    
    c->_init = true;
    c->amp = 1.0f;
    c->freq = 440.f;
    c->lastsyncin = 0.0f;
    c->sync = 0.0f;
    c->syncdir = 1.0f;
    c->softsync = 0;
    c->waveform = 0.0f;
    c->_z = 0.0f;
    c->_j = 0;
    memset (c->_f, 0, (FILLEN + STEP_DD_PULSE_LENGTH) * sizeof (float));
}

void tMBPulse_free(tMBPulse* const osc)
{
    _tMBPulse* c = *osc;
    mpool_free((char*)c, c->mempool);
}

float tMBPulse_tick(tMBPulse* const osc)
{
    _tMBPulse* c = *osc;
    
    int    j, k;
    float  freq, sync;
    float  a, b, p, w, x, z, sw;
    
    sync = c->sync;
    freq = c->freq;
    p = c->_p;  /* phase [0, 1) */
    w = c->_w;  /* phase increment */
    b = c->_b;  /* duty cycle (0, 1) */
    x = c->_x;  /* temporary output variable */
    z = c->_z;  /* low pass filter state */
    j = c->_j;  /* index into buffer _f */
    k = c->_k;  /* output state, 0 = high (0.5f), 1 = low (-0.5f) */
    //
    if (c->_init) {
        p = 0.0f;
        
        w = freq * c->invSampleRate;
        b = 0.5f * (1.0f + c->waveform );
        
        /* for variable-width rectangular wave, we could do DC compensation with:
         *     x = 1.0f - b;
         * but that doesn't work well with highly modulated hard sync.  Instead,
         * we keep things in the range [-0.5f, 0.5f]. */
        x = 0.5f;
        /* if we valued alias-free startup over low startup time, we could do:
         *   p -= w;
         *   place_step_dd(_f, j, 0.0f, w, 0.5f); */
        k = 0;
        c->_init = false;
    }
    //
    //    a = 0.2 + 0.8 * vco->_port [FILT];
    a = 0.5f; // when a = 1, LPfilter is disabled
    
    w = freq * c->invSampleRate;
    b = 0.5f * (1.0f + c->waveform);

    if (sync > 0.0f && c->softsync > 0) c->syncdir = -c->syncdir;
    
    sw = w * c->syncdir;
    p += sw - (int)sw;
    
    if (sync > 0.0f && c->softsync == 0) {  /* sync to master */
        float eof_offset = sync * sw;
        float p_at_reset = p - eof_offset;
    
        if (sw > 0) p = eof_offset;
        else if (sw < 0) p = 1.0f - eof_offset;
        
        /* place any DDs that may have occurred in subsample before reset */
        if (!k) {
            if (sw > 0)
            {
                if (p_at_reset >= b) {
                    place_step_dd(c->_f, j, p_at_reset - b + eof_offset, sw, -1.0f);
                    k = 1;
                    x = -0.5f;
                }
                if (p_at_reset >= 1.0f) {
                    p_at_reset -= 1.0f;
                    place_step_dd(c->_f, j, p_at_reset + eof_offset, sw, 1.0f);
                    k = 0;
                    x = 0.5f;
                }
            }
            else if (sw < 0)
            {
                if (p_at_reset < 0.0f) {
                    p_at_reset += 1.0f;
                    place_step_dd(c->_f, j, 1.0f - p_at_reset - eof_offset, -sw, -1.0f);
                    k = 1;
                    x = -0.5f;
                }
                if (k && p_at_reset < b) {
                    place_step_dd(c->_f, j, b - p_at_reset - eof_offset, -sw, 1.0f);
                    k = 0;
                    x = 0.5f;
                }
            }
        } else {
            if (sw > 0)
            {
                if (p_at_reset >= 1.0f) {
                    p_at_reset -= 1.0f;
                    place_step_dd(c->_f, j, p_at_reset + eof_offset, sw, 1.0f);
                    k = 0;
                    x = 0.5f;
                }
                if (!k && p_at_reset >= b) {
                    place_step_dd(c->_f, j, p_at_reset - b + eof_offset, sw, -1.0f);
                    k = 1;
                    x = -0.5f;
                }
            }
            else if (sw < 0)
            {
                if (p_at_reset < b) {
                    place_step_dd(c->_f, j, b - p_at_reset - eof_offset, -sw, 1.0f);
                    k = 0;
                    x = 0.5f;
                }
                if (p_at_reset < 0.0f) {
                    p_at_reset += 1.0f;
                    place_step_dd(c->_f, j, 1.0f - p_at_reset - eof_offset, -sw, -1.0f);
                    k = 1;
                    x = -0.5f;
                }
            }
        }
        
        /* now place reset DD */
        if (sw > 0)
        {
            if (k) {
                place_step_dd(c->_f, j, p, sw, 1.0f);
                k = 0;
                x = 0.5f;
            }
            if (p >= b) {
                place_step_dd(c->_f, j, p - b, sw, -1.0f);
                k = 1;
                x = -0.5f;
            }
        }
        else if (sw < 0)
        {
            if (!k) {
                place_step_dd(c->_f, j, 1.0f - p, -sw, -1.0f);
                k = 1;
                x = -0.5f;
            }
            if (p < b) {
                place_step_dd(c->_f, j, b - p, -sw, 1.0f);
                k = 0;
                x = 0.5f;
            }
        }
    } else if (!k) {  /* normal operation, signal currently high */
        
        if (sw > 0)
        {
            if (p >= b) {
                place_step_dd(c->_f, j, p - b, sw, -1.0f);
                k = 1;
                x = -0.5f;
            }
            if (p >= 1.0f) {
                p -= 1.0f;
                place_step_dd(c->_f, j, p, sw, 1.0f);
                k = 0;
                x = 0.5f;
            }
        }
        else if (sw < 0)
        {
            if (p < 0.0f) {
                p += 1.0f;
                place_step_dd(c->_f, j, 1.0f - p, -sw, -1.0f);
                k = 1;
                x = -0.5f;
            }
            if (k && p < b) {
                place_step_dd(c->_f, j, b - p, -sw, 1.0f);
                k = 0;
                x = 0.5f;
            }
        }
        
    } else {  /* normal operation, signal currently low */
        
        if (sw > 0)
        {
            if (p >= 1.0f) {
                p -= 1.0f;
                place_step_dd(c->_f, j, p, sw, 1.0f);
                k = 0;
                x = 0.5f;
            }
            if (!k && p >= b) {
                place_step_dd(c->_f, j, p - b, sw, -1.0f);
                k = 1;
                x = -0.5f;
            }
        }
        else if (sw < 0)
        {
            if (p < b) {
                place_step_dd(c->_f, j, b - p, -sw, 1.0f);
                k = 0;
                x = 0.5f;
            }
            if (p < 0.0f) {
                p += 1.0f;
                place_step_dd(c->_f, j, 1.0f - p, -sw, -1.0f);
                k = 1;
                x = -0.5f;
            }
        }
    }
    c->_f[j + DD_SAMPLE_DELAY] += x;
    
    z += a * (c->_f[j] - z);
    c->out = c->amp * z;
    
    if (++j == FILLEN)
    {
        j = 0;
        memcpy (c->_f, c->_f + FILLEN, STEP_DD_PULSE_LENGTH * sizeof (float));
        memset (c->_f + STEP_DD_PULSE_LENGTH, 0,  FILLEN * sizeof (float));
    }
    
    c->_p = p;
    c->_w = w;
    c->_b = b;
    c->_x = x;
    c->_z = z;
    c->_j = j;
    c->_k = k;
    
    return c->out;
}

void tMBPulse_setFreq(tMBPulse* const osc, float f)
{
    _tMBPulse* c = *osc;
    c->freq = f;
}

void tMBPulse_setWidth(tMBPulse* const osc, float w)
{
    _tMBPulse* c = *osc;
    c->waveform = w;
}

float tMBPulse_sync(tMBPulse* const osc, float value)
{
    _tMBPulse* c = *osc;
    
    //based on https://github.com/VCVRack/Fundamental/blob/5799ee2a9b21492b42ebcb9b65d5395ef5c1cbe2/src/VCO.cpp#L123
    float last = c->lastsyncin;
    float delta = value - last;
    float crossing = -last / delta;
    c->lastsyncin = value;
    if ((0.f < crossing) && (crossing <= 1.f) && (value >= 0.f))
        c->sync = (1.f - crossing) * delta;
    else c->sync = 0.f;
    
    return value;
}

void tMBPulse_setSyncMode(tMBPulse* const osc, int hardOrSoft)
{
    _tMBPulse* c = *osc;
    c->softsync = hardOrSoft > 0 ? 1 : 0;
}

void tMBPulse_setSampleRate(tMBPulse* const osc, float sr)
{
    _tMBPulse* c = *osc;
    c->invSampleRate = 1.0f/sr;
}

//==========================================================================================================
//==========================================================================================================

void tMBTriangle_init(tMBTriangle* const osc, LEAF* const leaf)
{
    tMBTriangle_initToPool(osc, &leaf->mempool);
}

void tMBTriangle_initToPool(tMBTriangle* const osc, tMempool* const pool)
{
    _tMempool* m = *pool;
    _tMBTriangle* c = *osc = (_tMBTriangle*) mpool_alloc(sizeof(_tMBTriangle), m);
    c->mempool = m;
    LEAF* leaf = c->mempool->leaf;
    
    c->invSampleRate = leaf->invSampleRate;
    c->amp = 1.0f;
    c->freq = 440.f;
    c->lastsyncin = 0.0f;
    c->sync = 0.0f;
    c->syncdir = 1.0f;
    c->softsync = 0;
    c->waveform = 0.0f;
    c->_init = true;
    c->_z = 0.0f;
    c->_j = 0;
    memset (c->_f, 0, (FILLEN + STEP_DD_PULSE_LENGTH) * sizeof (float));
}

void tMBTriangle_free(tMBTriangle* const osc)
{
    _tMBTriangle* c = *osc;
    mpool_free((char*)c, c->mempool);
}

float tMBTriangle_tick(tMBTriangle* const osc)
{
    _tMBTriangle* c = *osc;
    
    int    j, k, dir;
    float  freq, sync;
    float  a, b, b1, p, w, sw, x, z;
    
    sync = c->sync;
    dir = c->syncdir;
    freq = c->freq;
    p = c->_p;  /* phase [0, 1) */
    w = c->_w;  /* phase increment */
    b = c->_b;  /* duty cycle (0, 1) */
    z = c->_z;  /* low pass filter state */
    j = c->_j;  /* index into buffer _f */
    k = c->_k;  /* output state, 0 = positive slope, 1 = negative slope */
    
    if (c->_init) {
        //        w = (exp2ap (freq[1] + vco->_port[OCTN] + vco->_port[TUNE] + expm[1] * vco->_port[EXPG] + 8.03136)
        //                + 1e3 * linm[1] * vco->_port[LING]) / SAMPLERATE;
        w = freq * c->invSampleRate;
        b = 0.5f * (1.0f + c->waveform);
        p = 0.5f * b;
        /* if we valued alias-free startup over low startup time, we could do:
         *   p -= w;
         *   place_slope_dd(_f, j, 0.0f, w, 1.0f / b); */
        k = 0;
        c->_init = false;
    }
    
    //    a = 0.2 + 0.8 * vco->_port [FILT];
    a = 0.5f; // when a = 1, LPfilter is disabled
    
    w = freq * c->invSampleRate;
    b = 0.5f * (1.0f + c->waveform);
    b1 = 1.0f - b;
    
    if (sync > 0.0f && c->softsync > 0) c->syncdir = -c->syncdir;
    
    sw = w * c->syncdir;
    p += sw - (int)sw;
    
    if (sync > 0.0f && c->softsync == 0) {  /* sync to master */
        float eof_offset = sync * sw;
        float p_at_reset = p - eof_offset;
        
        if (sw > 0) p = eof_offset;
        else if (sw < 0) p = 1.0f - eof_offset;
        //
        /* place any DDs that may have occurred in subsample before reset */
            
        if (!k) {
            x = -0.5f + p_at_reset / b;
            if (sw > 0)
            {
                if (p_at_reset >= b) {
                    x = 0.5f - (p_at_reset - b) / b1;
                    place_slope_dd(c->_f, j, p_at_reset - b + eof_offset, sw, -1.0f / b1 - 1.0f / b);
                    k = 1;
                }
                if (p_at_reset >= 1.0f) {
                    p_at_reset -= 1.0f;
                    x = -0.5f + p_at_reset / b;
                    place_slope_dd(c->_f, j, p_at_reset + eof_offset, sw, 1.0f / b + 1.0f / b1);
                    k = 0;
                }
            }
            else if (sw < 0)
            {
                if (p_at_reset < 0.0f) {
                    p_at_reset += 1.0f;
                    x = 0.5f - (p_at_reset - b) / b1;
                    place_slope_dd(c->_f, j, 1.0f - p_at_reset - eof_offset, -sw, 1.0f / b + 1.0f / b1);
                    k = 1;
                }
                if (k && p_at_reset < b) {
                    x = -0.5f + p_at_reset / b;
                    place_slope_dd(c->_f, j, b - p_at_reset - eof_offset, -sw, -1.0f / b1 - 1.0f / b);
                    k = 0;
                }
            }
        } else {
            x = 0.5f - (p_at_reset - b) / b1;
            if (sw > 0)
            {
                if (p_at_reset >= 1.0f) {
                    p_at_reset -= 1.0f;
                    x = -0.5f + p_at_reset / b;
                    place_slope_dd(c->_f, j, p_at_reset + eof_offset, sw, 1.0f / b + 1.0f / b1);
                    k = 0;
                }
                if (!k && p_at_reset >= b) {
                    x = 0.5f - (p_at_reset - b) / b1;
                    place_slope_dd(c->_f, j, p_at_reset - b + eof_offset, sw, -1.0f / b1 - 1.0f / b);
                    k = 1;
                }
            }
            else if (sw < 0)
            {
                if (p_at_reset < b) {
                    x = -0.5f + p_at_reset / b;
                    place_slope_dd(c->_f, j, b - p_at_reset - eof_offset, -sw, -1.0f / b1 - 1.0f / b);
                    k = 0;
                }
                if (p_at_reset < 0.0f) {
                    p_at_reset += 1.0f;
                    x = 0.5f - (p_at_reset - b) / b1;
                    place_slope_dd(c->_f, j, 1.0f - p_at_reset - eof_offset, -sw, 1.0f / b + 1.0f / b1);
                    k = 1;
                }
            }
        }
        
        /* now place reset DDs */
        if (sw > 0)
        {
            if (k)
                place_slope_dd(c->_f, j, p, sw, 1.0f / b + 1.0f / b1);
            place_step_dd(c->_f, j, p, sw, -0.5f - x);
            x = -0.5f + p / b;
            k = 0;
            if (p >= b) {
                x = 0.5f - (p - b) / b1;
                place_slope_dd(c->_f, j, p - b, sw, -1.0f / b1 - 1.0f / b);
                k = 1;
            }
        }
        else if (sw < 0)
        {
            if (!k)
                place_slope_dd(c->_f, j, 1.0f - p, -sw, 1.0f / b + 1.0f / b1);
            place_step_dd(c->_f, j, 1.0f - p, -sw, -0.5f - x);
            x = 0.5f - (p - b) / b1;
            k = 1;
            if (p < b) {
                x = -0.5f + p / b;
                place_slope_dd(c->_f, j, b - p, -sw, -1.0f / b1 - 1.0f / b);
                k = 0;
            }
        }
    } else if (!k) {  /* normal operation, slope currently up */
        
        x = -0.5f + p / b;
        if (sw > 0)
        {
            if (p >= b) {
                x = 0.5f - (p - b) / b1;
                place_slope_dd(c->_f, j, p - b, sw, -1.0f / b1 - 1.0f / b);
                k = 1;
            }
            if (p >= 1.0f) {
                p -= 1.0f;
                x = -0.5f + p / b;
                place_slope_dd(c->_f, j, p, sw, 1.0f / b + 1.0f / b1);
                k = 0;
            }
        }
        else if (sw < 0)
        {
            if (p < 0.0f) {
                p += 1.0f;
                x = 0.5f - (p - b) / b1;
                place_slope_dd(c->_f, j, 1.0f - p, -sw, 1.0f / b + 1.0f / b1);
                k = 1;
            }
            if (k && p < b) {
                x = -0.5f + p / b;
                place_slope_dd(c->_f, j, b - p, -sw, -1.0f / b1 - 1.0f / b);
                k = 0;
            }
        }
        
    } else {  /* normal operation, slope currently down */
        
        x = 0.5f - (p - b) / b1;
        if (sw > 0)
        {
            if (p >= 1.0f) {
                p -= 1.0f;
                x = -0.5f + p / b;
                place_slope_dd(c->_f, j, p, sw, 1.0f / b + 1.0f / b1);
                k = 0;
            }
            if (!k && p >= b) {
                x = 0.5f - (p - b) / b1;
                place_slope_dd(c->_f, j, p - b, sw, -1.0f / b1 - 1.0f / b);
                k = 1;
            }
        }
        else if (sw < 0)
        {
            if (p < b) {
                x = -0.5f + p / b;
                place_slope_dd(c->_f, j, b - p, -sw, -1.0f / b1 - 1.0f / b);
                k = 0;
            }
            if (p < 0.0f) {
                p += 1.0f;
                x = 0.5f - (p - b) / b1;
                place_slope_dd(c->_f, j, 1.0f - p, -sw, 1.0f / b + 1.0f / b1);
                k = 1;
            }
        }
    }
    c->_f[j + DD_SAMPLE_DELAY] += x;
    
    z += a * (c->_f[j] - z);
    c->out = c->amp * z;
    
    if (++j == FILLEN)
    {
        j = 0;
        memcpy (c->_f, c->_f + FILLEN, STEP_DD_PULSE_LENGTH * sizeof (float));
        memset (c->_f + STEP_DD_PULSE_LENGTH, 0,  FILLEN * sizeof (float));
    }
    
    c->_p = p;
    c->_w = w;
    c->_b = b;
    c->_z = z;
    c->_j = j;
    c->_k = k;
    
    return c->out;
}

void tMBTriangle_setFreq(tMBTriangle* const osc, float f)
{
    _tMBTriangle* c = *osc;
    c->freq = f;
}

void tMBTriangle_setWidth(tMBTriangle* const osc, float w)
{
    _tMBTriangle* c = *osc;
    c->waveform = w;
}

float tMBTriangle_sync(tMBTriangle* const osc, float value)
{
    _tMBTriangle* c = *osc;
    
    //based on https://github.com/VCVRack/Fundamental/blob/5799ee2a9b21492b42ebcb9b65d5395ef5c1cbe2/src/VCO.cpp#L123
    float last = c->lastsyncin;
    float delta = value - last;
    float crossing = -last / delta;
    c->lastsyncin = value;
    if ((0.f < crossing) && (crossing <= 1.f) && (value >= 0.f))
        c->sync = (1.f - crossing) * delta;
    else c->sync = 0.f;
    
    return value;
}

void tMBTriangle_setSyncMode(tMBTriangle* const osc, int hardOrSoft)
{
    _tMBTriangle* c = *osc;
    c->softsync = hardOrSoft > 0 ? 1 : 0;
}

void tMBTriangle_setSampleRate(tMBTriangle* const osc, float sr)
{
    _tMBTriangle* c = *osc;
    c->invSampleRate = 1.0f/sr;
}

//==========================================================================================================
//==========================================================================================================

void tMBSaw_init(tMBSaw* const osc, LEAF* const leaf)
{
    tMBSaw_initToPool(osc, &leaf->mempool);
}

void tMBSaw_initToPool(tMBSaw* const osc, tMempool* const pool)
{
    _tMempool* m = *pool;
    _tMBSaw* c = *osc = (_tMBSaw*) mpool_alloc(sizeof(_tMBSaw), m);
    c->mempool = m;
    LEAF* leaf = c->mempool->leaf;
    
    c->invSampleRate = leaf->invSampleRate;
    c->_init = true;
    c->amp = 1.0f;
    c->freq = 440.f;
    c->lastsyncin = 0.0f;
    c->sync = 0.0f;
    c->syncdir = 1.0f;
    c->softsync = 0;
    c->_z = 0.0f;
    c->_j = 0;
    memset (c->_f, 0, (FILLEN + STEP_DD_PULSE_LENGTH) * sizeof (float));
}

void tMBSaw_free(tMBSaw* const osc)
{
    _tMBSaw* c = *osc;
    mpool_free((char*)c, c->mempool);
}

float tMBSaw_tick(tMBSaw* const osc)
{
    _tMBSaw* c = *osc;
    
    int    j;
    float  freq, sync;
    float  a, p, w, sw, z;
    
    sync = c->sync;
    freq = c->freq;
    
    p = c->_p;  /* phase [0, 1) */
    w = c->_w;  /* phase increment */
    z = c->_z;  /* low pass filter state */
    j = c->_j;  /* index into buffer _f */
    
    if (c->_init) {
        p = 0.5f;
        w = freq * c->invSampleRate;
        
        /* if we valued alias-free startup over low startup time, we could do:
         *   p -= w;
         *   place_slope_dd(_f, j, 0.0f, w, -1.0f); */
        c->_init = false;
    }
    
    //a = 0.2 + 0.8 * vco->_port [FILT];
    a = 0.5f; // when a = 1, LPfilter is disabled
    
    w = freq * c->invSampleRate;

    if (sync > 0.0f && c->softsync > 0) c->syncdir = -c->syncdir;
    // Should insert minblep for softsync?
    //            if (p_at_reset >= 1.0f) {
    //                p_at_reset -= (int)p_at_reset;
    //                place_slope_dd(c->_f, j, p_at_reset + eof_offset, sw, 2.0f);
    //            }
    //            if (p_at_reset < 0.0f) {
    //                p_at_reset += 1.0f - (int)p_at_reset;
    //                place_slope_dd(c->_f, j, 1.0f - p_at_reset - eof_offset, -sw, -2.0f);
    //            }
    //            if (sw > 0) place_slope_dd(c->_f, j, p, sw, 2.0f);
    //            else if (sw < 0) place_slope_dd(c->_f, j, 1.0f - p, -sw, -2.0f);
    
    sw = w * c->syncdir;
    p += sw - (int)sw;
    
    if (sync > 0.0f && c->softsync == 0) {  /* sync to master */
        float eof_offset = sync * sw;
        float p_at_reset = p - eof_offset;

        if (sw > 0) p = eof_offset;
        else if (sw < 0) p = 1.0f - eof_offset;
        
        /* place any DD that may have occurred in subsample before reset */
        if (p_at_reset >= 1.0f) {
            p_at_reset -= 1.0f;
            place_step_dd(c->_f, j, p_at_reset + eof_offset, sw, 1.0f);
        }
        if (p_at_reset < 0.0f) {
            p_at_reset += 1.0f;
            place_step_dd(c->_f, j, 1.0f - p_at_reset - eof_offset, -sw, -1.0f);
        }
        
        /* now place reset DD */
        if (sw > 0)
            place_step_dd(c->_f, j, p, sw, p_at_reset);
        else if (sw < 0)
            place_step_dd(c->_f, j, 1.0f - p, -sw, -p_at_reset);

    } else if (p >= 1.0f) {  /* normal phase reset */
        p -= 1.0f;
        place_step_dd(c->_f, j, p, sw, 1.0f);
        
    } else if (p < 0.0f) {
        p += 1.0f;
        place_step_dd(c->_f, j, 1.0f - p, -sw, -1.0f);
    }
    c->_f[j + DD_SAMPLE_DELAY] += 0.5f - p;
    
    z += a * (c->_f[j] - z); // LP filtering
    c->out = c->amp * z;
    
    if (++j == FILLEN)
    {
        j = 0;
        memcpy (c->_f, c->_f + FILLEN, STEP_DD_PULSE_LENGTH * sizeof (float));
        memset (c->_f + STEP_DD_PULSE_LENGTH, 0,  FILLEN * sizeof (float));
    }
    
    c->_p = p;
    c->_w = w;
    c->_z = z;
    c->_j = j;
    
    return c->out;
}

void tMBSaw_setFreq(tMBSaw* const osc, float f)
{
    _tMBSaw* c = *osc;
    c->freq = f;
}

float tMBSaw_sync(tMBSaw* const osc, float value)
{
    _tMBSaw* c = *osc;
    
    //based on https://github.com/VCVRack/Fundamental/blob/5799ee2a9b21492b42ebcb9b65d5395ef5c1cbe2/src/VCO.cpp#L123
    float last = c->lastsyncin;
    float delta = value - last;
    float crossing = -last / delta;
    c->lastsyncin = value;
    if ((0.f < crossing) && (crossing <= 1.f) && (value >= 0.f))
        c->sync = (1.f - crossing) * delta;
    else c->sync = 0.f;
    
    return value;
}

void tMBSaw_setSyncMode(tMBSaw* const osc, int hardOrSoft)
{
    _tMBSaw* c = *osc;
    c->softsync = hardOrSoft > 0 ? 1 : 0;
}

void tMBSaw_setSampleRate(tMBSaw* const osc, float sr)
{
    _tMBSaw* c = *osc;
    c->invSampleRate = 1.0f/sr;
}


// WaveTable
void    tTable_init(tTable* const cy, float* waveTable, int size, LEAF* const leaf)
{
    tTable_initToPool(cy, waveTable, size, &leaf->mempool);
}

void    tTable_initToPool(tTable* const cy, float* waveTable, int size, tMempool* const mp)
{
    _tMempool* m = *mp;
    _tTable* c = *cy = (_tTable*)mpool_alloc(sizeof(_tTable), m);
    c->mempool = m;
    LEAF* leaf = c->mempool->leaf;
    
    c->waveTable = waveTable;
    c->size = size;
    c->inc = 0.0f;
    c->phase = 0.0f;
    c->invSampleRate = leaf->invSampleRate;
}

void    tTable_free(tTable* const cy)
{
    _tTable* c = *cy;
    
    mpool_free((char*)c, c->mempool);
}

void     tTable_setFreq(tTable* const cy, float freq)
{
    _tTable* c = *cy;
    
    if (!isfinite(freq)) return;
    
    c->freq = freq;
    c->inc = freq * c->invSampleRate;
    c->inc -= (int)c->inc;
}

float   tTable_tick(tTable* const cy)
{
    _tTable* c = *cy;
    float temp;
    int intPart;
    float fracPart;
    float samp0;
    float samp1;
    
    // Phasor increment
    c->phase += c->inc;
    if (c->phase >= 1.0f) c->phase -= 1.0f;
    if (c->phase < 0.0f) c->phase += 1.0f;
    
    // Wavetable synthesis
    
    temp = c->size * c->phase;
    intPart = (int)temp;
    fracPart = temp - (float)intPart;
    samp0 = c->waveTable[intPart];
    if (++intPart >= c->size) intPart = 0;
    samp1 = c->waveTable[intPart];
    
    return (samp0 + (samp1 - samp0) * fracPart);
}

void     tTable_setSampleRate(tTable* const cy, float sr)
{
    _tTable* c = *cy;
    c->invSampleRate = 1.0f/sr;
    tTable_setFreq(cy, c->freq);
}

void tWaveTable_init(tWaveTable* const cy, float* table, int size, float maxFreq, LEAF* const leaf)
{
    tWaveTable_initToPool(cy, table, size, maxFreq, &leaf->mempool);
}

void tWaveTable_initToPool(tWaveTable* const cy, float* table, int size, float maxFreq, tMempool* const mp)
{
    _tMempool* m = *mp;
    _tWaveTable* c = *cy = (_tWaveTable*) mpool_alloc(sizeof(_tWaveTable), m);
    c->mempool = m;
    LEAF* leaf = c->mempool->leaf;
    
    c->sampleRate = leaf->sampleRate;
    
    c->maxFreq = maxFreq;
    
    // Determine base frequency
    c->baseFreq = c->sampleRate / (float) size;
    c->invBaseFreq = 1.0f / c->baseFreq;
    
    // Determine how many tables we need
    // Assume we need at least 2, the fundamental + one to account for setting extra anti aliasing
    c->numTables = 2;
    float f = c->baseFreq;
    while (f < c->maxFreq)
    {
        c->numTables++;
        f *= 2.0f; // pass this multiplier in to set spacing of tables? would need to change setFreq too
    }
    
    c->size = size;
    
    // Allocate memory for the tables
    c->tables = (float**) mpool_alloc(sizeof(float*) * c->numTables, c->mempool);
    c->baseTable = (float*) mpool_alloc(sizeof(float) * c->size, c->mempool);
    c->tables[0] = c->baseTable;
    for (int t = 1; t < c->numTables; ++t)
    {
        c->tables[t] = (float*) mpool_alloc(sizeof(float) * c->size, c->mempool);
    }
    
    // Copy table
    for (int i = 0; i < c->size; ++i)
    {
        c->baseTable[i] = table[i];
    }
    
    // Make bandlimited copies
    f = c->sampleRate * 0.25; //start at half nyquist
    // Not worth going over order 8 I think, and even 8 is only marginally better than 4.
    tButterworth_initToPool(&c->bl, 8, -1.0f, f, mp);
    for (int t = 1; t < c->numTables; ++t)
    {
        tButterworth_setF2(&c->bl, f);
        // Do several passes here to prevent errors at the beginning of the waveform
        // Not sure how many passes to do, seem to need more as the filter cutoff goes down
        // 12 might be excessive but seems to work for now.
        for (int p = 0; p < 12; ++p)
        {
            for (int i = 0; i < c->size; ++i)
            {
                c->tables[t][i] = tButterworth_tick(&c->bl, c->tables[t-1][i]);
            }
        }
        f *= 0.5f; //halve the cutoff for next pass
    }
    tButterworth_free(&c->bl);
}

void tWaveTable_free(tWaveTable* const cy)
{
    _tWaveTable* c = *cy;
    
    for (int t = 0; t < c->numTables; ++t)
    {
        mpool_free((char*)c->tables[t], c->mempool);
    }
    mpool_free((char*)c->tables, c->mempool);
    mpool_free((char*)c, c->mempool);
}

void tWaveTable_setSampleRate(tWaveTable* const cy, float sr)
{
    _tWaveTable* c = *cy;
        
    // Changing the sample rate of a wavetable requires up to partially reinitialize
    for (int t = 1; t < c->numTables; ++t)
    {
        mpool_free((char*)c->tables[t], c->mempool);
    }
    mpool_free((char*)c->tables, c->mempool);
    
    c->sampleRate = sr;
    
    // Determine base frequency
    c->baseFreq = c->sampleRate / (float) c->size;
    c->invBaseFreq = 1.0f / c->baseFreq;
    
    // Determine how many tables we need
    // Assume we need at least 2, the fundamental + one to account for setting extra anti aliasing
    c->numTables = 2;
    float f = c->baseFreq;
    while (f < c->maxFreq)
    {
        c->numTables++;
        f *= 2.0f; // pass this multiplier in to set spacing of tables? would need to change setFreq too
    }
    
    // Allocate memory for the tables
    c->tables = (float**) mpool_alloc(sizeof(float*) * c->numTables, c->mempool);
    c->tables[0] = c->baseTable;
    for (int t = 1; t < c->numTables; ++t)
    {
        c->tables[t] = (float*) mpool_alloc(sizeof(float) * c->size, c->mempool);
    }
    
    // Make bandlimited copies
    f = c->sampleRate * 0.25; //start at half nyquist
    // Not worth going over order 8 I think, and even 8 is only marginally better than 4.
    tButterworth_initToPool(&c->bl, 8, -1.0f, f, &c->mempool);
    tButterworth_setSampleRate(&c->bl, c->sampleRate);
    for (int t = 1; t < c->numTables; ++t)
    {
        tButterworth_setF2(&c->bl, f);
        // Do several passes here to prevent errors at the beginning of the waveform
        // Not sure how many passes to do, seem to need more as the filter cutoff goes down
        // 12 might be excessive but seems to work for now.
        for (int p = 0; p < 12; ++p)
        {
            for (int i = 0; i < c->size; ++i)
            {
                c->tables[t][i] = tButterworth_tick(&c->bl, c->tables[t-1][i]);
            }
        }
        f *= 0.5f; //halve the cutoff for next pass
    }
    tButterworth_free(&c->bl);
}

//=======================================================================================
//=======================================================================================

void tWaveOsc_init(tWaveOsc* const cy, tWaveTable* const table, LEAF* const leaf)
{
    tWaveOsc_initToPool(cy, table, &leaf->mempool);
}

void tWaveOsc_initToPool(tWaveOsc* const cy, tWaveTable* const table, tMempool* const mp)
{
    _tMempool* m = *mp;
    _tWaveOsc* c = *cy = (_tWaveOsc*) mpool_alloc(sizeof(_tWaveOsc), m);
    c->mempool = m;
    LEAF* leaf = c->mempool->leaf;
    
    c->table = *table;

    c->inc = 0.0f;
    c->phase = 0.0f;
    c->phaseOffset = 0.0f;
    c->aa = 0.5f;
    
    c->invSampleRate = leaf->invSampleRate;
    
    tWaveOsc_setFreq(cy, 220);
}

void tWaveOsc_free(tWaveOsc* const cy)
{
    _tWaveOsc* c = *cy;
    mpool_free((char*)c, c->mempool);
}

float tWaveOsc_tick(tWaveOsc* const cy)
{
    _tWaveOsc* c = *cy;
    
    float temp;
    int idx;
    float frac;
    float samp0;
    float samp1;
    
    int size = c->table->size;
    float** tables = c->table->tables;
    
    // Phasor increment
    c->phase += c->inc;
    if (c->phase + c->phaseOffset >= 1.0f) c->phase -= 1.0f;
    if (c->phase + c->phaseOffset < 0.0f) c->phase += 1.0f;
    
    // Wavetable synthesis
    temp = size * (c->phase + c->phaseOffset);
    
    idx = (int)temp;
    frac = temp - (float)idx;
    samp0 = tables[c->oct][idx];
    if (++idx >= size) idx = 0;
    samp1 = tables[c->oct][idx];
    
    float oct0 = (samp0 + (samp1 - samp0) * frac);
    
    idx = (int)temp;
    samp0 = tables[c->oct+1][idx];
    if (++idx >= size) idx = 0;
    samp1 = tables[c->oct+1][idx];
    
    float oct1 = (samp0 + (samp1 - samp0) * frac);
    
    return oct0 + (oct1 - oct0) * c->w;
}

void tWaveOsc_setFreq(tWaveOsc* const cy, float freq)
{
    _tWaveOsc* c = *cy;
    
    c->freq  = freq;
    
    c->inc = c->freq * c->invSampleRate;
    c->inc -= (int)c->inc;
    
    // abs for negative frequencies
    c->w = fabsf(c->freq * c->table->invBaseFreq);
    
    // Probably ok to use a log2 approx here; won't effect tuning at all, just crossfading between octave tables
    c->w = log2f_approx(c->w) + c->aa; // adding an offset here will shift our table selection upward, reducing aliasing but lower high freq fidelity. +1.0f should remove all aliasing
    if (c->w < 0.0f) c->w = 0.0f; // If c->w is < 0.0f, then freq is less than our base freq
    c->oct = (int)c->w;
    c->w -= c->oct;
    
    // When out of range of our tables, this will prevent a crash.
    // Also causes a blip but fine since we're above maxFreq at this point.
    if (c->oct >= c->table->numTables - 1) c->oct = c->table->numTables - 2;
}

void tWaveOsc_setAntiAliasing(tWaveOsc* const cy, float aa)
{
    _tWaveOsc* c = *cy;
    c->aa = aa;
    tWaveOsc_setFreq(cy, c->freq);
}

void tWaveOsc_setPhaseOffset(tWaveOsc* const cy, float phase)
{
    _tWaveOsc* c = *cy;
    c->phaseOffset = phase - (int)phase;
}

void tWaveOsc_setSampleRate(tWaveOsc* const cy, float sr)
{
    _tWaveOsc* c = *cy;
    c->invSampleRate = 1.0f/sr;
    tWaveOsc_setFreq(cy, c->freq);
}

//================================================================================================
//================================================================================================

void    tWaveSynth_init(tWaveSynth* const cy, int numVoices, float** tables, int* sizes,
                        int numTables, float maxFreq, LEAF* const leaf)
{
    tWaveSynth_initToPool(cy, numVoices, tables, sizes, numTables, maxFreq, &leaf->mempool);
}

void    tWaveSynth_initToPool(tWaveSynth* const cy, int numVoices, float** tables, int* sizes,
                              int numTables, float maxFreq, tMempool* const mp)
{
    _tMempool* m = *mp;
    _tWaveSynth* c = *cy = (_tWaveSynth*) mpool_alloc(sizeof(_tWaveSynth), m);
    c->mempool = m;
    
    c->numTables = 0;
    for (int t = 0; t < numTables; ++t)
    {
        if (sizes[t] > 0) c->numTables++;
    }
    
    c->tables = (tWaveTable*) mpool_alloc(sizeof(tWaveTable) * c->numTables, m);
    c->oscs = (tWaveOsc**) mpool_alloc(sizeof(tWaveOsc*) * c->numTables, m);

    c->numVoices = numVoices;
    
    int i = 0;
    for (int t = 0; t < numTables; ++t)
    {
        if (sizes[t] > 0)
        {
            tWaveTable_initToPool(&c->tables[i], tables[t], sizes[t], maxFreq, mp);
            c->oscs[i] = (tWaveOsc*) mpool_alloc(sizeof(tWaveOsc) * c->numVoices, m);
            for (int v = 0; v < c->numVoices; ++v) tWaveOsc_initToPool(&c->oscs[i][v], &c->tables[i], mp);
            i++;
        }
    }
    
    c->g = (float*) mpool_alloc(sizeof(float) * c->numTables, m);
    for (int i = 0; i < c->numTables; ++i)  c->g[i] = 1.0f;
    
    c->index = 0.0f;
    c->maxFreq = maxFreq;
}

void    tWaveSynth_free(tWaveSynth* const cy)
{
    _tWaveSynth* c = *cy;
    
    for (int i = 0; i < c->numTables; ++i)
    {
        tWaveTable_free(&c->tables[i]);
        for (int v = 0; v < c->numVoices; ++v) tWaveOsc_free(&c->oscs[i][v]);
        mpool_free((char*)c->oscs[i], c->mempool);
    }
    mpool_free((char*)c->g, c->mempool);
    mpool_free((char*)c->oscs, c->mempool);
    mpool_free((char*)c->tables, c->mempool);
    mpool_free((char*)c, c->mempool);
}

float   tWaveSynth_tick(tWaveSynth* const cy)
{
    _tWaveSynth* c = *cy;
    
    float f = c->index * (c->numTables - 1);
    
    int o1 = (int)f;
    int o2 = o1 + 1;
    if (c->index >= 1.0f) o2 = o1;
    float mix = f - o1;
    
    float s1 = 0.f, s2 = 0.f;
    for (int t = 0; t < c->numTables; ++t)
    {
        float s0 = 0.f;
        for (int v = 0; v < c->numVoices; ++v) s0 += tWaveOsc_tick(&c->oscs[t][v]);
        if (t == o1) s1 = s0 * c->g[t];
        if (t == o2) s2 = s0 * c->g[t];
    }
    
    // Ideally should determine correlation to get a good equal power fade between tables
    return s1 + (s2 - s1) * mix;
}

float tWaveSynth_tickVoice(tWaveSynth* const cy, int voice)
{
    _tWaveSynth* c = *cy;
    
    float f = c->index * (c->numTables - 1);
    
    int o1 = (int)f;
    int o2 = o1 + 1;
    if (c->index >= 1.0f) o2 = o1;
    float mix = f - o1;
    
    float s1 = 0.f, s2 = 0.f;
    for (int t = 0; t < c->numTables; ++t)
    {
        // Should we tick every voice anyway to preserve relative phases?
        float s0 = tWaveOsc_tick(&c->oscs[t][voice]);
        if (t == o1) s1 = s0 * c->g[t];
        if (t == o2) s2 = s0 * c->g[t];
    }
    
    // Ideally should determine correlation to get a good equal power fade between tables
    return s1 + (s2 - s1) * mix;
}

void tWaveSynth_setFreq(tWaveSynth* const cy, int voice, float freq)
{
    _tWaveSynth* c = *cy;
    for (int t = 0; t < c->numTables; ++t)
    {
        tWaveOsc_setFreq(&c->oscs[t][voice], freq);
    }
}

void tWaveSynth_setAntiAliasing(tWaveSynth* const cy, float aa)
{
    _tWaveSynth* c = *cy;
    for (int t = 0; t < c->numTables; ++t)
    {
        for (int v = 0; v < c->numVoices; ++v)
        {
            tWaveOsc_setAntiAliasing(&c->oscs[t][v], aa);
        }
    }
}

void tWaveSynth_setIndex(tWaveSynth* const cy, float index)
{
    _tWaveSynth* c = *cy;
    c->index = index;
}

void tWaveSynth_setIndexGain(tWaveSynth* const cy, int i, float gain)
{
    _tWaveSynth* c = *cy;
    if (i >= c->numTables) return;
    c->g[i] = gain;
}

void tWaveSynth_setIndexPhase(tWaveSynth* const cy, int i, float phase)
{
    _tWaveSynth* c = *cy;
    if (i >= c->numTables) return;
    for (int v = 0; v < c->numVoices; ++v)
    {
        tWaveOsc_setPhaseOffset(&c->oscs[i][v], phase);
    }
}

void tWaveSynth_setSampleRate(tWaveSynth* const cy, float sr)
{
    _tWaveSynth* c = *cy;
    
    for (int i = 0; i < c->numTables; ++i)
    {
        tWaveTable_setSampleRate(&c->tables[i], sr);
        for (int v = 0; v < c->numVoices; ++v) tWaveOsc_setSampleRate(&c->oscs[i][v], sr);
    }
}

//=======================================================================================
//=======================================================================================

void tWaveTableS_init(tWaveTableS* const cy, float* table, int size, float maxFreq, LEAF* const leaf)
{
    tWaveTableS_initToPool(cy, table, size, maxFreq, &leaf->mempool);
}

void tWaveTableS_initToPool(tWaveTableS* const cy, float* table, int size, float maxFreq, tMempool* const mp)
{
    _tMempool* m = *mp;
    _tWaveTableS* c = *cy = (_tWaveTableS*) mpool_alloc(sizeof(_tWaveTableS), m);
    c->mempool = m;
    LEAF* leaf = c->mempool->leaf;
    
    c->sampleRate = leaf->sampleRate;
    
    c->maxFreq = maxFreq;
    
    // Determine base frequency
    c->baseFreq = c->sampleRate / (float) size;
    c->invBaseFreq = 1.0f / c->baseFreq;
    
    // Determine how many tables we need
    c->numTables = 2;
    float f = c->baseFreq;
    while (f < maxFreq)
    {
        c->numTables++;
        f *= 2.0f; // pass this multiplier in to set spacing of tables?
    }
    
    // Allocate memory for the tables
    c->tables = (float**) mpool_alloc(sizeof(float*) * c->numTables, c->mempool);
    c->sizes = (int*) mpool_alloc(sizeof(int) * c->numTables, c->mempool);
    c->sizes[0] = size;
    c->baseTable = (float*) mpool_alloc(sizeof(float) * c->sizes[0], c->mempool);
    c->tables[0] = c->baseTable;
    for (int t = 1; t < c->numTables; ++t)
    {
        c->sizes[t] = c->sizes[t-1] / 2;
        c->tables[t] = (float*) mpool_alloc(sizeof(float) * c->sizes[t], c->mempool);
    }
    
    // Copy table
    for (int i = 0; i < c->sizes[0]; ++i)
    {
        c->baseTable[i] = table[i];
    }
    
    // Make bandlimited copies
    // Not worth going over order 8 I think, and even 8 is only marginally better than 4.
    tButterworth_initToPool(&c->bl, 8, -1.0f, c->sampleRate * 0.25f, mp);
    tOversampler_initToPool(&c->ds, 2, 1, mp);
    for (int t = 1; t < c->numTables; ++t)
    {
        // Similar to tWaveTable, doing multiple passes here helps, but not sure what number is optimal
        for (int p = 0; p < 12; ++p)
        {
            for (int i = 0; i < c->sizes[t]; ++i)
            {
                c->dsBuffer[0] = tButterworth_tick(&c->bl, c->tables[t-1][i*2]);
                c->dsBuffer[1] = tButterworth_tick(&c->bl, c->tables[t-1][(i*2)+1]);
                c->tables[t][i] = tOversampler_downsample(&c->ds, c->dsBuffer);
            }
        }
    }
    tOversampler_free(&c->ds);
    tButterworth_free(&c->bl);
}

void    tWaveTableS_free(tWaveTableS* const cy)
{
    _tWaveTableS* c = *cy;
    
    for (int t = 0; t < c->numTables; ++t)
    {
        mpool_free((char*)c->tables[t], c->mempool);
    }
    mpool_free((char*)c->tables, c->mempool);
    mpool_free((char*)c->sizes, c->mempool);
    mpool_free((char*)c, c->mempool);
}

void    tWaveTableS_setSampleRate(tWaveTableS* const cy, float sr)
{
    _tWaveTableS* c = *cy;
    
    int size = c->sizes[0];
    
    for (int t = 1; t < c->numTables; ++t)
    {
        mpool_free((char*)c->tables[t], c->mempool);
    }
    mpool_free((char*)c->tables, c->mempool);
    mpool_free((char*)c->sizes, c->mempool);
    
    c->sampleRate = sr;
    
    // Determine base frequency
    c->baseFreq = c->sampleRate / (float) size;
    c->invBaseFreq = 1.0f / c->baseFreq;
    
    // Determine how many tables we need
    c->numTables = 2;
    float f = c->baseFreq;
    while (f < c->maxFreq)
    {
        c->numTables++;
        f *= 2.0f; // pass this multiplier in to set spacing of tables?
    }
    
    // Allocate memory for the tables
    c->tables = (float**) mpool_alloc(sizeof(float*) * c->numTables, c->mempool);
    c->sizes = (int*) mpool_alloc(sizeof(int) * c->numTables, c->mempool);
    c->tables[0] = c->baseTable;
    c->sizes[0] = size;
    for (int t = 1; t < c->numTables; ++t)
    {
        c->sizes[t] = c->sizes[t-1] / 2;
        c->tables[t] = (float*) mpool_alloc(sizeof(float) * c->sizes[t], c->mempool);
    }
    
    // Make bandlimited copies
    // Not worth going over order 8 I think, and even 8 is only marginally better than 4.
    tButterworth_initToPool(&c->bl, 8, -1.0f, c->sampleRate * 0.25f, &c->mempool);
    tButterworth_setSampleRate(&c->bl, c->sampleRate);
    tOversampler_initToPool(&c->ds, 2, 1, &c->mempool);
    for (int t = 1; t < c->numTables; ++t)
    {
        // Similar to tWaveTable, doing multiple passes here helps, but not sure what number is optimal
        for (int p = 0; p < 12; ++p)
        {
            for (int i = 0; i < c->sizes[t]; ++i)
            {
                c->dsBuffer[0] = tButterworth_tick(&c->bl, c->tables[t-1][i*2]);
                c->dsBuffer[1] = tButterworth_tick(&c->bl, c->tables[t-1][(i*2)+1]);
                c->tables[t][i] = tOversampler_downsample(&c->ds, c->dsBuffer);
            }
        }
    }
    tOversampler_free(&c->ds);
    tButterworth_free(&c->bl);
}

//================================================================================================
//================================================================================================

void tWaveOscS_init(tWaveOscS* const cy, tWaveTableS* const table, LEAF* const leaf)
{
    tWaveOscS_initToPool(cy, table, &leaf->mempool);
}

void tWaveOscS_initToPool(tWaveOscS* const cy, tWaveTableS* const table, tMempool* const mp)
{
    _tMempool* m = *mp;
    _tWaveOscS* c = *cy = (_tWaveOscS*) mpool_alloc(sizeof(_tWaveOscS), m);
    c->mempool = m;
    LEAF* leaf = c->mempool->leaf;
    
    c->invSampleRate = leaf->invSampleRate;
    c->inc = 0.0f;
    c->phase = 0.0f;
    c->phaseOffset = 0.0f;
    c->aa = 0.5f;
    
    tWaveOscS_setFreq(cy, 220);
}

void tWaveOscS_free(tWaveOscS* const cy)
{
    _tWaveOscS* c = *cy;
    mpool_free((char*)c, c->mempool);
}

float tWaveOscS_tick(tWaveOscS* const cy)
{
    _tWaveOscS* c = *cy;
    
    float temp;
    int idx;
    float frac;
    float samp0;
    float samp1;
    
    // Phasor increment
    c->phase += c->inc;
    if (c->phase >= 1.0f) c->phase -= 1.0f;
    if (c->phase < 0.0f) c->phase += 1.0f;
    
    int* sizes = c->table->sizes;
    float** tables = c->table->tables;
    
    // Wavetable synthesis
    temp = sizes[c->oct] * (c->phase + c->phaseOffset);
    idx = (int)temp;
    frac = temp - (float)idx;
    samp0 = tables[c->oct][idx];
    if (++idx >= sizes[c->oct]) idx = 0;
    samp1 = tables[c->oct][idx];
    
    float oct0 = (samp0 + (samp1 - samp0) * frac);
    
    temp = sizes[c->oct+1] * c->phase;
    idx = (int)temp;
    frac = temp - (float)idx;
    samp0 = tables[c->oct+1][idx];
    if (++idx >= sizes[c->oct+1]) idx = 0;
    samp1 = tables[c->oct+1][idx];
    
    float oct1 = (samp0 + (samp1 - samp0) * frac);
    
    return oct0 + (oct1 - oct0) * c->w;
}

void tWaveOscS_setFreq(tWaveOscS* const cy, float freq)
{
    _tWaveOscS* c = *cy;

    c->freq  = freq;
    
    c->inc = c->freq * c->invSampleRate;
    c->inc -= (int)c->inc;
    
    // abs for negative frequencies
    c->w = fabsf(c->freq * c->table->invBaseFreq);
    
    // Probably ok to use a log2 approx here; won't effect tuning at all, just crossfading between octave tables
    c->w = log2f_approx(c->w) + c->aa;//+ LEAF_SQRT2 - 1.0f; adding an offset here will shift our table selection upward, reducing aliasing but lower high freq fidelity. +1.0f should remove all aliasing
    if (c->w < 0.0f) c->w = 0.0f; // If c->w is < 0.0f, then freq is less than our base freq
    c->oct = (int)c->w;
    c->w -= c->oct;
    if (c->oct >= c->table->numTables - 1) c->oct = c->table->numTables - 2;
}

void tWaveOscS_setAntiAliasing(tWaveOscS* const cy, float aa)
{
    _tWaveOscS* c = *cy;
    c->aa = aa;
    tWaveOscS_setFreq(cy, c->freq);
}

void tWaveOscS_setPhaseOffset(tWaveOscS* const cy, float phase)
{
    _tWaveOscS* c = *cy;
    c->phaseOffset = phase - (int)phase;
}

void tWaveOscS_setSampleRate(tWaveOscS* const cy, float sr)
{
    _tWaveOscS* c = *cy;
    c->invSampleRate = 1.0f/sr;
    tWaveOscS_setFreq(cy, c->freq);
}

//================================================================================================
//================================================================================================

void tWaveSynthS_init(tWaveSynthS* const cy, int numVoices, float** tables, int* sizes,
                            int numTables, float maxFreq, LEAF* const leaf)
{
    tWaveSynthS_initToPool(cy, numVoices, tables, sizes, numTables, maxFreq, &leaf->mempool);
}

void tWaveSynthS_initToPool(tWaveSynthS* const cy, int numVoices, float** tables, int* sizes,
                                  int numTables, float maxFreq, tMempool* const mp)
{
    _tMempool* m = *mp;
    _tWaveSynthS* c = *cy = (_tWaveSynthS*) mpool_alloc(sizeof(_tWaveSynthS), m);
    c->mempool = m;
    
    c->numTables = 0;
    for (int t = 0; t < numTables; ++t)
    {
        if (sizes[t] > 0) c->numTables++;
    }
    
    c->tables = (tWaveTableS*) mpool_alloc(sizeof(tWaveTableS) * c->numTables, m);
    c->oscs = (tWaveOscS**) mpool_alloc(sizeof(tWaveOscS*) * c->numTables, m);
    
    c->numVoices = numVoices;
    
    int i = 0;
    for (int t = 0; t < numTables; ++t)
    {
        if (sizes[t] > 0)
        {
            tWaveTableS_initToPool(&c->tables[i], tables[t], sizes[t], maxFreq, mp);
            c->oscs[i] = (tWaveOscS*) mpool_alloc(sizeof(tWaveOscS) * c->numVoices, m);
            for (int v = 0; v < c->numVoices; ++v) tWaveOscS_initToPool(&c->oscs[i][v], &c->tables[i], mp);
            i++;
        }
    }
    
    c->g = (float*) mpool_alloc(sizeof(float) * c->numTables, m);
    for (int i = 0; i < c->numTables; ++i)  c->g[i] = 1.0f;
    
    c->index = 0.0f;
    c->maxFreq = maxFreq;
}

void tWaveSynthS_free(tWaveSynthS* const cy)
{
    _tWaveSynthS* c = *cy;
    
    for (int i = 0; i < c->numTables; ++i)
    {
        tWaveTableS_free(&c->tables[i]);
        for (int v = 0; v < c->numVoices; ++v) tWaveOscS_free(&c->oscs[i][v]);
        mpool_free((char*)c->oscs[i], c->mempool);
    }
    mpool_free((char*)c->g, c->mempool);
    mpool_free((char*)c->oscs, c->mempool);
    mpool_free((char*)c->tables, c->mempool);
    mpool_free((char*)c, c->mempool);
}

float tWaveSynthS_tick(tWaveSynthS* const cy)
{
    _tWaveSynthS* c = *cy;
    
    float f = c->index * (c->numTables - 1);
    
    int o1 = (int)f;
    int o2 = o1 + 1;
    if (c->index >= 1.0f) o2 = o1;
    float mix = f - o1;
    
    float s1 = 0.f, s2 = 0.f;
    for (int t = 0; t < c->numTables; ++t)
    {
        float s0 = 0.f;
        for (int v = 0; v < c->numVoices; ++v) s0 += tWaveOscS_tick(&c->oscs[t][v]);
        if (t == o1) s1 = s0 * c->g[t];
        if (t == o2) s2 = s0 * c->g[t];
    }
    
    // Ideally should determine correlation to get a good equal power fade between tables
    return s1 + (s2 - s1) * mix;
}

float tWaveSynthS_tickVoice(tWaveSynthS* const cy, int voice)
{
    _tWaveSynthS* c = *cy;
    
    float f = c->index * (c->numTables - 1);
    
    int o1 = (int)f;
    int o2 = o1 + 1;
    if (c->index >= 1.0f) o2 = o1;
    float mix = f - o1;
    
    float s1 = 0.f, s2 = 0.f;
    for (int t = 0; t < c->numTables; ++t)
    {
        // Should we tick every voice anyway to preserve relative phases?
        float s0 = tWaveOscS_tick(&c->oscs[t][voice]);
        if (t == o1) s1 = s0 * c->g[t];
        if (t == o2) s2 = s0 * c->g[t];
    }
    
    // Ideally should determine correlation to get a good equal power fade between tables
    return s1 + (s2 - s1) * mix;
}

void tWaveSynthS_setFreq(tWaveSynthS* const cy, int voice, float freq)
{
    _tWaveSynthS* c = *cy;
    for (int t = 0; t < c->numTables; ++t)
    {
        tWaveOscS_setFreq(&c->oscs[t][voice], freq);
    }
}

void tWaveSynthS_setAntiAliasing(tWaveSynthS* const cy, float aa)
{
    _tWaveSynthS* c = *cy;
    for (int t = 0; t < c->numTables; ++t)
    {
        for (int v = 0; v < c->numVoices; ++v)
        {
            tWaveOscS_setAntiAliasing(&c->oscs[t][v], aa);
        }
    }
}

void tWaveSynthS_setIndex(tWaveSynthS* const cy, float index)
{
    _tWaveSynthS* c = *cy;
    c->index = index;
}

void tWaveSynthS_setIndexGain(tWaveSynthS* const cy, int i, float gain)
{
    _tWaveSynthS* c = *cy;
    if (i >= c->numTables) return;
    c->g[i] = gain;
}

void tWaveSynthS_setIndexPhase(tWaveSynthS* const cy, int i, float phase)
{
    _tWaveSynthS* c = *cy;
    if (i >= c->numTables) return;
    for (int v = 0; v < c->numVoices; ++v)
    {
        tWaveOscS_setPhaseOffset(&c->oscs[i][v], phase);
    }
}

void tWaveSynthS_setSampleRate(tWaveSynthS* const cy, float sr)
{
    _tWaveSynthS* c = *cy;
    for (int i = 0; i < c->numTables; ++i)
    {
        tWaveTableS_setSampleRate(&c->tables[i], sr);
        for (int v = 0; v < c->numVoices; ++v) tWaveOscS_setSampleRate(&c->oscs[i][v], sr);
    }
}
//
//void tWaveSynthS_setIndexTable(tWaveSynthS* const cy, int i, float* table, int size)
//{
//    _tWaveSynthS* c = *cy;
//    if (i >= c->numTables) return;
//    tWaveTableS_free(&c->tables[i]);
//    tWaveTableS_initToPool(&c->tables[i], table, size, c->maxFreq, &c->mempool);
//    for (int v = 0; v < c->numVoices; ++v)
//    {
//        tWaveOscS_setFreq(&c->oscs[i][v], c->oscs[i][v]->freq);
//    }
//}
