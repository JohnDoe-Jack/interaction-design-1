#include "SD.h"
#include <M5Unified.h>

const char* Path_Display = "/display.png";
const char* Path_Spinning = "/SPINNING.png";
const int colsX[] = {50, 123, 196};

const int scrollSpeed = 2;  // フレームあたりの移動ピクセル数
const int screenW = 320;
const int screenH = 240;

// 各種キャンバス
M5Canvas displaySprite(&M5.Display);
M5Canvas uiSprite(&M5.Display);
// 全画面合成用
M5Canvas frameSprite(&M5.Display);

// 各列の Y 位置
int yPos[3];

void setup() {
  auto cfg = M5.config();
  cfg.serial_baudrate = 115200;
  M5.begin(cfg);
  delay(500);

  // SD マウント
  while (!SD.begin(GPIO_NUM_4, SPI, 15000000)) {
    Serial.println("SD Wait...");
    delay(500);
  }

  // displaySprite の準備
  uint16_t imgW = 73, imgH = 504;  // 実際の画像サイズに合わせて
  displaySprite.createSprite(imgW, imgH);
  displaySprite.drawPngFile(SD, Path_Display, 0, 0);

  // uiSprite の準備
  uiSprite.createSprite(screenW, screenH);
  uiSprite.drawPngFile(SD, Path_Spinning, 0, 0);

  // frameSprite の準備（画面サイズ）
  frameSprite.createSprite(screenW, screenH);

  // 各列の初期 Y 設定
  for (int i = 0; i < 3; i++) {
    yPos[i] = -(imgH / 9) * i;
  }
}

void loop() {
  // 1) frameSprite を黒でクリア
  frameSprite.fillRect(0, 0, screenW, screenH, TFT_BLACK);

  // 2) 各列の displaySprite を合成
  uint16_t imgW = displaySprite.width();
  uint16_t imgH = displaySprite.height();
  uint16_t* buf16 = (uint16_t*)displaySprite.getBuffer();  // RGB565 buffer

  for (int i = 0; i < 3; i++) {
    int x = colsX[i];
    int y = yPos[i];

    // 上下２枚合成
    frameSprite.pushImage(x, y, imgW, imgH, buf16);
    frameSprite.pushImage(x, y + imgH, imgW, imgH, buf16);

    // Y 更新＆ループ戻し
    yPos[i] += scrollSpeed;
    if (yPos[i] >= imgH) {
      yPos[i] -= imgH;
    }
  }

  // 3) UI を合成（透明度無視でそのまま）
  uint16_t* uiBuf = (uint16_t*)uiSprite.getBuffer();
  frameSprite.pushImage(0, 0, screenW, screenH, uiBuf);

  // 4) 画面に一度だけ転送
  uint16_t* frameBuf = (uint16_t*)frameSprite.getBuffer();
  M5.Display.pushImage(0, 0, screenW, screenH, frameBuf);

  delay(16);  // 約60FPS
}
