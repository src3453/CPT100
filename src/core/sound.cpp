#include <math.h>

SDL_AudioSpec Desired;
SDL_AudioSpec Obtained;
double double Total_time = 0;

float sind(float theta) {
    return sin(theta*2*M_PI);
}

double generateFMWave(double t, int addr)
{
    float f1 = ((float)ram_peek(ram, addr+0)*256+ram_peek(ram, addr+1).toInt());
    float v1 = ((float)ram_peek(ram, addr+2).toInt())/255;
    float f2 = ((float)ram_peek(ram, addr+3).toInt())/16;
    float v2 = ((float)ram_peek(ram, addr+4).toInt())/255;
    float f3 = ((float)ram_peek(ram, addr+5).toInt())/16;
    float v3 = ((float)ram_peek(ram, addr+6).toInt())/255;
    float f4 = ((float)ram_peek(ram, addr+7).toInt())/16;
    float v4 = ((float)ram_peek(ram, addr+8).toInt())/255;
    double value = sind(t*f1+sind(t*f1*f2+sind(t*f1*f3+sind(t*f1*f4)*v4)*v3)*v2)*v1*127+127;
    return value;
}

void callback(void *unused, Uint8 *stream, int len)
{
    int i;
    Uint16 *frames = (Uint16 *) stream;
    int framesize = len / 2;
    for (i = Total_time; i < framesize + Total_time; i++, Total_time++) {
        frames[i]= generateFMWave(Total_time / Obtained.freq,0x10000);
    }
}

void playSound() {
    Desired.freq= 48000; /* Sampling rate: 48000Hz */
    Desired.format= AUDIO_S16LSB; /* 16-bit signed audio */
    Desired.channels= 0; /* Mono */
    Desired.samples= 512; /* Buffer size: 512 */
    Desired.callback= callback;
    Desired.userdata= NULL;

    SDL_OpenAudio(&Desired, &Obtained);
    SDL_PauseAudio(0);
}