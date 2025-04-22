#include "sound.hpp"

AudioGeneratorWAV *wav = nullptr;
AudioFileSourceSD *file = nullptr;
AudioOutputI2S *out = nullptr;

bool isStoppedExplicitly = false;

void Sound::begin() {
    M5.Speaker.begin();
    M5.Speaker.setVolume(128);

    out = new AudioOutputI2S(0, 0);
    out->SetPinout(12, 0, 2);
    out->SetOutputModeMono(true);
    out->SetGain((float)OUTPUT_GAIN / 100.0);

    wav = new AudioGeneratorWAV();
    isStoppedExplicitly = false;

    play(LOOP_SOUND_FILE);
}

void Sound::play(const char *filename) {
    if (wav && wav->isRunning()) { 
        wav->stop();
    }

    if (file) { 
        delete file;
        file = nullptr; 
    }

    file = new AudioFileSourceSD(filename);
    if (file->isOpen()) {
        if (wav && out) {
            wav->begin(file, out);
            isStoppedExplicitly = false; 
        } else {
            Serial.println("Error: Sound system components not initialized in Sound::play");
            M5.Display.println("Error: Sound system components not initialized in Sound::play");
            if (file) { delete file; file = nullptr; }
        }
    } else {
        Serial.printf("Failed to open %s\n", filename);
        M5.Display.printf("Failed to open %s\n", filename);
    }
}

void Sound::stop() {
    if (wav && wav->isRunning()) {
        wav->stop();
    }
    if (file) { 
        delete file;
        file = nullptr; 
    }
    isStoppedExplicitly = true;
    Serial.println("Sound explicitly stopped.");
}

void Sound::loop() {
    if (wav && !isStoppedExplicitly) { 
    
        if (wav->loop()) {
            // サウンドは正常に再生中
        } else {
            // サウンドが終了した (wav->loop() が false を返した)
            Serial.println("Current sound finished (via loop() returning false).");
            // 終了したサウンドのファイルオブジェクトを解放
            wav->stop();
            if (file) { 
                delete file;
                file = nullptr;
            }

            // ★ 再生が終了し、かつ明示的に停止されていない場合、ループサウンドを再開する
            Serial.println("Restarting loop sound after playback finished.");
            // ループサウンドファイルを再度開く
            file = new AudioFileSourceSD(LOOP_SOUND_FILE);
            // ファイルが正常に開けたか確認
            if (file->isOpen()) {
                if (out) { // out が有効かチェック
                   // ループサウンドの再生を開始
                   wav->begin(file, out);
                   Serial.printf("Loop sound %s restarted.\n", LOOP_SOUND_FILE);
                } else {
                    // out の初期化に問題がある場合
                    Serial.println("Error: Audio output not initialized for loop restart.");
                    if (file) { delete file; file = nullptr; } // 開いたファイルも解放
                }
            } else {
                // ループサウンドファイルが開けなかった場合のエラーメッセージ
                Serial.printf("Failed to open %s for loop restart\n", LOOP_SOUND_FILE);
                M5.Display.printf("Failed to open %s for loop restart\n", LOOP_SOUND_FILE);
            }
        }
    }
    else if (!wav) {
         Serial.println("Error: WAV generator is not initialized in Sound::loop (Critical).");
    }
}