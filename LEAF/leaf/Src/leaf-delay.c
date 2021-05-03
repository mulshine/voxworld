/*==============================================================================

    leaf-delay.c
    Created: 20 Jan 2017 12:01:24pm
    Author:  Michael R Mulshine

==============================================================================*/

#if _WIN32 || _WIN64

#include "..\Inc\leaf-delay.h"
#include "..\leaf.h"

#else

#include "../Inc/leaf-delay.h"
#include "../leaf.h"

#endif

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ Delay ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ //
void    tDelay_init (tDelay* const dl, uint32_t delay, uint32_t maxDelay, LEAF* const leaf)
{
    tDelay_initToPool(dl, delay, maxDelay, &leaf->mempool);
}

void    tDelay_initToPool   (tDelay* const dl, uint32_t delay, uint32_t maxDelay, tMempool* const mp)
{
    _tMempool* m = *mp;
    _tDelay* d = *dl = (_tDelay*) mpool_alloc(sizeof(_tDelay), m);
    d->mempool = m;

    d->maxDelay = maxDelay;

    d->delay = delay;

    d->buff = (float*) mpool_alloc(sizeof(float) * maxDelay, m);
    
    d->inPoint = 0;
    d->outPoint = 0;
    
    d->lastIn = 0.0f;
    d->lastOut = 0.0f;
    
    d->gain = 1.0f;
    
    tDelay_setDelay(dl, d->delay);
}

void tDelay_free (tDelay* const dl)
{
    _tDelay* d = *dl;
    
    mpool_free((char*)d->buff, d->mempool);
    mpool_free((char*)d, d->mempool);
}

void    tDelay_clear(tDelay* const dl)
{
    _tDelay* d = *dl;
    for (unsigned i = 0; i < d->maxDelay; i++)
    {
        d->buff[i] = 0;
    }
}

float   tDelay_tick (tDelay* const dl, float input)
{
    _tDelay* d = *dl;

    // Input
    d->lastIn = input;
    d->buff[d->inPoint] = input * d->gain;
    if (++(d->inPoint) == d->maxDelay)     d->inPoint = 0;

    // Output
    d->lastOut = d->buff[d->outPoint];
    if (++(d->outPoint) == d->maxDelay)    d->outPoint = 0;

    return d->lastOut;
}

void     tDelay_setDelay (tDelay* const dl, uint32_t delay)
{
    _tDelay* d = *dl;

    d->delay = LEAF_clip(0.0f, delay,  d->maxDelay);

    // read chases write
    if ( d->inPoint >= delay )  d->outPoint = d->inPoint - d->delay;
    else                        d->outPoint = d->maxDelay + d->inPoint - d->delay;
}

float tDelay_tapOut (tDelay* const dl, uint32_t tapDelay)
{
    _tDelay* d = *dl;

    int32_t tap = d->inPoint - tapDelay - 1;

    // Check for wraparound.
    while ( tap < 0 )   tap += d->maxDelay;

    return d->buff[tap];

}

void tDelay_tapIn (tDelay* const dl, float value, uint32_t tapDelay)
{
    _tDelay* d = *dl;

    int32_t tap = d->inPoint - tapDelay - 1;
    
    // Check for wraparound.
    while ( tap < 0 )   tap += d->maxDelay;
    
    d->buff[tap] = value;
}

float tDelay_addTo (tDelay* const dl, float value, uint32_t tapDelay)
{
    _tDelay* d = *dl;
    
    int32_t tap = d->inPoint - tapDelay - 1;
    
    // Check for wraparound.
    while ( tap < 0 )   tap += d->maxDelay;
    
    return (d->buff[tap] += value);
}

uint32_t   tDelay_getDelay (tDelay* const dl)
{
    _tDelay* d = *dl;
    return d->delay;
}

float   tDelay_getLastOut (tDelay* const dl)
{
    _tDelay* d = *dl;
    return d->lastOut;
}

float   tDelay_getLastIn (tDelay* const dl)
{
    _tDelay* d = *dl;
    return d->lastIn;
}

void tDelay_setGain (tDelay* const dl, float gain)
{
    _tDelay* d = *dl;
    if (gain < 0.0f)    d->gain = 0.0f;
    else                d->gain = gain;
}

float tDelay_getGain (tDelay* const dl)
{
    _tDelay* d = *dl;
    return d->gain;
}

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ LinearDelay ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ //
void   tLinearDelay_init (tLinearDelay* const dl, float delay, uint32_t maxDelay, LEAF* const leaf)
{
    tLinearDelay_initToPool(dl, delay, maxDelay, &leaf->mempool);
}

void tLinearDelay_initToPool  (tLinearDelay* const dl, float delay, uint32_t maxDelay, tMempool* const mp)
{
    _tMempool* m = *mp;
    _tLinearDelay* d = *dl = (_tLinearDelay*) mpool_alloc(sizeof(_tLinearDelay), m);
    d->mempool = m;

    d->maxDelay = maxDelay;

    if (delay > maxDelay)   d->delay = maxDelay;
    else if (delay < 0.0f)  d->delay = 0.0f;
    else                    d->delay = delay;

    d->buff = (float*) mpool_alloc(sizeof(float) * maxDelay, m);

    d->gain = 1.0f;

    d->lastIn = 0.0f;
    d->lastOut = 0.0f;
    
    d->inPoint = 0;
    d->outPoint = 0;
    
    tLinearDelay_setDelay(dl, d->delay);
}

void tLinearDelay_free (tLinearDelay* const dl)
{
    _tLinearDelay* d = *dl;
    
    mpool_free((char*)d->buff, d->mempool);
    mpool_free((char*)d, d->mempool);
}

void    tLinearDelay_clear(tLinearDelay* const dl)
{
    _tLinearDelay* d = *dl;
    for (unsigned i = 0; i < d->maxDelay; i++)
    {
        d->buff[i] = 0;
    }
}

float   tLinearDelay_tick (tLinearDelay* const dl, float input)
{
    _tLinearDelay* d = *dl;

    d->buff[d->inPoint] = input * d->gain;

    // Increment input pointer modulo length.
    if (++(d->inPoint) == d->maxDelay )    d->inPoint = 0;

    uint32_t idx = (uint32_t) d->outPoint;
    // First 1/2 of interpolation
    d->lastOut = d->buff[idx] * d->omAlpha;
        // Second 1/2 of interpolation
    if ((idx + 1) < d->maxDelay)
        d->lastOut += d->buff[idx+1] * d->alpha;
    else
        d->lastOut += d->buff[0] * d->alpha;

    // Increment output pointer modulo length
    if ( (++d->outPoint) >= d->maxDelay )   d->outPoint = 0;

    return d->lastOut;
}

void   tLinearDelay_tickIn (tLinearDelay* const dl, float input)
{
    _tLinearDelay* d = *dl;

    d->buff[d->inPoint] = input * d->gain;

    // Increment input pointer modulo length.
    if (++(d->inPoint) == d->maxDelay )    d->inPoint = 0;
}

float   tLinearDelay_tickOut (tLinearDelay* const dl)
{
    _tLinearDelay* d = *dl;

    uint32_t idx = (uint32_t) d->outPoint;
    // First 1/2 of interpolation
    d->lastOut = d->buff[idx] * d->omAlpha;
        // Second 1/2 of interpolation
    if ((idx + 1) < d->maxDelay)
        d->lastOut += d->buff[idx+1] * d->alpha;
    else
        d->lastOut += d->buff[0] * d->alpha;

    // Increment output pointer modulo length
    if ( (++d->outPoint) >= d->maxDelay )   d->outPoint = 0;

    return d->lastOut;
}

void     tLinearDelay_setDelay (tLinearDelay* const dl, float delay)
{
    _tLinearDelay* d = *dl;

    d->delay = LEAF_clip(0.0f, delay,  d->maxDelay);

    float outPointer = d->inPoint - d->delay;

    while ( outPointer < 0 )
        outPointer += d->maxDelay; // modulo maximum length

    d->outPoint = (uint32_t) outPointer;   // integer part

    d->alpha = outPointer - d->outPoint; // fractional part
    d->omAlpha = 1.0f - d->alpha;

    if ( d->outPoint == d->maxDelay ) d->outPoint = 0;
}

float tLinearDelay_tapOut (tLinearDelay* const dl, uint32_t tapDelay)
{
    _tLinearDelay* d = *dl;

    int32_t tap = d->inPoint - tapDelay - 1;
    // Check for wraparound.
    while ( tap < 0 )   tap += d->maxDelay;

    return d->buff[tap];
}

void tLinearDelay_tapIn (tLinearDelay* const dl, float value, uint32_t tapDelay)
{
    _tLinearDelay* d = *dl;

    int32_t tap = d->inPoint - tapDelay - 1;

    // Check for wraparound.
    while ( tap < 0 )   tap += d->maxDelay;

    d->buff[tap] = value;
}

float tLinearDelay_addTo (tLinearDelay* const dl, float value, uint32_t tapDelay)
{
    _tLinearDelay* d = *dl;

    int32_t tap = d->inPoint - tapDelay - 1;

    // Check for wraparound.
    while ( tap < 0 )   tap += d->maxDelay;
    
    return (d->buff[tap] += value);
}

float   tLinearDelay_getDelay (tLinearDelay* const dl)
{
    _tLinearDelay* d = *dl;
    return d->delay;
}

float   tLinearDelay_getLastOut (tLinearDelay* const dl)
{
    _tLinearDelay* d = *dl;
    return d->lastOut;
}

float   tLinearDelay_getLastIn (tLinearDelay* const dl)
{
    _tLinearDelay* d = *dl;
    return d->lastIn;
}

void tLinearDelay_setGain (tLinearDelay* const dl, float gain)
{
    _tLinearDelay* d = *dl;
    if (gain < 0.0f)    d->gain = 0.0f;
    else                d->gain = gain;
}

float tLinearDelay_getGain (tLinearDelay* const dl)
{
    _tLinearDelay* d = *dl;
    return d->gain;
}


/// Hermite Interpolated Delay
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ LinearDelay ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ //
void tHermiteDelay_init (tHermiteDelay* const dl, float delay, uint32_t maxDelay, LEAF* const leaf)
{
    tHermiteDelay_initToPool(dl, delay, maxDelay, &leaf->mempool);
}

void tHermiteDelay_initToPool  (tHermiteDelay* const dl, float delay, uint32_t maxDelay, tMempool* const mp)
{
    _tMempool* m = *mp;
    _tHermiteDelay* d = *dl = (_tHermiteDelay*) mpool_alloc(sizeof(_tHermiteDelay), m);
    d->mempool = m;

    d->maxDelay = maxDelay;

    if (delay > maxDelay)   d->delay = maxDelay;
    else if (delay < 0.0f)  d->delay = 0.0f;
    else                    d->delay = delay;

    
    if ((maxDelay != 0) && ((maxDelay & (maxDelay - 1)) == 0))
    {
        d->maxDelay = maxDelay;
        d->bufferMask = maxDelay - 1;
    }
    else
    {
        //make the delay size into a power of 2
        maxDelay--;
        maxDelay |= maxDelay >> 1;
        maxDelay |= maxDelay >> 2;
        maxDelay |= maxDelay >> 4;
        maxDelay |= maxDelay >> 8;
        maxDelay |= maxDelay >> 16;
        maxDelay++;
        d->maxDelay = maxDelay;
        d->bufferMask = maxDelay - 1;
    }
    d->buff = (float*) mpool_alloc(sizeof(float) * maxDelay, m);

    d->gain = 1.0f;

    d->lastIn = 0.0f;
    d->lastOut = 0.0f;

    d->inPoint = 0;
    d->outPoint = 0;

    tHermiteDelay_setDelay(dl, d->delay);
}

void tHermiteDelay_free (tHermiteDelay* const dl)
{
    _tHermiteDelay* d = *dl;

    mpool_free((char*)d->buff, d->mempool);
    mpool_free((char*)d, d->mempool);
}


void    tHermiteDelay_clear(tHermiteDelay* const dl)
{
    _tHermiteDelay* d = *dl;
    for (unsigned i = 0; i < d->maxDelay; i++)
    {
        d->buff[i] = 0;
    }
}

float   tHermiteDelay_tick (tHermiteDelay* const dl, float input)
{
    _tHermiteDelay* d = *dl;

    d->buff[d->inPoint] = input * d->gain;

    
    // Increment input pointer modulo length.
    d->inPoint = (d->inPoint + 1) & d->bufferMask;
    
    
    uint32_t idx = (uint32_t) d->outPoint;
    d->lastOut =    LEAF_interpolate_hermite_x (d->buff[((idx - 1) + d->maxDelay) & d->bufferMask],
                                                d->buff[idx],
                                                d->buff[(idx + 1) & d->bufferMask],
                                                d->buff[(idx + 2) & d->bufferMask],
                                                d->alpha);
    
    // Increment output pointer modulo length
    d->outPoint = (d->outPoint + 1) & d->bufferMask;

    return d->lastOut;
}

void   tHermiteDelay_tickIn (tHermiteDelay* const dl, float input)
{
    _tHermiteDelay* d = *dl;
    
    d->buff[d->inPoint] = input;
    
    // Increment input pointer modulo length.
    d->inPoint = (d->inPoint + 1) & d->bufferMask;
}

float   tHermiteDelay_tickOut (tHermiteDelay* const dl)
{
    _tHermiteDelay* d = *dl;
    
    uint32_t idx = (uint32_t) d->outPoint;
    
    
    
    d->lastOut =    LEAF_interpolate_hermite_x (d->buff[((idx - 1) + d->maxDelay) & d->bufferMask],
                                                d->buff[idx],
                                                d->buff[(idx + 1) & d->bufferMask],
                                                d->buff[(idx + 2) & d->bufferMask],
                                                d->alpha);
    
    // Increment output pointer modulo length
    d->outPoint = (d->outPoint + 1) & d->bufferMask;
    
    return d->lastOut;
}

void tHermiteDelay_setDelay (tHermiteDelay* const dl, float delay)
{
    _tHermiteDelay* d = *dl;
    //d->delay = LEAF_clip(0.0f, delay,  d->maxDelay);
    d->delay = delay; // not safe but faster
    float outPointer = d->inPoint - d->delay;
    while ( outPointer < 0 )
        outPointer += d->maxDelay; // modulo maximum length
    
    d->outPoint = (uint32_t) outPointer;   // integer part
    
    d->alpha = outPointer - d->outPoint; // fractional part
    d->omAlpha = 1.0f - d->alpha;
    
    d->outPoint &= d->bufferMask;
}

float tHermiteDelay_tapOut (tHermiteDelay* const dl, uint32_t tapDelay)
{
    _tHermiteDelay* d = *dl;
    
    int32_t tap = (d->inPoint - tapDelay - 1) & d->bufferMask;
    
    return d->buff[tap];

}

float   tHermiteDelay_tapOutInterpolated (tHermiteDelay* const dl, uint32_t tapDelay, float alpha)
{
    _tHermiteDelay* d = *dl;
    
    int32_t idx = (d->inPoint - tapDelay - 1) & d->bufferMask;
    
    return    LEAF_interpolate_hermite_x (d->buff[((idx - 1) + d->maxDelay) & d->bufferMask],
                                          d->buff[idx],
                                          d->buff[(idx + 1) & d->bufferMask],
                                          d->buff[(idx + 2) & d->bufferMask],
                                          alpha);
}

void tHermiteDelay_tapIn (tHermiteDelay* const dl, float value, uint32_t tapDelay)
{
    _tHermiteDelay* d = *dl;
    
    int32_t tap = (d->inPoint - tapDelay - 1)  & d->bufferMask;
    
    d->buff[tap] = value;
}

float tHermiteDelay_addTo (tHermiteDelay* const dl, float value, uint32_t tapDelay)
{
    _tHermiteDelay* d = *dl;
    
    int32_t tap = (d->inPoint - tapDelay - 1)  & d->bufferMask;
    
    return (d->buff[tap] += value);
}

float   tHermiteDelay_getDelay (tHermiteDelay* const dl)
{
    _tHermiteDelay* d = *dl;
    return d->delay;
}

float   tHermiteDelay_getLastOut (tHermiteDelay* const dl)
{
    _tHermiteDelay* d = *dl;
    return d->lastOut;
}

float   tHermiteDelay_getLastIn (tHermiteDelay* const dl)
{
    _tHermiteDelay* d = *dl;
    return d->lastIn;
}

void tHermiteDelay_setGain (tHermiteDelay* const dl, float gain)
{
    _tHermiteDelay* d = *dl;
    if (gain < 0.0f)    d->gain = 0.0f;
    else                d->gain = gain;
}

float tHermiteDelay_getGain (tHermiteDelay* const dl)
{
    _tHermiteDelay* d = *dl;
    return d->gain;
}





// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ AllpassDelay ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ //
void tAllpassDelay_init (tAllpassDelay* const dl, float delay, uint32_t maxDelay, LEAF* const leaf)
{
    tAllpassDelay_initToPool(dl, delay, maxDelay, &leaf->mempool);
}

void tAllpassDelay_initToPool  (tAllpassDelay* const dl, float delay, uint32_t maxDelay, tMempool* const mp)
{
    _tMempool* m = *mp;
    _tAllpassDelay* d = *dl = (_tAllpassDelay*) mpool_alloc(sizeof(_tAllpassDelay), m);
    d->mempool = m;

    d->maxDelay = maxDelay;

    if (delay > maxDelay)   d->delay = maxDelay;
    else if (delay < 0.0f)  d->delay = 0.0f;
    else                    d->delay = delay;

    d->buff = (float*) mpool_alloc(sizeof(float) * maxDelay, m);

    d->gain = 1.0f;
    
    d->lastIn = 0.0f;
    d->lastOut = 0.0f;
    
    d->inPoint = 0;
    d->outPoint = 0;
    
    tAllpassDelay_setDelay(dl, d->delay);
    
    d->apInput = 0.0f;
}

void tAllpassDelay_free (tAllpassDelay* const dl)
{
    _tAllpassDelay* d = *dl;
    
    mpool_free((char*)d->buff, d->mempool);
    mpool_free((char*)d, d->mempool);
}

void tAllpassDelay_clear(tAllpassDelay* const dl)
{
    _tAllpassDelay* d = *dl;
    for (unsigned i = 0; i < d->maxDelay; i++)
    {
        d->buff[i] = 0;
    }
}

float tAllpassDelay_tick (tAllpassDelay* const dl, float input)
{
    _tAllpassDelay* d = *dl;

    d->buff[d->inPoint] = input * d->gain;

    // Increment input pointer modulo length.
    if ( ++(d->inPoint) >= d->maxDelay )    d->inPoint = 0;

    // Do allpass interpolation delay.
    float out = d->lastOut * -d->coeff;
    out += d->apInput + ( d->coeff * d->buff[d->outPoint] );
    d->lastOut = out;

    // Save allpass input
    d->apInput = d->buff[d->outPoint];

    // Increment output pointer modulo length.
    if (++(d->outPoint) >= d->maxDelay )   d->outPoint = 0;

    return d->lastOut;
}

void     tAllpassDelay_setDelay (tAllpassDelay* const dl, float delay)
{
    _tAllpassDelay* d = *dl;

    d->delay = LEAF_clip(0.5f, delay,  d->maxDelay);

    // outPoint chases inPoint
    float outPointer = (float)d->inPoint - d->delay + 1.0f;

    while ( outPointer < 0 )    outPointer += d->maxDelay;  // mod max length

    d->outPoint = (uint32_t) outPointer;         // integer part

    if ( d->outPoint >= d->maxDelay )   d->outPoint = 0;

    d->alpha = 1.0f + (float)d->outPoint - outPointer; // fractional part

    if ( d->alpha < 0.5f )
    {
        // The optimal range for alpha is about 0.5 - 1.5 in order to
        // achieve the flattest phase delay response.
        
        d->outPoint += 1;
        
        if ( d->outPoint >= d->maxDelay ) d->outPoint -= d->maxDelay;
        
        d->alpha += 1.0f;
    }

    d->coeff = (1.0f - d->alpha) / (1.0f + d->alpha);  // coefficient for allpass
}

float tAllpassDelay_tapOut (tAllpassDelay* const dl, uint32_t tapDelay)
{
    _tAllpassDelay* d = *dl;

    int32_t tap = d->inPoint - tapDelay - 1;

    // Check for wraparound.
    while ( tap < 0 )   tap += d->maxDelay;

    return d->buff[tap];

}

void tAllpassDelay_tapIn (tAllpassDelay* const dl, float value, uint32_t tapDelay)
{
    _tAllpassDelay* d = *dl;

    int32_t tap = d->inPoint - tapDelay - 1;

    // Check for wraparound.
    while ( tap < 0 )   tap += d->maxDelay;

    d->buff[tap] = value;
}

float tAllpassDelay_addTo (tAllpassDelay* const dl, float value, uint32_t tapDelay)
{
    _tAllpassDelay* d = *dl;

    int32_t tap = d->inPoint - tapDelay - 1;

    // Check for wraparound.
    while ( tap < 0 )   tap += d->maxDelay;

    return (d->buff[tap] += value);
}

float   tAllpassDelay_getDelay (tAllpassDelay* const dl)
{
    _tAllpassDelay* d = *dl;
    return d->delay;
}

float   tAllpassDelay_getLastOut (tAllpassDelay* const dl)
{
    _tAllpassDelay* d = *dl;
    return d->lastOut;
}

float   tAllpassDelay_getLastIn (tAllpassDelay* const dl)
{
    _tAllpassDelay* d = *dl;
    return d->lastIn;
}

void tAllpassDelay_setGain (tAllpassDelay* const dl, float gain)
{
    _tAllpassDelay* d = *dl;
    if (gain < 0.0f)    d->gain = 0.0f;
    else                d->gain = gain;
}

float tAllpassDelay_getGain (tAllpassDelay* const dl)
{
    _tAllpassDelay* d = *dl;
    return d->gain;
}

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ TapeDelay ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ //
void tTapeDelay_init (tTapeDelay* const dl, float delay, uint32_t maxDelay, LEAF* const leaf)
{
    tTapeDelay_initToPool(dl, delay, maxDelay, &leaf->mempool);
}

void tTapeDelay_initToPool (tTapeDelay* const dl, float delay, uint32_t maxDelay, tMempool* const mp)
{
    _tMempool* m = *mp;
    _tTapeDelay* d = *dl = (_tTapeDelay*) mpool_alloc(sizeof(_tTapeDelay), m);
    d->mempool = m;

    d->maxDelay = maxDelay;

    d->buff = (float*) mpool_alloc(sizeof(float) * maxDelay, m);

    d->gain = 1.0f;

    d->lastIn = 0.0f;
    d->lastOut = 0.0f;

    d->idx = 0.0f;
    d->inc = 1.0f;
    d->inPoint = 0;

    tTapeDelay_setDelay(dl, delay);
}

void tTapeDelay_free (tTapeDelay* const dl)
{
    _tTapeDelay* d = *dl;

    mpool_free((char*)d->buff, d->mempool);
    mpool_free((char*)d, d->mempool);
}

void tTapeDelay_clear(tTapeDelay* const dl)
{
    _tTapeDelay* d = *dl;
    for (unsigned i = 0; i < d->maxDelay; i++)
    {
        d->buff[i] = 0;
    }
}

//#define SMOOTH_FACTOR 10.f

float   tTapeDelay_tick (tTapeDelay* const dl, float input)
{
    _tTapeDelay* d = *dl;

    d->buff[d->inPoint] = input * d->gain;

    // Increment input pointer modulo length.
    if (++(d->inPoint) == d->maxDelay )    d->inPoint = 0;

    int idx =  (int) d->idx;
    float alpha = d->idx - idx;

    d->lastOut =    LEAF_interpolate_hermite_x (d->buff[((idx - 1) + d->maxDelay) % d->maxDelay],
                                              d->buff[idx],
                                              d->buff[(idx + 1) % d->maxDelay],
                                              d->buff[(idx + 2) % d->maxDelay],
                                              alpha);

    float diff = (d->inPoint - d->idx);
    while (diff < 0.f) diff += d->maxDelay;

    d->inc = 1.0f + (diff - d->delay) / d->delay; //* SMOOTH_FACTOR;

    d->idx += d->inc;

    while (d->idx >= d->maxDelay) d->idx -= d->maxDelay;

    if (d->lastOut)
        return d->lastOut;
    return 0.0f;
}

void  tTapeDelay_incrementInPoint(tTapeDelay* const dl)
{
    _tTapeDelay* d = *dl;
    // Increment input pointer modulo length.
    if (++(d->inPoint) == d->maxDelay )    d->inPoint = 0;
}


void tTapeDelay_setRate(tTapeDelay* const dl, float rate)
{
    _tTapeDelay* d = *dl;
    d->inc = rate;
}

void     tTapeDelay_setDelay (tTapeDelay* const dl, float delay)
{
    _tTapeDelay* d = *dl;
    d->delay = LEAF_clip(1.f, delay,  d->maxDelay);
}

float tTapeDelay_tapOut (tTapeDelay* const dl, float tapDelay)
{
    _tTapeDelay* d = *dl;

    float tap = (float) d->inPoint - tapDelay - 1.f;

    // Check for wraparound.
    while ( tap < 0.f )   tap += (float)d->maxDelay;

    int idx =  (int) tap;

    float alpha = tap - idx;

    float samp =    LEAF_interpolate_hermite_x (d->buff[((idx - 1) + d->maxDelay) % d->maxDelay],
                                              d->buff[idx],
                                              d->buff[(idx + 1) % d->maxDelay],
                                              d->buff[(idx + 2) % d->maxDelay],
                                              alpha);

    return samp;

}

void tTapeDelay_tapIn (tTapeDelay* const dl, float value, uint32_t tapDelay)
{
    _tTapeDelay* d = *dl;

    int32_t tap = d->inPoint - tapDelay - 1;
    
    // Check for wraparound.
    while ( tap < 0 )   tap += d->maxDelay;
    
    d->buff[tap] = value;
}

float tTapeDelay_addTo (tTapeDelay* const dl, float value, uint32_t tapDelay)
{
    _tTapeDelay* d = *dl;
    
    int32_t tap = d->inPoint - tapDelay - 1;
    
    // Check for wraparound.
    while ( tap < 0 )   tap += d->maxDelay;
    
    return (d->buff[tap] += value);
}

float   tTapeDelay_getDelay (tTapeDelay *dl)
{
    _tTapeDelay* d = *dl;
    return d->delay;
}

float   tTapeDelay_getLastOut (tTapeDelay* const dl)
{
    _tTapeDelay* d = *dl;
    return d->lastOut;
}

float   tTapeDelay_getLastIn (tTapeDelay* const dl)
{
    _tTapeDelay* d = *dl;
    return d->lastIn;
}

void tTapeDelay_setGain (tTapeDelay* const dl, float gain)
{
    _tTapeDelay* d = *dl;
    if (gain < 0.0f)    d->gain = 0.0f;
    else                d->gain = gain;
}

float tTapeDelay_getGain (tTapeDelay* const dl)
{
    _tTapeDelay* d = *dl;
    return d->gain;
}



void    tRingBuffer_init     (tRingBuffer* const ring, int size, LEAF* const leaf)
{
    tRingBuffer_initToPool(ring, size, &leaf->mempool);
}

void    tRingBuffer_initToPool   (tRingBuffer* const ring, int size, tMempool* const mempool)
{
    _tMempool* m = *mempool;
    _tRingBuffer* r = *ring = (_tRingBuffer*) mpool_alloc(sizeof(_tRingBuffer), m);
    r->mempool = m;
    
    // Ensure size is a power of 2
    if (size <= 0) r->size = 1;
    else r->size = pow(2, ceil(log2(size)));
    r->mask = r->size - 1;
    
    r->buffer = (float*) mpool_calloc(sizeof(float) * r->size, m);
    r->pos = 0;
}

void    tRingBuffer_free     (tRingBuffer* const ring)
{
    _tRingBuffer* r = *ring;
    
    mpool_free((char*) r->buffer, r->mempool);
    mpool_free((char*) r, r->mempool);
}

void   tRingBuffer_push     (tRingBuffer* const ring, float val)
{
    _tRingBuffer* r = *ring;
    
    --r->pos;
    r->pos &= r->mask;
    r->buffer[r->pos] = val;
}

float   tRingBuffer_getNewest    (tRingBuffer* const ring)
{
    _tRingBuffer* r = *ring;
    
    return r->buffer[r->pos];
}

float   tRingBuffer_getOldest    (tRingBuffer* const ring)
{
    _tRingBuffer* r = *ring;
    
    return r->buffer[(r->pos + r->size - 1) & r->mask];
}

float   tRingBuffer_get      (tRingBuffer* const ring, int index)
{
    _tRingBuffer* r = *ring;
    
    return r->buffer[(r->pos + index) & r->mask];
}

int     tRingBuffer_getSize  (tRingBuffer* const ring)
{
    _tRingBuffer* r = *ring;
    
    return r->size;
}
