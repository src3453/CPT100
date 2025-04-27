#include "sound.cpp"
#include "SDL.h"
#include "SDL_audio.h"

#define SOUND_CHUNK 512
#define SAMPLE_FREQ 48000

SDL_AudioSpec want, have;
SDL_AudioStream *stream = SDL_NewAudioStream(AUDIO_S16, 1, SAMPLE_FREQ, AUDIO_F32, 2, SAMPLE_FREQ);
SDL_AudioDeviceID dev;
S3HS_sound chip(ram);

void wrapper(void *unused, Uint8 *stream, int len) {
  std::vector<std::vector<std::vector<int16_t>>> result = chip.AudioCallBack(len);
  // Calculate total length in bytes for a stereo int16_t stream
  size_t total_bytes = len * 2; // 2 bytes per sample for int16_t
  // Allocate buffer for stereo int16_t samples
  int16_t* stereo_buffer = new int16_t[len * 2]; // 2 channels

  // Fill the buffer with L/R audio data
  for (int i = 0; i < len; i++) {
    // Left channel from result[0][12][i]
    stereo_buffer[i * 2] = (i < result[0][12].size()) ? result[0][12][i] : 0;
    // Right channel from result[1][12][i]
    stereo_buffer[i * 2 + 1] = (i < result[1][12].size()) ? result[1][12][i] : 0;
  }

  // Copy the buffer to the SDL audio stream
  SDL_memcpy(stream, stereo_buffer, total_bytes);

  // Clean up
  delete[] stereo_buffer;
}

void initSound() {

  chip.initSound();
  int count = SDL_GetNumAudioDevices(0);
  want.freq = SAMPLE_FREQ;
  want.format = AUDIO_S16;
  want.channels = 2;
  want.samples = SOUND_CHUNK;
  want.callback = wrapper;

  dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);

  SDL_PauseAudioDevice(dev, 0);
  
}

void closeSound() {
  SDL_CloseAudioDevice(dev);
  delete &chip;
  SDL_FreeAudioStream(stream);
  SDL_QuitSubSystem(SDL_INIT_AUDIO);
}