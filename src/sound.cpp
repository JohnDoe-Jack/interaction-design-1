#include "sound.hpp"

AudioGeneratorWAV *wav = nullptr;
AudioFileSourceSD *file = nullptr;
AudioOutputI2S *out = nullptr;

// ★ 追加: 明示的に停止状態かを示すフラグ
bool isStoppedExplicitly = false;

// Sound クラスの初期化関数
void Sound::begin() {
    // M5Unified のスピーカー機能を初期化
    M5.Speaker.begin();
    // スピーカーの音量を設定 (0~255)
    M5.Speaker.setVolume(128);

    // I2S (Inter-IC Sound) 出力を初期化
    // i2s_port=0 を使用し、内蔵DACは使用しない (useDAC=0)
    out = new AudioOutputI2S(0, 0);
    // M5Core2 のスピーカーに接続されているピンを指定
    // BCK=12, WS=0 (未使用), DATA=2
    out->SetPinout(12, 0, 2);
    // モノラル出力に設定
    out->SetOutputModeMono(true);
    // 出力ゲインを設定 (OUTPUT_GAIN は sound.hpp で定義されている)
    out->SetGain((float)OUTPUT_GAIN / 100.0);

    // WAV ファイル生成器を初期化
    wav = new AudioGeneratorWAV();

    // 初期化時はサウンドを再生する意図なので、明示的な停止フラグを解除
    isStoppedExplicitly = false;

    // ★ 修正: begin() の中で直接ループサウンドを開始するために play() を呼び出す
    // play() 関数の中でファイルオープンと AudioGeneratorWAV::begin() が行われる
    play(LOOP_SOUND_FILE);

    // ファイルが開けなかった場合のエラー処理は play() の中で行われる
}

// 指定されたWAVファイルを再生する関数
void Sound::play(const char *filename) {
    // 現在再生中のサウンドがあれば停止する
    if (wav && wav->isRunning()) { // wav オブジェクトが有効かつ再生中か確認
        wav->stop();
    }
    // 前に開いていたファイルオブジェクトをクリーンアップ（メモリ解放）
    if (file) { // file オブジェクトが有効か確認
        delete file;
        file = nullptr; // nullptr に設定して無効化
    }

    // 指定されたファイル名でSDカード上のファイルを開く
    file = new AudioFileSourceSD(filename);
    // ファイルが正常に開けたか確認
    if (file->isOpen()) {
        // wav と out オブジェクトが有効か確認（初期化済みか）
        if (wav && out) {
            // 開いたファイルを AudioGeneratorWAV に渡して再生を開始
            wav->begin(file, out);
            // ★ 追加: play() が呼ばれたら、明示的な停止状態ではない
            isStoppedExplicitly = false; // play が呼ばれたら停止フラグを解除
        } else {
            // wav または out の初期化に問題がある場合のエラーメッセージ
            Serial.println("Error: Sound system components not initialized in Sound::play");
            M5.Display.println("Error: Sound system components not initialized in Sound::play");
            // 初期化エラーの場合、開いたファイルオブジェクトも解放
            if (file) { delete file; file = nullptr; }
        }
    } else {
        // ファイルが開けなかった場合のエラーメッセージ
        Serial.printf("Failed to open %s\n", filename);
        M5.Display.printf("Failed to open %s\n", filename);
        // ファイルが開けなかった場合、file は nullptr のまま。
        // delay(2000); // 前回残っていたが、音声処理をブロックするので削除が望ましい
    }
}

// サウンド再生を明示的に停止する関数
void Sound::stop() {
    // 現在再生中のサウンドがあれば停止する
    if (wav && wav->isRunning()) { // wav オブジェクトが有効かつ再生中か確認
        wav->stop();
    }
    // 開いているファイルオブジェクトをクリーンアップ（メモリ解放）
    if (file) { // file オブジェクトが有効か確認
        delete file;
        file = nullptr; // nullptr に設定して無効化
    }
    // ★ 追加: 明示的に停止したことを示すフラグを立てる
    isStoppedExplicitly = true;
    Serial.println("Sound explicitly stopped.");
}

void Sound::loop() {
    if (wav && !isStoppedExplicitly) { // ★ フラグによる制御を追加
        // AudioGeneratorWAV::loop() を呼び出し、音声データの読み込みと出力処理を進める
        // 再生中は true を返し、サウンドの最後まで達したら false を返す
        if (wav->loop()) {
            // サウンドは正常に再生中
        } else {
            // サウンドが終了した (wav->loop() が false を返した)
            Serial.println("Current sound finished (via loop() returning false).");
            // 終了したサウンドのファイルオブジェクトを解放
            // WAV 生成器も停止 (念のため)
            wav->stop();
            if (file) { // ファイルオブジェクトを解放
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
                // この場合、ファイルは nullptr のまま。明示的な停止ではないが音は止まる。
            }
        }
    }
    // wav が存在しない場合（初期化エラーなど）や、isStoppedExplicitly が true の場合は、
    // この if ブロックの条件を満たさないため、何もしない。
    // これにより、stop() が呼ばれると isStoppedExplicitly が true になり、
    // 以後 loop() は呼ばれても何も処理せず、完全に無音状態を維持できる。
    else if (!wav) {
         // この状態は通常発生しないはず。初期化失敗が考えられる。
         Serial.println("Error: WAV generator is not initialized in Sound::loop (Critical).");
    }
    // else if (isStoppedExplicitly) の場合は、意図的に何もしないため、処理はスキップされる。
}