#include <math.h>

#include "envelove.cpp"

#define SAMPLE_FREQ 48000
#define SOUND_CLOCK 400
#define SOUND_CHUNK SAMPLE_FREQ/SOUND_CLOCK

SDL_AudioSpec want, have;
SDL_AudioStream *stream = SDL_NewAudioStream(AUDIO_S16, 1, SAMPLE_FREQ, AUDIO_F32, 2, SAMPLE_FREQ);
SDL_AudioDeviceID dev;

long long Total_time = 0;
double t1[4] = {0,0,0,0};
double t2[4] = {0,0,0,0};
double t3[4] = {0,0,0,0};
double t4[4] = {0,0,0,0};
double twt[2] = {0,0};
double in1[2]  = {0.0,0.0};
double in2[2]  = {0.0,0.0};
double out1[2] = {0.0,0.0};
double out2[2] = {0.0,0.0};
std::vector<int> gateTick = {0,0,0,0};
std::vector<Byte> reg,regenvl,regwt;
std::vector<EnvGenerator> envl;
EnvGenerator _envl;
double prev = 0;
double sind(double theta) {
    return sin(theta*2*M_PI);
}

double generateFMWave(double t1, double v1, double t2, double v2, double t3, double v3, double t4, double v4) {

    double value = sind(t1+sind(t2+sind(t3+sind(t4)*v4)*v3)*v2)*v1*255*255;
    return value;

}

void applyEnveloveToRegisters(std::vector<Byte> &reg, std::vector<Byte> &regenvl, int opNum, int ch, double dt) {
    ADSRConfig adsr;
    adsr.attackTime = ((double)regenvl.at(ch*16+opNum*4+0).toInt())/64;
    adsr.decayTime = ((double)regenvl.at(ch*16+opNum*4+1).toInt())/64;
    adsr.sustainLevel = ((double)regenvl.at(ch*16+opNum*4+2).toInt())/255;
    adsr.releaseTime = ((double)regenvl.at(ch*16+opNum*4+3).toInt())/64;
    if (regenvl.at(64+ch).toInt() == 0 && gateTick.at(ch) == 1) {
        envl.at((size_t)(ch*4+opNum)).noteOff();
        if(opNum == 3) {
            gateTick.at(ch)=0;
        }
        
    }
    if (regenvl.at(64+ch).toInt() == 1 && gateTick.at(ch) == 0) {
        envl.at((size_t)(ch*4+opNum)).reset(EnvGenerator::State::Attack); 
        if(opNum == 3) {
            gateTick.at(ch)=1;
        }
    }
    //std::cout << dt << std::endl; //envl.at((size_t)(ch*4+opNum)).m_elapsed
    reg.at(ch*16+opNum+5) = (Byte)(envl.at((size_t)(ch*4+opNum)).currentLevel()*255*((double)(reg.at(ch*16+opNum+9).toInt())/255));
    envl.at((size_t)(ch*4+opNum)).update(adsr,dt);
}

void AudioCallBack(void *unused, Uint8 *stream, int len)
{
    int i;
    Uint16 *frames = (Uint16 *) stream;
    int framesize = len/2;
    //lua["SOUNDTICK"](); //120Hz
    reg = ram_peek2array(ram,0x10000,64);
    regenvl = ram_peek2array(ram,0x10040,68);
    regwt = ram_peek2array(ram,0x10084,76);
    for(int ch=0; ch < 4; ch++) {
        for (int opNum=0; opNum < 4; opNum++) {
            applyEnveloveToRegisters(reg,regenvl,opNum,ch,((double)SOUND_CHUNK/(double)SAMPLE_FREQ));
            ram_poke(ram,0x10000+16*ch+opNum+5,reg.at(16*ch+opNum+5));
        }
    }
    for (i = 0; i < framesize; i++) {
        double result = 0;
        for(int ch=0; ch < 4; ch++) {
            int addr = 16*ch;
            double f1 = ((double)reg.at(addr+0).toInt()*256+reg.at(addr+1).toInt());
            t1[ch] = t1[ch] + (f1/SAMPLE_FREQ);
            double v1 = ((double)reg.at(addr+5).toInt())/255;
            t2[ch] = t2[ch] + ((double)(f1*reg.at(addr+2).toInt()))/16/SAMPLE_FREQ;
            double v2 = ((double)reg.at(addr+6).toInt())/128;
            t3[ch] = t3[ch] + ((double)(f1*reg.at(addr+3).toInt()))/16/SAMPLE_FREQ;
            double v3 = ((double)reg.at(addr+7).toInt())/128;
            t4[ch] = t4[ch] + ((double)(f1*reg.at(addr+4).toInt()))/16/SAMPLE_FREQ;
            double v4 = ((double)reg.at(addr+8).toInt())/128;
            result += generateFMWave(t1[ch],v1,t2[ch],v2,t3[ch],v3,t4[ch],v4);
        }
        for(int ch=0; ch<2; ch++) {
            int addr = 32*ch;
            double ft = ((double)regwt.at(ch*2+0).toInt()*256+regwt.at(ch*2+1).toInt());
            twt[ch] = twt[ch] + (ft/SAMPLE_FREQ)*32;
            double vt = ((double)regwt.at(ch+4).toInt())/255;
            int val = 0;
            if (regwt.at(ch+6).toInt() == 1) {
                val = regwt.at(12+32*ch+((int)twt[ch]%32)).toInt();
                if ((int)(twt[ch]*2)%2 == 0) {
                    val /= 16;
                } else {
                    val %= 16;
                }
                val *= 16;
            } else {
                val = regwt.at(12+32*ch+((int)twt[ch]%32)).toInt();
            }
            val -= 128;
            double omega = 2.0 * 3.14159265 * ((double)regwt.at(ch+8).toInt()+1)*32 / SAMPLE_FREQ;
            double alpha = sin(omega) / (2.0 * 2.0);
            double a0 =  1.0 + alpha;
            double a1 = -2.0 * cos(omega);
            double a2 =  1.0 - alpha;
            double b0 = (1.0 - cos(omega)) / 2.0;
            double b1 =  1.0 - cos(omega);
            double b2 = (1.0 - cos(omega)) / 2.0;
            double output = b0/a0*(double)val+b1/a0*in1[ch]+b2/a0*in2[ch]-a1/a0*out1[ch]-a2/a0*out2[ch];
		    in2[ch]  = in1[ch];
		    in1[ch]  = val; 
		    out2[ch] = out1[ch];     
		    out1[ch] = output; 
            if (regwt.at(ch+8).toInt() == 255) {
                result += (double)(val)*255*vt;
            } else {
                result += (double)(output)*255*vt;
            }
        }
        result /= 6;
        frames[i] = (Sint16)result;
    }

    reg.clear();
    regenvl.clear();
    regwt.clear();
    prev = (double)clock()/1000;
    Total_time++;
}

void initSound() {
    
    envl.resize(16,_envl);
    
    for (int addr=0x10090;addr<0x100d0;addr++) {
        ram_poke(ram,addr,0x00);
    }

    int count = SDL_GetNumAudioDevices(0);
    want.freq = SAMPLE_FREQ;
    want.format = AUDIO_S16;
    want.channels = 1;
    want.samples = SOUND_CHUNK;
    want.callback = AudioCallBack;

    dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);

    SDL_PauseAudioDevice(dev, 0);
}

void resetGate(int ch) {
    for (int i=0;i<4;i++) {
        envl.at((size_t)(ch*4+i)).reset(EnvGenerator::State::Attack); 
    }
}