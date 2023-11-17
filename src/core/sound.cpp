#include <math.h>

#define SAMPLE_FREQ 48000
#define SOUND_CHUNK 1024

#include "ringBuffer.cpp"

SDL_AudioSpec want, have;
SDL_AudioStream *stream = SDL_NewAudioStream(AUDIO_S8, 1, SAMPLE_FREQ, AUDIO_F32, 2, SAMPLE_FREQ);
SDL_AudioDeviceID dev;

long long Total_time = 0;

CRingBuffer samples;

float sind(float theta) {
    return sin(theta*2*M_PI);
}

double generateFMWave(double t, float f1, float v1, float f2, float v2, float f3, float v3, float f4, float v4) {
    
    double value = sind(t*f1+sind(t*f1*f2+sind(t*f1*f3+sind(t*f1*f4)*v4)*v3)*v2)*v1*127;
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
            float f1 = ((float)reg.at(addr+0).toInt()*256+reg.at(addr+1).toInt());
            float v1 = ((float)reg.at(addr+2).toInt())/255;
            float f2 = ((float)reg.at(addr+3).toInt())/16;
            float v2 = ((float)reg.at(addr+4).toInt())/64;
            float f3 = ((float)reg.at(addr+5).toInt())/16;
            float v3 = ((float)reg.at(addr+6).toInt())/64;
            float f4 = ((float)reg.at(addr+7).toInt())/16;
            float v4 = ((float)reg.at(addr+8).toInt())/64;
            result /= 2;
            result += generateFMWave((double)Total_time / have.freq,f1,v1,f2,v2,f3,v3,f4,v4);
        }
        samples.Write((Sint8)result);
        frames[i] = samples.Read();
        samples.Update();
        Total_time++;
    }
    
}

void initSound() {
    int count = SDL_GetNumAudioDevices(0);
    want.freq = SAMPLE_FREQ;
    want.format = AUDIO_S8;
    want.channels = 1;
    want.samples = SOUND_CHUNK;
    want.callback = AudioCallBack;

    dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);

    SDL_PauseAudioDevice(dev, 0);
}
