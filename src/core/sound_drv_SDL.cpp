#include "sound.cpp"
#include "SDL.h"
#include "SDL_audio.h"

// バッファサイズを増やして安定性を向上
#define SOUND_CHUNK 1024
#define SAMPLE_FREQ 48000

SDL_AudioSpec want, have;
SDL_AudioDeviceID dev;
S3HS_sound chip(ram);

void wrapper(void *unused, Uint8 *stream, int len) {
  // ストリームバッファをクリア（音声のノイズ防止に重要）
  SDL_memset(stream, 0, len);
  
  std::vector<std::vector<std::vector<float>>> result = chip.AudioCallBack(len / 4);
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
  SDL_QuitSubSystem(SDL_INIT_AUDIO);
}