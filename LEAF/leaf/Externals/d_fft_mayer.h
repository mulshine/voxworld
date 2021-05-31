/*
 * d_fft_mayer.h
 *
 * Created: 5/4/2017 10:33:59 PM
 *  Author: Kenny
 */ 


#ifndef D_FFT_MAYER_H_
#define D_FFT_MAYER_H_

#ifdef __cplusplus
extern "C" {
#endif


#include "../Inc/leaf-mempool.h"


typedef float t_sample;       /* a float type at most the same size */

#define REAL t_sample
#define GOOD_TRIG

#ifdef GOOD_TRIG
#else
#define FAST_TRIG
#endif

typedef struct _tMayer
{
    tMempool mempool;
    float* halsec;
    float* costab;
    float* sintab;
    float* coswrk;
    float* sinwrk;
    
} _tMayer;

typedef _tMayer* tMayer;

void mayer_initToPool(tMayer* const mayer, tMempool* const mp);
void mayer_free(tMayer* const mayer);
void mayer_fht(tMayer* const, REAL *fz, int n);
void mayer_fft(tMayer* const, int n, REAL *real, REAL *imag);
void mayer_ifft(tMayer* const, int n, REAL *real, REAL *imag);
void mayer_realfft(tMayer* const, int n, REAL *real);
void mayer_realifft(tMayer* const, int n, REAL *real);


#ifdef __cplusplus
}
#endif

#endif /* D_FFT_MAYER_H_ */
