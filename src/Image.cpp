#include "Image.hpp"

const int Image::colsX[3] = {52, 126, 199};
const int Image::slotY = 65;

// 初期化関数
void Image::setup() {
  // SDカードとM5初期化
  auto cfg = M5.config();
  cfg.serial_baudrate = 115200;
  M5.begin(cfg);
  delay(500);

  while (!SD.begin(GPIO_NUM_4, SPI, 15000000)) {
    Serial.println("SD Wait...");
    delay(500);
  }

  // 背景描画
  bgSprite.createSprite(screenW, screenH);
  bgSprite.drawPngFile(SD, UI_START_base, 0, 0);

  for (int i = 0; i < N_SLOTS; i++) {
    slotPaths[i] = String("/slot_center_") + i + ".png";
    slotSprite[i].createSprite(slotW, slotH);
    slotSprite[i].drawPngFile(SD, slotPaths[i].c_str(), 0, 0);
  }

  frameSprite.createSprite(screenW, screenH);
  randomSeed(millis());
}

// 背景を描画する関数
void Image::setBackground(const char* path) {
  bgSprite.drawPngFile(SD, path, 0, 0);
}

// StartFrameを描画する関数
void Image::drawRandomFrame(int idx0, int idx1, int idx2) {
  int indices[3] = {idx0, idx1, idx2};

  // 背景描画（初期化）
  frameSprite.pushImage(0, 0, screenW, screenH, (uint16_t*)bgSprite.getBuffer());

  for (int i = 0; i < 3; i++) {
    int idx = (indices[i] == -1) ? random(0, N_SLOTS) : indices[i];

    // 範囲チェック（安全のため）
    if (idx < 0 || idx >= N_SLOTS) {
      idx = 0;  // fallback
    }

    frameSprite.pushImage(
        colsX[i], slotY, slotW, slotH, (uint16_t*)slotSprite[idx].getBuffer());
  }

  // 合成結果を画面に描画
  M5.Display.pushImage(0, 0, screenW, screenH, (uint16_t*)frameSprite.getBuffer());
}

// StartFrameを描画する関数
void Image::drawStartFrame() {
  // スタート画面を描画
  setBackground(UI_START_base);
  // 背景を描画
  frameSprite.pushImage(0, 0, screenW, screenH, (uint16_t*)bgSprite.getBuffer());
  // 画面へ転送
  M5.Display.pushImage(0, 0, screenW, screenH, (uint16_t*)frameSprite.getBuffer());
}
