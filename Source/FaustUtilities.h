/*
  ==============================================================================

    FaustUtilities.h
    Created: 2 Jun 2021 10:57:54am
    Author:  airship

  ==============================================================================
*/

#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <math.h>

#ifndef FAUSTFLOAT
#define FAUSTFLOAT float
#endif

// from Faust
static float mydsp_faustpower2_f(float value) {
    return (value * value);
}
static float mydsp_faustpower3_f(float value) {
    return ((value * value) * value);
}
static float mydsp_faustpower4_f(float value) {
    return (((value * value) * value) * value);
}
