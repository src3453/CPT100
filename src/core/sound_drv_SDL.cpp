#include "sound.cpp"
#include "SDL.h"
#include "SDL_audio.h"
#include "../../../lib/pocketfft/pocketfft_hdronly.h"

// バッファサイズを増やして安定性を向上
#define SOUND_CHUNK 256
#define SAMPLE_FREQ 48000

SDL_AudioSpec want, have;
SDL_AudioDeviceID dev;
S3HS_sound chip(ram);

void wrapper(void *unused, Uint8 *stream, int len) {
  // ストリームバッファをクリア（音声のノイズ防止に重要）
  std::vector<std::vector<std::vector<float>>> result;
  SDL_memset(stream, 0, len);
  {
    MICROPROFILE_SCOPEI("AudioCallbackOutput", "ChipGenerateAudio", 0x00FF00);
    result = chip.AudioCallBack(len / 4);
  }
  // 出力バッファのサンプル数を計算（ステレオint16_t形式）
  size_t samples = len / (2 * sizeof(int16_t));
  
  // ステレオサンプル用バッファの割り当て
  int16_t* stereo_buffer = new int16_t[samples * 2]; // 2 channels
  
  // L/Rチャンネルのデータでバッファを埋める
  for (size_t i = 0; i < samples; i++) {
    // 左チャンネル
    stereo_buffer[i * 2] = (i < result[0][12].size()) ? (int16_t)(result[0][12][i]) : 0;
    // 右チャンネル
    stereo_buffer[i * 2 + 1] = (i < result[1][12].size()) ? (int16_t)(result[1][12][i]) : 0;
  }
  
  // コピーではなく、適切にミキシングする
  SDL_MixAudioFormat(stream, (Uint8*)stereo_buffer, AUDIO_S16, len, SDL_MIX_MAXVOLUME);
  
  // メモリ解放
  delete[] stereo_buffer;
}

static SDL_AudioDeviceID input_dev = 0;
static std::vector<int16_t> input_buffer;
static SDL_mutex* input_mutex = nullptr;

void input_callback(void* userdata, Uint8* stream, int len) {
  {
    MICROPROFILE_SCOPEI("AudioCallbackInput", "AudioInputCapture", 0x0000FF);
    // 入力バッファに新しいサンプルを追加
    if (input_mutex) SDL_LockMutex(input_mutex);
    
    int16_t* samples = (int16_t*)stream;
    int sample_count = len / sizeof(int16_t);
    
    for (int i = 0; i < sample_count; i++) {
      input_buffer.push_back(samples[i]);
    }
    
    if (input_mutex) SDL_UnlockMutex(input_mutex);
  }
}

void initSoundInput(int samples=1024) {
  // Initialize audio input for VJing purpose
  // Lua API: init_sound_input()
  SDL_AudioSpec want, have;
  
  input_mutex = SDL_CreateMutex();
  input_buffer.clear();
  
  want.freq = SAMPLE_FREQ;
  want.format = AUDIO_S16;
  want.channels = 1;
  want.samples = samples;
  want.callback = input_callback;
  want.userdata = nullptr;
  
  input_dev = SDL_OpenAudioDevice(nullptr, 1, &want, &have, 0);
  printf("Requested Audio Input: freq=%d, format=%d, channels=%d, samples=%d\n", want.freq, want.format, want.channels, want.samples);
  printf("Obtained Audio Input: freq=%d, format=%d, channels=%d, samples=%d\n", have.freq, have.format, have.channels, have.samples);
  
  if (input_dev == 0) {
    fprintf(stderr, "SDL audio input error: %s\n", SDL_GetError());
    return;
  }
  
  SDL_PauseAudioDevice(input_dev, 0);
}

std::vector<float> acquireSoundInput() {
  // Acquire PCM sample from the audio device. for VJing purpose
  // Lua API: acquire_sound_input(samples: table)
  std::vector<float> result;
  
  if (input_mutex) SDL_LockMutex(input_mutex);
  
  for (int16_t sample : input_buffer) {
    result.push_back(static_cast<float>(sample)/32768.0f); // Normalize to -1.0 to 1.0
  }
  input_buffer.clear();
  
  if (input_mutex) SDL_UnlockMutex(input_mutex);
  
  //printf("Acquired %zu samples from audio input\n", result.size());
  return result;
}

std::vector<float> acquireSoundInputFFT(int fft_size) {
  // Get FFT result from acquired audio input samples. for VJing purpose
  // Lua API: acquire_sound_input_fft(fft_size: int) -> table
  std::vector<float> fft_result;
  
  if (input_mutex) SDL_LockMutex(input_mutex);
  
  size_t n = input_buffer.size();
  if (n < (size_t)fft_size) {
    if (input_mutex) SDL_UnlockMutex(input_mutex);
    return fft_result; // Not enough data
  }
  
  // Copy data and normalize to [-1.0, 1.0]
  std::vector<float> real_data(fft_size);
  for (int i = 0; i < fft_size; i++) {
    real_data[i] = static_cast<float>(input_buffer[i]) / 32768.0f;
  }
  
  std::vector<std::complex<float>> data(fft_size / 2 + 1);
  
  pocketfft::shape_t shape = { static_cast<size_t>(fft_size) };
  pocketfft::stride_t stride_in = { sizeof(float) };
  pocketfft::stride_t stride_out = { sizeof(std::complex<float>) };
  pocketfft::shape_t axes = { 0 };
  
  pocketfft::r2c(shape, stride_in, stride_out, axes, false, real_data.data(), data.data(), 1.0f);
  
  for (size_t i = 0; i < fft_size / 2 + 1; i++) {
    float magnitude = std::sqrt(data[i].real() * data[i].real() + data[i].imag() * data[i].imag());
    fft_result.push_back(magnitude);
  }
  
  // Clear consumed data（DO NOT clear entire buffer, only consumed part）
  if (fft_size <= (int)input_buffer.size()) {
    input_buffer.erase(input_buffer.begin(), input_buffer.begin() + fft_size);
  }
  
  if (input_mutex) SDL_UnlockMutex(input_mutex);
  //printf("Acquired FFT result with %zu bins\n", fft_result.size());
  return fft_result;
}

void initSound() {
  chip.initSound();
  
  // オーディオデバイスの設定
  want.freq = SAMPLE_FREQ;
  want.format = AUDIO_S16;
  want.channels = 2;
  want.samples = SOUND_CHUNK;
  want.callback = wrapper;
  
  // オーディオデバイスを開く
  dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, SDL_AUDIO_ALLOW_FREQUENCY_CHANGE);
  
  // エラーチェックを追加
  if (dev == 0) {
    fprintf(stderr, "SDL audio error: %s\n", SDL_GetError());
    return;
  }
  
  SDL_PauseAudioDevice(dev, 0);
}

void closeSound() {
  SDL_CloseAudioDevice(dev);
  SDL_CloseAudioDevice(input_dev);
  if (input_mutex) {
    SDL_DestroyMutex(input_mutex);
    input_mutex = nullptr;
  }
  SDL_QuitSubSystem(SDL_INIT_AUDIO);
}