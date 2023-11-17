#include <math.h>

#define SAMPLE_FREQ 48000
#define SOUND_CHUNK 2

#include "ringBuffer.cpp"

SDL_AudioSpec want, have;
SDL_AudioStream *stream = SDL_NewAudioStream(AUDIO_S16, 1, SAMPLE_FREQ, AUDIO_F32, 2, SAMPLE_FREQ);
SDL_AudioDeviceID dev;

long long Total_time = 0;
std::vector<double> t1 = {0,0,0,0};
std::vector<double> t2 = {0,0,0,0};
std::vector<double> t3 = {0,0,0,0};
std::vector<double> t4 = {0,0,0,0};

double sind(double theta) {
    return sin(theta*2*M_PI);
}

double generateFMWave(double t1, double v1, double t2, double v2, double t3, double v3, double t4, double v4) {

    double value = sind(t1+sind(t2+sind(t3+sind(t4)*v4)*v3)*v2)*v1*127*255;
    return value;

}

void AudioCallBack(void *unused, Uint8 *stream, int len)
{
    int i;
    Uint16 *frames = (Uint16 *) stream;
    int framesize = len;
    std::vector<Byte> reg = ram_peek2array(ram,0x10000,63);

    for (i = 0; i < framesize; i++) {
        double result = 0;
        for(int ch=0; ch < 4; ch++) {
            int addr = 16*ch;
            double f1 = ((double)reg.at(addr+0).toInt()*256+reg.at(addr+1).toInt());
            t1[ch] = f1/SAMPLE_FREQ;
            double v1 = ((double)reg.at(addr+2).toInt())/255;
            t2[ch] += ((double)(f1*reg.at(addr+3).toInt()))/16/SAMPLE_FREQ;
            double v2 = ((double)reg.at(addr+4).toInt())/64;
            t3[ch] += ((double)(f1*reg.at(addr+5).toInt()))/16/SAMPLE_FREQ;
            double v3 = ((double)reg.at(addr+6).toInt())/64;
            t4[ch] += ((double)(f1*reg.at(addr+7).toInt()))/16/SAMPLE_FREQ;
            double v4 = ((double)reg.at(addr+8).toInt())/64;
            result /= 2;
            result += generateFMWave(t1[ch],v1,t2[ch],v2,t3[ch],v3,t4[ch],v4);
        }
        
        frames[i] = (Sint16)result;
        Total_time++;
    }
    
}

void initSound() {
    int count = SDL_GetNumAudioDevices(0);
    want.freq = SAMPLE_FREQ;
    want.format = AUDIO_S16;
    want.channels = 1;
    want.samples = SOUND_CHUNK;
    want.callback = AudioCallBack;

    dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);

    SDL_PauseAudioDevice(dev, 0);
}
