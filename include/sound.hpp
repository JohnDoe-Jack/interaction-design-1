#pragma once
#include <Arduino.h>
#include <M5Unified.h>
#include "AudioFileSourceSD.h"
#include "AudioGeneratorWAV.h"
#include "AudioOutputI2S.h"

// #define TFCARD_CS_PIN 4 // SDカードのCSピン  // main.cpp に移動 (一箇所で定義)
#define OUTPUT_GAIN 50

// サウンドファイル名の定数定義
#define LOOP_SOUND_FILE "/slotSound.wav"   // loop音
#define BEGIN_SOUND_FILE "/beginSound.wav"  // スタート音
#define BOSN_SOUND_FILE "/BossonSound.wav"  // 回転を止める時の音
#define WIN_SOUND_FILE "/winSound.wav"    // 勝利音
#define LOSE_SOUND_FILE "/loseSound.wav"   // 外れ音

class Sound {
public:
    void begin(); // 初期化
    void play(const char *filename); // 再生
    void stop();  // 明示的に停止
    void loop();  // ループ再生
    bool isStoppedExplicitly = false;

private:
    AudioGeneratorWAV *wav = nullptr;
    AudioFileSourceSD *file = nullptr;
    AudioOutputI2S *out = nullptr;
};