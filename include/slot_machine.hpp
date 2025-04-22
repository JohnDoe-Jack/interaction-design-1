#pragma once
#include "imu.hpp"
#include "sound.hpp"  // Sound クラスを使うのでインクルード
#include "Image.hpp"
#include <M5Unified.h>
#include <cmath>

class SlotMachine {
 private:
  // --- スロット構成 ---
  static constexpr int SYMBOL_COUNT = 9;
  static constexpr float WIN_RATE = 20;  // 大体の当選確率（0 < n < 100）
  static constexpr int WIN_VAL = static_cast<int>(9.0f / (-std::sqrt(WIN_RATE / 100) + 1.0f));

  // --- 画面・画像 ---
  static constexpr int screenW = 320;
  static constexpr int screenH = 240;
  static constexpr int slotW = 70;
  static constexpr int slotH = 106;
  static constexpr int colsX[3] = {52, 126, 199};
  static constexpr int slotY = 65;
  static constexpr const char *BG_PATH = "/START_base.png";

  static constexpr const char *SLOT_PATHS[SYMBOL_COUNT] = {
      "/slot_center_0.png", "/slot_center_1.png", "/slot_center_2.png",
      "/slot_center_3.png", "/slot_center_4.png", "/slot_center_5.png",
      "/slot_center_6.png", "/slot_center_7.png", "/slot_center_8.png"};

  // M5Canvas
  M5Canvas bgSprite{&M5.Display};
  M5Canvas frameSprite{&M5.Display};
  M5Canvas slotSprite[SYMBOL_COUNT];

  IMU &imuRef;      // IMUクラス参照
  Sound &soundRef;  // Soundクラス参照
  Image &imageRef;  // Imageクラス参照

  int count = 0;  // スロットのアニメーションカウント

  int syms[3] = {-1, -1, -1};

  // 定数定義 (マジックナンバー、マジックストリングの解消)
  static constexpr float ACCEL_THRESHOLD = 2.0f;
  static constexpr int STOP_DELAY_MS = 500;
  static constexpr int UPDATE_DELAY_MS = 30;

 public:
  // 当選率を指定可能なコンストラクタ
  explicit SlotMachine(IMU &imu, Sound &sound, Image &image)  // Sound クラスへの参照を追加
      : imuRef(imu), soundRef(sound), imageRef(image) {       // コンストラクタで Sound 参照を初期化
  }

  // 初期化
  void begin() {
    soundRef.begin();
    soundRef.play(BEGIN_SOUND_FILE);  // Sound クラスの play() を使用
    delay(10000);
    imageRef.setup();
    imageRef.drawStartFrame();
    imuRef.initIMU();
    initSlots();
  }

  // 毎フレーム呼ばれる更新処理
  void update() {
    M5.update();

    // 停止していない列はランダム色でアニメーション
    imageRef.drawRandomFrame(syms[0], syms[1], syms[2]);
    soundRef.loop();

    // 加速度検出でスロット停止 or リセット
    if (imuRef.updateIMU()) {
      auto data = imuRef.getIMUData();
      float ax = fabsf(data.accel.x);
      float ay = fabsf(data.accel.y);
      float az = fabsf(data.accel.z);

      // しきい値以上の加速度を検出
      if ((ax > ACCEL_THRESHOLD) || (ay > ACCEL_THRESHOLD) || (az > ACCEL_THRESHOLD)) {
        if (count > 3) {
          soundRef.play(BEGIN_SOUND_FILE);  // Sound クラスの play() を使用
          initSlots();
          count = 0;
        } else if (count == 3) {
          // すでに全列停止後なら再度初期化
          if (syms[0] == syms[1]) {
            syms[2] = random(0, WIN_VAL) > 9 ? syms[0] : random(0, 9);
          } else {
            syms[2] = random(0, 9);
          }

          imageRef.drawRandomFrame(syms[0], syms[1], syms[2]);

          checkWin();

          count++;

        } else {
          // 次の列を停止
          count++;
          if (count == 1) {
            syms[0] = random(0, 9);
            imageRef.drawRandomFrame(syms[0], syms[1], syms[2]);
          } else if (count == 2) {
            syms[1] = random(0, WIN_VAL) > 9 ? syms[0] : random(0, 9);
            imageRef.drawRandomFrame(syms[0], syms[1], syms[2]);
            count = 3;
          }
          soundRef.play(BOSN_SOUND_FILE);  // Sound クラスの play() を使用
          delay(50);
        }

        M5.delay(STOP_DELAY_MS);  // 同一動作の誤検出防止
      }
    }

    // M5.delay(UPDATE_DELAY_MS);
    delay(UPDATE_DELAY_MS);
  }

 private:
  // スロットを初期化（当選かどうかを決め、停止時に使う最終配置を作成）
  void initSlots() {
    imageRef.drawSpinningFrame();
    // 配列を初期化
    for (int i = 0; i < 3; ++i) {
      syms[i] = -1;
    }
  }

  // 当たり判定(一例: 横/縦/斜めが揃えばtrue)
  void checkWin() {
    if (syms[0] == syms[1] && syms[1] == syms[2]) {
      soundRef.stop();
      soundRef.play(WIN_SOUND_FILE);  // Sound クラスの play() を使用
      imageRef.drawWinFrame();

    } else {
      soundRef.stop();
      soundRef.play(LOSE_SOUND_FILE);  // Sound クラスの play() を使用
      imageRef.drawLoseFrame();
    }
  }
};
