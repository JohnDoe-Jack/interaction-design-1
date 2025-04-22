#include "slot.hpp"


// 初期化
void Slot::begin() {
  soundRef.begin();
  soundRef.play(BEGIN_SOUND_FILE);  // Sound クラスの play() を使用
  delay(10000);
  imageRef.setup();
  imageRef.drawStartFrame();
  imuRef.initIMU();
  initSlots();
}

// 毎フレーム呼ばれる更新処理
void Slot::update() {
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
          syms[2] = random(0, WIN_VAL) > SYMBOL_COUNT ? syms[0] : random(0, SYMBOL_COUNT);
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
          syms[1] = random(0, WIN_VAL) > SYMBOL_COUNT ? syms[0] : random(0, SYMBOL_COUNT);
          imageRef.drawRandomFrame(syms[0], syms[1], syms[2]);
          count = 3;
        }
        soundRef.play(BOSN_SOUND_FILE);  // Sound クラスの play() を使用
        delay(50);
      }

      M5.delay(STOP_DELAY_MS);  // 同一動作の誤検出防止
    }
  }

  delay(UPDATE_DELAY_MS);
}

// スロットを初期化（当選かどうかを決め、停止時に使う最終配置を作成）
void Slot::initSlots() {
  imageRef.drawSpinningFrame();
  // 配列を初期化
  for (int i = 0; i < 3; ++i) {
    syms[i] = -1;
  }
}

// 当たり判定(一例: 横/縦/斜めが揃えばtrue)
void Slot::checkWin() {
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
