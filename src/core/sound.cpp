SDL_AudioSpec Desired;
SDL_AudioSpec Obtained;

double generateFMWave(double t)
{
    float f1 = (float)ram_peek(ram, 0x10000)*256+ram_peek(ram, 0x10001).toInt();
    float v1 = ((float)ram_peek(ram, 0x10002).toInt())/255;
    float f2 = ((float)ram_peek(ram, 0x10003).toInt())/16;
    float v2 = ((float)ram_peek(ram, 0x10004).toInt())/255;
    float f3 = ((float)ram_peek(ram, 0x10005).toInt())/16;
    float v3 = ((float)ram_peek(ram, 0x10006).toInt())/255;
    float f4 = ((float)ram_peek(ram, 0x10007).toInt())/16;
    float v4 = ((float)ram_peek(ram, 0x10008).toInt())/255;
}

void callback(void *unused, Uint8 *stream, int len)
{
    int i;
    static unsigned int step= 0;
    Uint16 *frames = (Uint16 *) stream;
    int framesize = len / 2;
    for (i = 0; i < framesize; i++, step++) {
        frames[i]= generateFMWave(step * Frequency / Obtained.freq) * 3000;
    }
}

void playSound() {
    Desired.freq= 22050; /* Sampling rate: 22050Hz */
    Desired.format= AUDIO_S16LSB; /* 16-bit signed audio */
    Desired.channels= 0; /* Mono */
    Desired.samples= 8192; /* Buffer size: 8K = 0.37 sec. */
    Desired.callback= callback;
    Desired.userdata= NULL;

    SDL_OpenAudio(&Desired, &Obtained);
    SDL_PauseAudio(0);
}