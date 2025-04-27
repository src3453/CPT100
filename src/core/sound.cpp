#include <math.h>
#include <random>
#include <iostream>
#define M_PI 3.14159265358979323846
#include "lib/effecter.cpp"
#define Byte unsigned char

class S3HS_sound {
public:
    #include "envelove.cpp"
    #ifndef MIN
    #define MIN(a,b) (((a)>(b))?(b):(a))
    #endif
    #ifndef MAX
    #define MAX(a,b) (((a)>(b))?(a):(b))
    #endif
    #ifndef CLAMP_VAR
    #define CLAMP_VAR(x,xMin,xMax) \
    if ((x)<(xMin)) (x)=(xMin); \
    if ((x)>(xMax)) (x)=(xMax);
    #endif
    std::mt19937 mt;
    long long Total_time = 0;
    int t1[8] = {0,0,0,0,0,0,0,0};
    int t2[8] = {0,0,0,0,0,0,0,0};
    int t3[8] = {0,0,0,0,0,0,0,0};
    int t4[8] = {0,0,0,0,0,0,0,0};
    int t5[8] = {0,0,0,0,0,0,0,0};
    int t6[8] = {0,0,0,0,0,0,0,0};
    int t7[8] = {0,0,0,0,0,0,0,0};
    int t8[8] = {0,0,0,0,0,0,0,0};
    unsigned long long twt[4] = {0,0,0,0};
    float in1[4]  = {0.0,0.0,0.0,0.0};
    float in2[4]  = {0.0,0.0,0.0,0.0};
    float out1[4] = {0.0,0.0,0.0,0.0};
    float out2[4] = {0.0,0.0,0.0,0.0};
    float feedback = 0;
    int vols[64] = {};
    double previous[12] = {0.0};
    std::vector<int> gateTick = {0,0,0,0,0,0,0,0};
    std::vector<Byte> reg;
    std::vector<Byte> regenvl;
    std::vector<Byte> regwt;
    std::vector<Byte> regother;
    std::vector<int> noise;
    std::vector<std::vector<signed char>> sintable;
    std::vector<EnvGenerator> envl;
    EnvGenerator _envl;
    S3HS_Effecter effecter;
    double prev = 0;
    unsigned int pcm_addr[4],pcm_addr_end[4],pcm_loop_start[4],pcm_loop_end[4]={0,0,0,0};
    std::vector<std::vector<Byte>> pcm_ram; 

    S3HS_sound(std::vector<Byte> ram) {
    };
    
    float S3HS_SAMPLE_FREQ = 48000;
    #define SINTABLE_LENGTH 256
    #define PHASE_RESOLUTION 16

    void setSampleRate(float sr) {
        S3HS_SAMPLE_FREQ = sr;
    }

    double sind(double theta) {
        return sin((double)theta*2*M_PI);
    }

    double modulate(double theta, int wf) {
        const double scale = 256.0 / S3HS_SAMPLE_FREQ;
        const double scaledTheta = theta * scale;
        
        // Calculate indices with bitwise operations for efficiency
        const int index = ((int)scaledTheta) & 0xff;
        const int nextIndex = (index + 1) & 0xff;
        
        // Get sample values
        const double pre = sintable[wf][index];
        const double nxt = sintable[wf][nextIndex];
        
        // Quick return conditions
        if (pre == nxt || std::abs(nxt - pre) > 127)
            return pre;
        
        // Calculate fractional part directly (avoid modf call)
        const double fracPart = scaledTheta - (int)scaledTheta;
        
        // Linear interpolation
        return pre + (nxt - pre) * fracPart;
    }

    double generateFMWave(double t1, double v1, double t2, double v2, double t3, double v3, double t4, double v4, int w1, int w2, int w3, int w4) {

        double value = modulate(t1+modulate(t2+modulate(t3+modulate(t4,w4)*v4,w3)*v3,w2)*v2,w1)*v1*255*127;
        return value;

    }

    double generateHSWave(int mode, double t1, double v1, double t2, double v2, double t3, double v3, double t4, double v4, double t5, double v5, double t6, double v6, double t7, double v7, double t8, double v8, int w1, int w2, int w3, int w4, int w5, int w6, int w7, int w8, float fb, int ch, double* result) {
        double value = 0;
        double phase = 0;
        double phase2 = 0;
        double phase3 = 0;
        double phase4 = 0;
        double phase5 = 0;
        double phase6 = 0;
        double phase7 = 0;
        feedback = (MIN(MAX(((result[ch]/255/127)+1.0),0),2)-1.0)*fb;
        switch (mode)
        {
        case 0:
            value = (modulate(t1,w1)*v1+modulate(t2,w2)*v2+modulate(t3,w3)*v3+modulate(t4,w4)*v4+
                    modulate(t5,w5)*v5+modulate(t6,w6)*v6+modulate(t7,w7)*v7+modulate(t8,w8)*v8+feedback)*255*127; //Additive
            break;
        case 1:
            phase = (modulate(t5,w5)*v5+modulate(t6,w6)*v6+modulate(t7,w7)*v7+modulate(t8,w8)*v8+feedback)*4*S3HS_SAMPLE_FREQ;
            value = (modulate(t1+phase,w1)*v1+modulate(t2+phase,w2)*v2+modulate(t3+phase,w3)*v3+modulate(t4+phase,w4)*v4)*255*127; //FM2op
            break;
        case 2:
            value = ((modulate(t1,w1)*v1+modulate(t2,w2)*v2+modulate(t3,w3)*v3+modulate(t4,w4)*v4)*
                    (modulate(t5,w5)*v5+modulate(t6,w6)*v6+modulate(t7,w7)*v7+modulate(t8,w8)*v8)+feedback)*255*127; //RingMod
            break;
        case 3:
            phase  = (modulate(t7,w7)*v7+modulate(t8,w8)*v8)*4*S3HS_SAMPLE_FREQ;
            phase2 = (modulate(t5+phase,w5)*v5+modulate(t6+phase,w6)*v6)*4*S3HS_SAMPLE_FREQ;
            phase3 = (modulate(t3+phase2,w3)*v3+modulate(t4+phase2,w4)*v4+feedback)*4*S3HS_SAMPLE_FREQ;
            value = (modulate(t1+phase3,w1)*v1+modulate(t2+phase3,w2)*v2)*255*127; //FM4op
            break;
        case 4:
            phase = (modulate(t8,w8)*v8)*4*S3HS_SAMPLE_FREQ;
            phase2 = (modulate(t7+phase,w7)*v7)*4*S3HS_SAMPLE_FREQ;
            phase3 = (modulate(t6+phase2,w6)*v6)*4*S3HS_SAMPLE_FREQ;
            phase4 = (modulate(t5+phase3,w5)*v5)*4*S3HS_SAMPLE_FREQ;
            phase5 = (modulate(t4+phase4,w4)*v4)*4*S3HS_SAMPLE_FREQ;
            phase6 = (modulate(t3+phase5,w3)*v3)*4*S3HS_SAMPLE_FREQ;
            phase7 = (modulate(t2+phase6,w2)*v2+feedback)*4*S3HS_SAMPLE_FREQ;
            value = (modulate(t1+phase7,w1)*v1)*255*127; //FM8op
            break;
        case 5:
            phase = (modulate(t8,w8)*v8)*4*S3HS_SAMPLE_FREQ;
            phase2 = (modulate(t7+phase,w7)*v7)*4*S3HS_SAMPLE_FREQ;
            phase3 = (modulate(t6+phase2,w6)*v6)*4*S3HS_SAMPLE_FREQ;
            phase5 = (modulate(t4,w4)*v4)*4*S3HS_SAMPLE_FREQ;
            phase6 = (modulate(t3+phase5,w3)*v3)*4*S3HS_SAMPLE_FREQ;
            phase7 = (modulate(t2+phase6,w2)*v2+feedback)*4*S3HS_SAMPLE_FREQ;
            value = (modulate(t5+phase3,w5)*v5+modulate(t1+phase7,w1)*v1)*255*127; //FM4opx2
            break;
        case 6:
            phase = (modulate(t8,w8)*v8)*4*S3HS_SAMPLE_FREQ;
            phase3 = (modulate(t6,w6)*v6)*4*S3HS_SAMPLE_FREQ;
            phase5 = (modulate(t4,w4)*v4)*4*S3HS_SAMPLE_FREQ;
            phase7 = (modulate(t2,w2)*v2+feedback)*4*S3HS_SAMPLE_FREQ;
            value = (modulate(t1+phase7,w1)*v1+modulate(t7+phase,w7)*v7+modulate(t5+phase3,w5)*v5+modulate(t3+phase5,w3)*v3)*255*127; //FM2opx4
            break;
        case 7:
            phase = (modulate(t8,w8)*v8)*4*S3HS_SAMPLE_FREQ;
            phase2 = (modulate(t7+phase,w7)*v7)*4*S3HS_SAMPLE_FREQ;
            phase3 = (modulate(t6+phase2,w6)*v6)*4*S3HS_SAMPLE_FREQ;
            phase5 = (modulate(t4,w4)*v4)*4*S3HS_SAMPLE_FREQ;
            phase6 = (modulate(t3+phase5,w3)*v3)*4*S3HS_SAMPLE_FREQ;
            phase7 = (modulate(t2+phase6,w2)*v2+feedback)*4*S3HS_SAMPLE_FREQ;
            value = (modulate(t5+phase3,w5)*v5*modulate(t1+phase7,w1)*v1)*255*127; //FM4opxRM2
            break;
        case 8:
            value = ((modulate(t1,w1)*v1+modulate(t2,w2)*v2)*(modulate(t3,w3)*v3+modulate(t4,w4)*v4)*
                    (modulate(t5,w5)*v5+modulate(t6,w6)*v6)*(modulate(t7,w7)*v7+modulate(t8,w8)*v8)+feedback)*255*127; //RingModx4
            break;
        case 9:
            phase = (modulate(t8,w8)*v8)*4*S3HS_SAMPLE_FREQ;
            phase3 = (modulate(t6,w6)*v6)*4*S3HS_SAMPLE_FREQ;
            phase5 = (modulate(t4,w4)*v4)*4*S3HS_SAMPLE_FREQ;
            phase7 = (modulate(t2,w2)*v2+feedback)*4*S3HS_SAMPLE_FREQ;
            value = (modulate(t1+phase7,w1)*v1*modulate(t7+phase,w7)*v7*modulate(t5+phase3,w5)*v5*modulate(t3+phase5,w3)*v3)*255*127; //FM2opxRM4
            break;
        case 10:
            phase = (modulate(t5,w5)*v5+modulate(t6,w6)*v6+modulate(t7,w7)*v7+modulate(t8,w8)*v8+feedback)*4*S3HS_SAMPLE_FREQ;
            value = (modulate(phase,w1)*v1+modulate(phase,w2)*v2+modulate(phase,w3)*v3+modulate(phase,w4)*v4)*255*127; //DirectPhase2op
            break;
        case 11:
            phase  = (modulate(t7,w7)*v7+modulate(t8,w8)*v8)*4*S3HS_SAMPLE_FREQ;
            phase2 = (modulate(phase,w5)*v5+modulate(phase,w6)*v6)*4*S3HS_SAMPLE_FREQ;
            phase3 = (modulate(phase2,w3)*v3+modulate(phase2,w4)*v4+feedback)*4*S3HS_SAMPLE_FREQ;
            value = (modulate(phase3,w1)*v1+modulate(phase3,w2)*v2)*255*127; //DirectPhase4op
            break;
        case 12:
            phase = (modulate(t8,w8)*v8)*4*S3HS_SAMPLE_FREQ;
            phase2 = (modulate(phase,w7)*v7)*4*S3HS_SAMPLE_FREQ;
            phase3 = (modulate(phase2,w6)*v6)*4*S3HS_SAMPLE_FREQ;
            phase4 = (modulate(phase3,w5)*v5)*4*S3HS_SAMPLE_FREQ;
            phase5 = (modulate(phase4,w4)*v4)*4*S3HS_SAMPLE_FREQ;
            phase6 = (modulate(phase5,w3)*v3)*4*S3HS_SAMPLE_FREQ;
            phase7 = (modulate(phase6,w2)*v2+feedback)*4*S3HS_SAMPLE_FREQ;
            value = (modulate(phase7,w1)*v1)*255*127; //DirectPhase8OP
            break;
        default:
            break;
        }
        
        return value;

    }

    void applyEnveloveToRegisters(std::vector<Byte> &reg, std::vector<Byte> &regenvl, int opNum, int ch, double dt) {
        ADSRConfig adsr;
        // 2025/1/24 add exponential decay
        double ADSRfactor = 1.5;
        adsr.attackTime = ((double)reg.at(32+64*ch+opNum*4+0))/64*1.5;
        adsr.attackTime = adsr.attackTime==0?0/64*1.5:adsr.attackTime;
        adsr.decayTime = ((double)reg.at(32+64*ch+opNum*4+1))/64*1.5;
        adsr.decayTime = adsr.decayTime==0?0.5/64*1.5:adsr.decayTime;
        adsr.sustainLevel = ((double)reg.at(32+64*ch+opNum*4+2))/255;
        adsr.releaseTime = ((double)reg.at(32+64*ch+opNum*4+3))/64*1.5;
        adsr.releaseTime = adsr.releaseTime==0?0/64*1.5:adsr.releaseTime;
        if (reg.at(64*ch+0x1e) == 0 && gateTick.at(ch) == 1) {
            envl.at((size_t)(ch*8+opNum)).noteOff();
            if(opNum == 7) {
                gateTick.at(ch)=0;
            }
            
        }
        if (reg.at(64*ch+0x1e) == 1 && gateTick.at(ch) == 0) {
            envl.at((size_t)(ch*8+opNum)).reset(EnvGenerator::State::Attack); 
            if(opNum == 7) {
                gateTick.at(ch)=1;
            }
        }
        //std::cout << dt << std::endl; //envl.at((size_t)(ch*4+opNum)).m_elapsed
        vols[ch*8+opNum] = (envl.at((size_t)(ch*8+opNum)).currentLevel()*255*((double)(reg.at(ch*64+opNum+16))/255));
        envl.at((size_t)(ch*8+opNum)).update(adsr,dt);
    }

    double quantizeFreqByPeriod(double freq) {
        const double masterClock = 48000 * 4.0;   
        int calculatedPeriod = std::floor(masterClock/freq);
        double quantizedFreq = masterClock/(double)calculatedPeriod;
        return quantizedFreq;
    }

    #define OVERSAMPLE_MULT 1

    std::vector<std::vector<std::vector<int16_t>>> AudioCallBack(int len)
    {
        int i;
        std::vector<int16_t> __frames(len,0);
        std::vector<std::vector<int16_t>> _frames(13,__frames);
        std::vector<std::vector<std::vector<int16_t>>> frames(2,_frames);
        std::vector<float> outL(len,0);
        std::vector<float> outR(len,0);
        std::vector<float> procL(len,0);
        std::vector<float> procR(len,0);
        int framesize = len;
        reg = ram_peek2array(ram,0x400000,512);
        regwt = ram_peek2array(ram,0x400200,192);
        regother = ram_peek2array(ram,0x4002C0,0x240);

        /*for (int wf=10; wf<14; wf++) {
            for (int i=0; i<256; i++) {
                int val = regwt.at(16+48*(wf-10)+((int)(i/8)%32));
                if ((int)((i/8)*2)%2 == 0) {
                    val /= 16;
                } else {
                    val %= 16;
                }
                val *= 16;
                val -= 128;
                sintable.at(wf).at(i) = (signed char)(val);
            }
        }*/
        for (int wf=10; wf<14; wf++) {
            for (int i=0; i<256; i++) {
                float pre = (float)regwt.at(16+48*(wf-10)+((int)(i/8)%32));
                float nxt = (float)regwt.at(16+48*(wf-10)+((int)(((float)i/8)+1)%32));
                int val = (int)(pre+(nxt-pre)*fmod((((float)i)/8),1));
                val -= 128;
                sintable.at(wf).at(i) = (signed char)(val);
            }
        }
        for (i = 0; i < framesize * OVERSAMPLE_MULT; i++) {
            double result[12] = {0};
            for(int ch=0; ch < 8; ch++) {
                for (int opNum=0; opNum < 8; opNum++) {
                    applyEnveloveToRegisters(reg,regenvl,opNum,ch,((double)1/(double)S3HS_SAMPLE_FREQ)/OVERSAMPLE_MULT);
                }
            }
            
            for(int ch=0; ch < 8; ch++) {
                int addr = 64*ch;
                int f1 = (int)(quantizeFreqByPeriod((double)reg[addr+0]*256+reg[addr+1]))*PHASE_RESOLUTION/OVERSAMPLE_MULT;
                t1[ch] = t1[ch] + f1;
                t2[ch] = t2[ch] + (int)(double)f1*(((double)reg[addr+2]*256+reg[addr+3])/4096);
                t3[ch] = t3[ch] + (int)(double)f1*(((double)reg[addr+4]*256+reg[addr+5])/4096);
                t4[ch] = t4[ch] + (int)(double)f1*(((double)reg[addr+6]*256+reg[addr+7])/4096);
                t5[ch] = t5[ch] + (int)(double)f1*(((double)reg[addr+8]*256+reg[addr+9])/4096);
                t6[ch] = t6[ch] + (int)(double)f1*(((double)reg[addr+10]*256+reg[addr+11])/4096);
                t7[ch] = t7[ch] + (int)(double)f1*(((double)reg[addr+12]*256+reg[addr+13])/4096);
                t8[ch] = t8[ch] + (int)(double)f1*(((double)reg[addr+14]*256+reg[addr+15])/4096);
                double v1 = (double)(vols[ch*8+0])/32768;
                double v2 = (double)(vols[ch*8+1])/32768;
                double v3 = (double)(vols[ch*8+2])/32768;
                double v4 = (double)(vols[ch*8+3])/32768;
                double v5 = (double)(vols[ch*8+4])/32768;
                double v6 = (double)(vols[ch*8+5])/32768;
                double v7 = (double)(vols[ch*8+6])/32768;
                double v8 = (double)(vols[ch*8+7])/32768;
                int w1 = reg[addr+24]>>4;
                int w2 = reg[addr+24]&0xf;
                int w3 = reg[addr+25]>>4;
                int w4 = reg[addr+25]&0xf;
                int w5 = reg[addr+26]>>4;
                int w6 = reg[addr+26]&0xf;
                int w7 = reg[addr+27]>>4;
                int w8 = reg[addr+27]&0xf;
                int mode = reg[addr+0x1c];
                float fb = ((float)(reg[addr+0x1f])/256-0.5)*2;
                result[ch] += generateHSWave(mode,
                (double)(t1[ch])/PHASE_RESOLUTION,v1,
                (double)(t2[ch])/PHASE_RESOLUTION,v2,
                (double)(t3[ch])/PHASE_RESOLUTION,v3,
                (double)(t4[ch])/PHASE_RESOLUTION,v4,
                (double)(t5[ch])/PHASE_RESOLUTION,v5,
                (double)(t6[ch])/PHASE_RESOLUTION,v6,
                (double)(t7[ch])/PHASE_RESOLUTION,v7,
                (double)(t8[ch])/PHASE_RESOLUTION,v8,
                w1,w2,w3,w4,w5,w6,w7,w8,fb,ch,previous);
                previous[ch] = result[ch];
                //std::cout << v1 << std::endl;
            }
            for (int ch=0;ch<4;ch++) {
                if(regwt[48*ch+3] == 0) {
                    pcm_addr[ch] = regwt[16+48*ch+0]*65536+regwt[16+48*ch+1]*256+regwt[16+48*ch+2];
                    pcm_addr_end[ch] = regwt[16+48*ch+3]*65536+regwt[16+48*ch+4]*256+regwt[16+48*ch+5];
                    pcm_loop_start[ch] = regwt[16+48*ch+6]*65536+regwt[16+48*ch+7]*256+regwt[16+48*ch+8];
                    //pcm_loop_end[ch] = regwt[12+64*ch+9]*65536+regwt[12+64*ch+10]*256+regwt[12+64*ch+11];
                    //std::cout << pcm_addr[ch] << std::endl;
                    //std::cout << pcm_addr_end[ch] << std::endl;
                }
            }
            for(int ch=0; ch<4; ch++) {
                int ft = quantizeFreqByPeriod(regwt[ch*48+0]*256+regwt[ch*48+1])*PHASE_RESOLUTION/OVERSAMPLE_MULT;
                twt[ch] = twt[ch] + ft;
                double vt = ((double)regwt[ch*48+2])/255;
                int val = 0;
                double phase = (double)(twt[ch])/PHASE_RESOLUTION/S3HS_SAMPLE_FREQ*32;
                //std::cout << ch << std::endl;
                //std::cout << pcm_addr[ch] << std::endl;
                //std::cout << pcm_addr_end[ch] << std::endl;
                //std::cout << pcm_loop_start[ch] << std::endl;

                #define fmod(val) ((val) - ((int)(val)))

                if (regwt[ch*48+3] == 4) {
                    val = regwt[16+48*ch+((int)phase%32)];
                } else if(regwt[ch*48+3] == 2) {
                    val = noise[((int)phase%65536)]*255;
                } else if(regwt[ch*48+3] == 3) {
                    val = noise[((int)phase%64)]*255;
                } else if(regwt[ch*48+3] == 1) {
                    float pre = (float)regwt[16+48*ch+((int)phase%32)];
                    float nxt = (float)regwt[16+48*ch+((int)(phase+1)%32)];
                    val = (int)(pre+(nxt-pre)*fmod((((float)phase))));
                } else if(regwt[ch*48+3] == 0) {
                    int pre,nxt;
                    if (pcm_addr[ch]+(int)phase > pcm_addr_end[ch] && pcm_loop_start[ch] < pcm_addr_end[ch] && pcm_loop_start[ch] != 0xFFFFFF) {
                        pre = ram_peek(ram,pcm_addr[ch]+((int)phase%(pcm_addr_end[ch]-pcm_loop_start[ch])));
                        nxt = ram_peek(ram,pcm_addr[ch]+((int)(phase+1)%(pcm_addr_end[ch]-pcm_loop_start[ch])));
                    } else {
                        pre = ram_peek(ram,std::min(pcm_addr[ch]+(int)phase,pcm_addr_end[ch]));
                        nxt = ram_peek(ram,std::min(pcm_addr[ch]+(int)phase+1,pcm_addr_end[ch]));
                    }
                    val = (int)(pre+((float)(nxt-pre)*fmod((((float)phase)))));
                    //std::cout << phase << std::endl;
                }

                #undef fmod
                
                val -= 128;
                double omega, alpha, a0, a1, a2, b0, b1, b2;
                switch (regwt[ch*48+4])
                {
                case 0:
                    omega = 2.0 * 3.14159265 * ((double)regwt[ch*48+5]+1)*8 / S3HS_SAMPLE_FREQ * OVERSAMPLE_MULT;
                    alpha = sin(omega) / (2.0 * 0.5+((double)regwt[ch*48+6]+1)/16);
                    a0 =  1.0 + alpha;
                    a1 = -2.0 * cos(omega);
                    a2 =  1.0 - alpha;
                    b0 = (1.0 - cos(omega)) / 2.0;
                    b1 =  1.0 - cos(omega);
                    b2 = (1.0 - cos(omega)) / 2.0;
                    break;
                case 1:
                    omega = 2.0f * 3.14159265f *  ((double)regwt[ch*48+5]+1)*8 / S3HS_SAMPLE_FREQ * OVERSAMPLE_MULT;
                    alpha = sin(omega) / (2.0f * 0.5+((double)regwt[ch*48+6]+1)/16);
                    a0 =   1.0f + alpha;
                    a1 =  -2.0f * cos(omega);
                    a2 =   1.0f - alpha;
                    b0 =  (1.0f + cos(omega)) / 2.0f;
                    b1 = -(1.0f + cos(omega));
                    b2 =  (1.0f + cos(omega)) / 2.0f;
                    break;
                case 2:
                    omega = 2.0f * 3.14159265f * ((double)regwt[ch*48+5]+1)*8 / S3HS_SAMPLE_FREQ * OVERSAMPLE_MULT;
                    alpha = sin(omega) * sinh(log(2.0f) / 1.0 * ((double)regwt[ch*48+6]+1)/256 * omega / sin(omega));
                    a0 =  1.0f + alpha;
                    a1 = -2.0f * cos(omega);
                    a2 =  1.0f - alpha;
                    b0 =  alpha;
                    b1 =  0.0f;
                    b2 = -alpha;
                    break;
                case 3:
                    omega = 2.0f * 3.14159265f *  ((double)regwt[ch*48+5]+1)*8 / S3HS_SAMPLE_FREQ * OVERSAMPLE_MULT;
                    alpha = sin(omega) * sinh(log(2.0f) / 1.0 * ((double)regwt[ch*48+6]+1)/256 * omega / sin(omega));
                    a0 =  1.0f + alpha;
                    a1 = -2.0f * cos(omega);
                    a2 =  1.0f - alpha;
                    b0 =  1.0f;
                    b1 = -2.0f * cos(omega);
                    b2 =  1.0f;
                    break;
                default:
                    omega = 2.0 * 3.14159265 * ((double)regwt[ch*48+5]+1)*8 / S3HS_SAMPLE_FREQ * OVERSAMPLE_MULT;
                    alpha = sin(omega) / (2.0 * 0.5+((double)regwt[ch*48+6]+1)/64);
                    a0 =  1.0 + alpha;
                    a1 = -2.0 * cos(omega);
                    a2 =  1.0 - alpha;
                    b0 = (1.0 - cos(omega)) / 2.0;
                    b1 =  1.0 - cos(omega);
                    b2 = (1.0 - cos(omega)) / 2.0;
                    break;
                }
                
                double output = b0/a0*(double)val+b1/a0*in1[ch]+b2/a0*in2[ch]-a1/a0*out1[ch]-a2/a0*out2[ch];
                in2[ch]  = in1[ch];
                in1[ch]  = val; 
                out2[ch] = out1[ch];     
                out1[ch] = output; 
                if (regwt[ch*48+5] == 0) {
                    result[ch+8] += (double)(val)*255*vt;
                } else {
                    result[ch+8] += std::min(std::max((double)output*255*vt,-32768.0),32767.0);
                }
                //result[ch+8] += (double)(val)*255*vt;
            }
            
            for(int ch=0; ch<12; ch++) {
                int panL, panR;
                if (ch < 8) {
                    panL = reg[0x1d+64*ch]>>4;
                    panR = reg[0x1d+64*ch]&0xf;
                } else {
                    panL = regwt[0x07+48*(ch-8)]>>4;
                    panR = regwt[0x07+48*(ch-8)]&0xf;
                }
                if (panL == 0 && panR == 0) {
                    panL = 15;
                    panR = 15;
                }
                if (!regother[0x010+ch] == 1) {
                    frames[0][ch][i/OVERSAMPLE_MULT] += (int16_t)std::min(std::max(result[ch]*((double)(panL)/15),-32768.0),32767.0)/OVERSAMPLE_MULT;
                    frames[1][ch][i/OVERSAMPLE_MULT] += (int16_t)std::min(std::max(result[ch]*((double)(panR)/15),-32768.0),32767.0)/OVERSAMPLE_MULT;
                    outL[i/OVERSAMPLE_MULT] += std::min(std::max(result[ch]*((double)(panL)/15),-32768.0),32767.0)/OVERSAMPLE_MULT/32768.0;
                    outR[i/OVERSAMPLE_MULT] += std::min(std::max(result[ch]*((double)(panR)/15),-32768.0),32767.0)/OVERSAMPLE_MULT/32768.0;
                }
            }

        }
        procL = outL;
        procR = outR;

        // Master -> EQ -> Compressor -> Final Output

        if(regother[0x001] == 1) {
            float lowgain = (float)(regother[0x005])/8;
            float midgain = (float)(regother[0x006])/8;
            float highgain = (float)(regother[0x007])/8;
            std::vector<std::vector<float>> out = effecter.EQ3band(procL,procR,framesize,lowgain,midgain,highgain);
            procL = out[0];
            procR = out[1];
            //printf("EQ3band %f %f %f\n",lowgain,midgain,highgain);
        } else {
            //procL = outL;
            //procR = outR;
        }
        //printf("Register %x %x\n",regother[0x000],regother[0x001]);
        if(regother[0x000] == 1) {
            float threshold = (float)(regother[0x002])/255;
            float ratio = (float)(regother[0x003])/255; 
            float volume = (float)(regother[0x004])/32;
            std::vector<std::vector<float>> out = effecter.Compressor(procL,procR,framesize,threshold,ratio,volume);
            procL = out[0];
            procR = out[1];
            //printf("Compressor %f %f %f\n",threshold,ratio,volume);
        } else {
            //procL = outL;
            //procR = outR;
        }
       
        outL = procL;
        outR = procR;
        for (int i=0;i<framesize*OVERSAMPLE_MULT;i++) {
            double tmpL = outL[i/OVERSAMPLE_MULT]*32767.0/8;
            double tmpR = outR[i/OVERSAMPLE_MULT]*32767.0/8;
            if (tmpL < -32768.0) tmpL = -32768.0;
            if (tmpL > 32767.0) tmpL = 32767.0;
            if (tmpR < -32768.0) tmpR = -32768.0;
            if (tmpR > 32767.0) tmpR = 32767.0;
            frames[0][12][i/OVERSAMPLE_MULT] = (int16_t)tmpL;
            frames[1][12][i/OVERSAMPLE_MULT] = (int16_t)tmpR;
        }
        reg.clear();
        regenvl.clear();
        regwt.clear();
        Total_time++;
        return frames;
    }

    void initSound() {
        mt.seed(0);
        envl.resize(64,_envl);
        noise.resize(65536,0);
        std::vector<signed char> _sintable;
        _sintable.resize(256,0);
        sintable.resize(16,_sintable);
        for (int i=0;i<65536;i++) {
            noise[i] = mt()%2;
        }
        for (int i=0; i<256; i++) {
            sintable.at(0).at(i) = (signed char)(sind((double)i/256)*127);
            
            
            sintable.at(1).at(i) = (signed char)(std::max(0.0,sind((double)i/256))*127);
            double qi = (double)((int)((double)i/(256/SINTABLE_LENGTH))*256/SINTABLE_LENGTH);
            sintable.at(4).at((i+64)%256) = (signed char)((i<128?(double)qi/64-1.0:(double)qi/-64+3.0)*127);
            sintable.at(5).at((i+128)%256) = (signed char)(((double)qi/128-1.0)*127);
            sintable.at(7).at((i+64)%256) = (signed char)(MAX(i<128?(double)qi/64-1.0:(double)qi/-64+3.0,0.0)*127);
            sintable.at(8).at((i+128)%256) = (signed char)(MAX((double)qi/128-1.0,0.0)*127);
            //sintable.at(9).at(i) = (double)(mt()%2*2-1);
            sintable.at(9).at(i) = (signed char)((mt()&1)*255-128);
            if (i<128) {
                sintable.at(15).at(i) = (signed char)(abs(sind((double)i/128))*127);
                sintable.at(14).at(i) = (signed char)(abs(sind((double)i/128))*127);
                sintable.at(2).at(i) = (signed char)(sind((double)i/128)*127);
                sintable.at(3).at(i) = 127;
                sintable.at(6).at(i) = 127;
            } else {
                sintable.at(15).at(i) = (signed char)(-abs(sind((double)i/128))*127);
                sintable.at(3).at(i) = -128;
            }
        }
        
        for (int addr=400000;addr<0x400400;addr++) {
            ram_poke(ram,addr,0x00);
        }
    }

    void resetGate(int ch) {
        for (int i=0;i<8;i++) {
            envl.at((size_t)(ch*8+i)).reset(EnvGenerator::State::Attack); 
        }
        t1[ch] = 0;
        t2[ch] = 0;
        t3[ch] = 0;
        t4[ch] = 0;
        t5[ch] = 0;
        t6[ch] = 0;
        t7[ch] = 0;
        t8[ch] = 0;
    }

    void wtSync(int ch) {
        twt[ch]=0;
    }
    
};