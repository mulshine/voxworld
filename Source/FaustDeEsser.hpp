#ifndef DeEsserDSPKernel_h
#define DeEsserDSPKernel_h

#include "FaustUtilities.h"

class DeEsser {
public:
    // MARK: Member Functions

    DeEsser(float sampleRate)
    {
        init(sampleRate);
    }

    ~DeEsser() {
    }

    void init(int sample_rate) {
        fSampleRate = float(sample_rate);
    
        fConst0 = std::min<float>(192000.0f, std::max<float>(1.0f, float(fSampleRate)));
        fConst1 = (1.0f / fConst0);
        fConst2 = mydsp_faustpower2_f(fConst1);
        fConst3 = (3.1622777f * fConst2);
        fConst4 = (2.0f * fConst0);
        fConst5 = (3.14159274f / fConst0);
        fConst6 = (0.5f / fConst0);
        fConst7 = (4.0f * mydsp_faustpower2_f(fConst0));
        fConst8 = (3.12144518f / fConst0);
        fConst9 = (8.88912392f / fConst0);
        fConst10 = (13.3035135f / fConst0);
        fConst11 = (15.692564f / fConst0);
        fConst12 = mydsp_faustpower3_f(fConst1);
        fConst13 = (4.0f * fConst1);
        fConst14 = (31.385128f / fConst0);
        fConst15 = (6.0f * fConst2);
        fConst16 = (26.6070271f / fConst0);
        fConst17 = (17.7782478f / fConst0);
        fConst18 = (6.24289036f / fConst0);
        
        lad = FAUSTFLOAT(0.001f);
        attack = FAUSTFLOAT(0.00020000000000000001f);
        release = FAUSTFLOAT(0.035000000000000003f);
        threshold = FAUSTFLOAT(-50.0f);
        ratio = FAUSTFLOAT(6.0f);
        
        freq = FAUSTFLOAT(6500.0);
        width = FAUSTFLOAT(50.0f);
    }
    
    void compute(int count, const float* input, float* output)  {
        int iSlow0 = int(std::max<float>(0.0f, std::floor(((fConst0 * float(lad)) + 0.5f))));
        float fSlow1 = float(attack);
        float fSlow2 = (0.5f * fSlow1);
        int iSlow3 = (std::fabs(fSlow2) < 1.1920929e-07f);
        float fSlow4 = (iSlow3 ? 0.0f : std::exp((0.0f - (fConst1 / (iSlow3 ? 1.0f : fSlow2)))));
        float fSlow5 = ((1.0f / std::max<float>(1.1920929e-07f, float(ratio))) + -1.0f);
        float fSlow6 = float(width);
        float fSlow7 = float(freq);
        float fSlow8 = std::tan((fConst5 * (fSlow6 + fSlow7)));
        float fSlow9 = std::sqrt((fConst7 * (fSlow8 * std::tan((fConst5 * (fSlow7 - fSlow6))))));
        float fSlow10 = mydsp_faustpower2_f(fSlow9);
        float fSlow11 = ((fConst4 * fSlow8) - (fConst6 * (fSlow10 / fSlow8)));
        float fSlow12 = mydsp_faustpower2_f(fSlow11);
        float fSlow13 = (4.0f * fSlow12);
        float fSlow14 = (fSlow13 + (8.0f * fSlow10));
        float fSlow15 = (fConst1 * fSlow10);
        float fSlow16 = (0.780361295f * fSlow11);
        float fSlow17 = (fConst8 * fSlow11);
        float fSlow18 = (((fConst2 * (fSlow14 + (fConst1 * (fSlow10 * (fSlow15 + fSlow16))))) + fSlow17) + 16.0f);
        float fSlow19 = (fConst3 / fSlow18);
        float fSlow20 = (8.0f * fSlow12);
        float fSlow21 = (0.0f - fSlow20);
        float fSlow22 = (2.22228098f * fSlow11);
        float fSlow23 = (fConst9 * fSlow11);
        float fSlow24 = (((fConst2 * (fSlow14 + (fConst1 * (fSlow10 * (fSlow15 + fSlow22))))) + fSlow23) + 16.0f);
        float fSlow25 = (fConst2 / fSlow24);
        float fSlow26 = (3.32587838f * fSlow11);
        float fSlow27 = (fConst10 * fSlow11);
        float fSlow28 = (((fConst2 * (fSlow14 + (fConst1 * (fSlow10 * (fSlow15 + fSlow26))))) + fSlow27) + 16.0f);
        float fSlow29 = (fConst2 / fSlow28);
        float fSlow30 = (3.923141f * fSlow11);
        float fSlow31 = (fConst11 * fSlow11);
        float fSlow32 = (((fConst2 * (fSlow14 + (fConst1 * (fSlow10 * (fSlow15 + fSlow30))))) + fSlow31) + 16.0f);
        float fSlow33 = (fConst2 / fSlow32);
        float fSlow34 = (1.0f / fSlow32);
        float fSlow35 = (fConst13 * fSlow10);
        float fSlow36 = (7.84628201f * fSlow11);
        float fSlow37 = (fConst14 * fSlow11);
        float fSlow38 = ((fConst12 * (fSlow10 * (fSlow35 + fSlow36))) + (-64.0f - fSlow37));
        float fSlow39 = ((fConst2 * ((0.0f - (fSlow20 + (16.0f * fSlow10))) + (fConst15 * mydsp_faustpower4_f(fSlow9)))) + 96.0f);
        float fSlow40 = ((fSlow37 + (fConst12 * (fSlow10 * (fSlow35 - fSlow36)))) + -64.0f);
        float fSlow41 = ((fConst2 * (fSlow14 + (fConst1 * (fSlow10 * (fSlow15 - fSlow30))))) + (16.0f - fSlow31));
        float fSlow42 = (1.0f / fSlow28);
        float fSlow43 = (6.65175676f * fSlow11);
        float fSlow44 = (fConst16 * fSlow11);
        float fSlow45 = ((fConst12 * (fSlow10 * (fSlow35 + fSlow43))) + (-64.0f - fSlow44));
        float fSlow46 = ((fSlow44 + (fConst12 * (fSlow10 * (fSlow35 - fSlow43)))) + -64.0f);
        float fSlow47 = ((fConst2 * (fSlow14 + (fConst1 * (fSlow10 * (fSlow15 - fSlow26))))) + (16.0f - fSlow27));
        float fSlow48 = (1.0f / fSlow24);
        float fSlow49 = (4.44456196f * fSlow11);
        float fSlow50 = (fConst17 * fSlow11);
        float fSlow51 = ((fConst12 * (fSlow10 * (fSlow35 + fSlow49))) + (-64.0f - fSlow50));
        float fSlow52 = ((fSlow50 + (fConst12 * (fSlow10 * (fSlow35 - fSlow49)))) + -64.0f);
        float fSlow53 = ((fConst2 * (fSlow14 + (fConst1 * (fSlow10 * (fSlow15 - fSlow22))))) + (16.0f - fSlow23));
        float fSlow54 = (1.0f / fSlow18);
        float fSlow55 = (1.56072259f * fSlow11);
        float fSlow56 = (fConst18 * fSlow11);
        float fSlow57 = ((fConst12 * (fSlow10 * (fSlow35 + fSlow55))) + (-64.0f - fSlow56));
        float fSlow58 = ((fSlow56 + (fConst12 * (fSlow10 * (fSlow35 - fSlow55)))) + -64.0f);
        float fSlow59 = ((fConst2 * (fSlow14 + (fConst1 * (fSlow10 * (fSlow15 - fSlow16))))) + (16.0f - fSlow17));
        int iSlow60 = (std::fabs(fSlow1) < 1.1920929e-07f);
        float fSlow61 = (iSlow60 ? 0.0f : std::exp((0.0f - (fConst1 / (iSlow60 ? 1.0f : fSlow1)))));
        float fSlow62 = float(release);
        int iSlow63 = (std::fabs(fSlow62) < 1.1920929e-07f);
        float fSlow64 = (iSlow63 ? 0.0f : std::exp((0.0f - (fConst1 / (iSlow63 ? 1.0f : fSlow62)))));
        float fSlow65 = float(threshold);
        float fSlow66 = (1.0f - fSlow4);
        for (int i = 0; (i < count); i = (i + 1)) {
            fVec0[(IOTA & 2047)] = float(input[i]); // was input0, then input1
            fRec6[0] = (float(input[i]) - (fSlow34 * ((((fSlow38 * fRec6[1]) + (fSlow39 * fRec6[2])) + (fSlow40 * fRec6[3])) + (fSlow41 * fRec6[4]))));
            fRec5[0] = ((fSlow33 * (((fSlow21 * fRec6[2]) + (fSlow13 * fRec6[0])) + (fSlow13 * fRec6[4]))) - (fSlow42 * ((((fSlow45 * fRec5[1]) + (fSlow39 * fRec5[2])) + (fSlow46 * fRec5[3])) + (fSlow47 * fRec5[4]))));
            fRec4[0] = ((fSlow29 * (((fSlow21 * fRec5[2]) + (fSlow13 * fRec5[0])) + (fSlow13 * fRec5[4]))) - (fSlow48 * ((((fSlow51 * fRec4[1]) + (fSlow39 * fRec4[2])) + (fSlow52 * fRec4[3])) + (fSlow53 * fRec4[4]))));
            fRec3[0] = ((fSlow25 * (((fSlow21 * fRec4[2]) + (fSlow13 * fRec4[0])) + (fSlow13 * fRec4[4]))) - (fSlow54 * ((((fSlow57 * fRec3[1]) + (fSlow39 * fRec3[2])) + (fSlow58 * fRec3[3])) + (fSlow59 * fRec3[4]))));
            float fTemp0 = std::fabs((fSlow19 * (((fSlow21 * fRec3[2]) + (fSlow13 * fRec3[0])) + (fSlow13 * fRec3[4]))));
            float fTemp1 = ((fRec1[1] > fTemp0) ? fSlow64 : fSlow61);
            fRec2[0] = ((fRec2[1] * fTemp1) + (fTemp0 * (1.0f - fTemp1)));
            fRec1[0] = fRec2[0];
            fRec0[0] = ((fRec0[1] * fSlow4) + (fSlow5 * (std::max<float>(((20.0f * std::log10(fRec1[0])) - fSlow65), 0.0f) * fSlow66)));
            float fTemp2 = (fVec0[((IOTA - iSlow0) & 2047)] * std::pow(10.0f, (0.0500000007f * fRec0[0])));
            output[i] = FAUSTFLOAT(fTemp2);
            IOTA = (IOTA + 1);
            for (int j0 = 4; (j0 > 0); j0 = (j0 - 1)) {
                fRec6[j0] = fRec6[(j0 - 1)];
            }
            for (int j1 = 4; (j1 > 0); j1 = (j1 - 1)) {
                fRec5[j1] = fRec5[(j1 - 1)];
            }
            for (int j2 = 4; (j2 > 0); j2 = (j2 - 1)) {
                fRec4[j2] = fRec4[(j2 - 1)];
            }
            for (int j3 = 4; (j3 > 0); j3 = (j3 - 1)) {
                fRec3[j3] = fRec3[(j3 - 1)];
            }
            fRec2[1] = fRec2[0];
            fRec1[1] = fRec1[0];
            fRec0[1] = fRec0[0];
        }
    }

    // MARK: Member Variables

private:
    float sampleRate = 44100.0;
    
    // from Faust
    int IOTA;
    float fVec0[2048];
    int fSampleRate;
    float fConst0;
    float fConst1;
    float fConst2;
    float fConst3;
    float fConst4;
    float fConst5;
    float fConst6;
    float fConst7;
    float fConst8;
    float fConst9;
    float fConst10;
    float fConst11;
    float fConst12;
    float fConst13;
    float fConst14;
    float fConst15;
    float fRec6[5];
    float fConst16;
    float fRec5[5];
    float fConst17;
    float fRec4[5];
    float fConst18;
    float fRec3[5];
    float fRec2[2];
    float fRec1[2];
    float fRec0[2];
    
    FAUSTFLOAT lad;
    FAUSTFLOAT attack;
    FAUSTFLOAT ratio;
    FAUSTFLOAT width;
    FAUSTFLOAT freq;
    FAUSTFLOAT release;
    FAUSTFLOAT threshold;
};

#endif /* DeEsserDSPKernel_h */
