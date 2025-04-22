#pragma once

#include "imu.hpp"
#include "sound.hpp"  // Sound クラスを使うのでインクルード
#include "image.hpp"
#include <M5Unified.h>
#include <cmath>

class Slot {
 private:
  // --- スロット構成 ---
  static constexpr int SYMBOL_COUNT = 9;
  static constexpr float WIN_RATE = 25;  // 大体の当選確率（0 < n < 100）
  static constexpr int WIN_VAL = static_cast<int>(SYMBOL_COUNT / (-std::sqrt(WIN_RATE / 100) + 1.0f));

  // --- 画面・画像 ---
  static constexpr int screenW = 320;
  static constexpr int screenH = 240;
  static constexpr int slotW = 70;
  static constexpr int slotH = 106;
  static constexpr int colsX[3] = {52, 126, 199};
  static constexpr int slotY = 65;
  static constexpr const char *SLOT_PATHS[SYMBOL_COUNT] = {
      "/slot_center_0.png", "/slot_center_1.png", "/slot_center_2.png",
      "/slot_center_3.png", "/slot_center_4.png", "/slot_center_5.png",
      "/slot_center_6.png", "/slot_center_7.png", "/slot_center_8.png"};

  static constexpr float ACCEL_THRESHOLD = 2.0f;
  static constexpr int STOP_DELAY_MS = 500;
  static constexpr int UPDATE_DELAY_MS = 20;

  int count = 0;
  int syms[3] = {-1, -1, -1};  //-1 はランダム生成の意味

  // M5Canvas
  M5Canvas bgSprite{&M5.Display};
  M5Canvas frameSprite{&M5.Display};
  M5Canvas slotSprite[SYMBOL_COUNT];

  IMU &imuRef;      // IMUクラス参照
  Sound &soundRef;  // Soundクラス参照
  Image &imageRef;  // Imageクラス参照

 public:
  explicit Slot(IMU &imu, Sound &sound, Image &image)
      : imuRef(imu), soundRef(sound), imageRef(image) {
  }

  void begin();   // 初期化
  void update();  // 毎フレーム呼ばれる更新処理

 private:
  void initSlots();
  void checkWin();
};
