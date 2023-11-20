#include <math.h>

#include "envelove.cpp"

#define SAMPLE_FREQ 48000
#define SOUND_CHUNK 64

SDL_AudioSpec want, have;
SDL_AudioStream *stream = SDL_NewAudioStream(AUDIO_S16, 1, SAMPLE_FREQ, AUDIO_F32, 2, SAMPLE_FREQ);
SDL_AudioDeviceID dev;

long long Total_time = 0;
double t1[4] = {0,0,0,0};
double t2[4] = {0,0,0,0};
double t3[4] = {0,0,0,0};
double t4[4] = {0,0,0,0};
std::vector<int> gateTick = {0,0,0,0};
std::vector<Byte> reg,regenvl;
std::vector<EnvGenerator> envl;
double prev = 0;
double sind(double theta) {
    return sin(theta*4*M_PI);
}

double generateFMWave(double t1, double v1, double t2, double v2, double t3, double v3, double t4, double v4) {

    double value = sind(t1+sind(t2+sind(t3+sind(t4)*v4)*v3)*v2)*v1*96*255;
    return value;

}

void applyEnveloveToRegisters(std::vector<EnvGenerator> *envl, std::vector<Byte> &reg, std::vector<Byte> &regenvl, int opNum, int ch, double dt) {
    ADSRConfig adsr;
    EnvGenerator *envelove = &envl->at((size_t)(ch*4+opNum));
    adsr.attackTime = ((double)regenvl.at(ch*16+opNum*4+0).toInt())/64;
    adsr.decayTime = ((double)regenvl.at(ch*16+opNum*4+1).toInt())/64;
    adsr.sustainLevel = ((double)regenvl.at(ch*16+opNum*4+2).toInt())/255;
    adsr.releaseTime = ((double)regenvl.at(ch*16+opNum*4+3).toInt())/64;
    if (regenvl.at(64+ch).toInt() == 0 && gateTick.at(ch) == 1) {
        envelove->noteOff();
        if(opNum == 3) {
            gateTick.at(ch)=0;
        }
        
    }
    if (regenvl.at(64+ch).toInt() == 1 && gateTick.at(ch) == 0) {
        envelove->reset(EnvGenerator::State::Attack); 
        if(opNum == 3) {
            gateTick.at(ch)=1;
        }
    }
    //std::cout << envelove->currentLevel() << std::endl;
    reg.at(ch*16+opNum+5) = (Byte)(envelove->currentLevel()*255*((double)(reg.at(ch*16+opNum+9).toInt())/255));
    envelove->update(adsr,dt);
}

void AudioCallBack(void *unused, Uint8 *stream, int len)
{
    int i;
    Uint16 *frames = (Uint16 *) stream;
    int framesize = len/2;
    reg = ram_peek2array(ram,0x10000,63);
    regenvl = ram_peek2array(ram,0x10040,68);
    for (i = 0; i < framesize; i++) {
        double result = 0;
        for(int ch=0; ch < 4; ch++) {
            int addr = 16*ch;
            for (int opNum=0; opNum < 4; opNum++) {
                applyEnveloveToRegisters(&envl,reg,regenvl,opNum,ch,((double)clock()/1000-prev)/SOUND_CHUNK);
                ram_poke(ram,0x10000+addr+opNum+5,reg.at(addr+opNum+5));
            }
            double f1 = ((double)reg.at(addr+0).toInt()*256+reg.at(addr+1).toInt());
            t1[ch] = t1[ch] + (f1/SAMPLE_FREQ);
            double v1 = ((double)reg.at(addr+5).toInt())/255;
            t2[ch] = t2[ch] + ((double)(f1*reg.at(addr+2).toInt()))/16/SAMPLE_FREQ;
            double v2 = ((double)reg.at(addr+6).toInt())/128;
            t3[ch] = t3[ch] + ((double)(f1*reg.at(addr+3).toInt()))/16/SAMPLE_FREQ;
            double v3 = ((double)reg.at(addr+7).toInt())/128;
            t4[ch] = t4[ch] + ((double)(f1*reg.at(addr+4).toInt()))/16/SAMPLE_FREQ;
            double v4 = ((double)reg.at(addr+8).toInt())/128;
            result /= 2;
            result += generateFMWave(t1[ch],v1,t2[ch],v2,t3[ch],v3,t4[ch],v4);
        }
        frames[i] = (Sint16)result;
        Total_time++;
    }
    reg.clear();
    regenvl.clear();
    prev = (double)clock()/1000;
}

void initSound() {
    
    envl.resize(16);
    
    int count = SDL_GetNumAudioDevices(0);
    want.freq = SAMPLE_FREQ;
    want.format = AUDIO_S16;
    want.channels = 1;
    want.samples = SOUND_CHUNK;
    want.callback = AudioCallBack;

    dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);

    SDL_PauseAudioDevice(dev, 0);
}
