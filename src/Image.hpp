#pragma once
#include "SD.h"
#include <M5Unified.h>

class Image {
 public:
  static const int N_SLOTS = 9;
  static const int slotW = 70;
  static const int slotH = 106;
  static const int screenW = 320;
  static const int screenH = 240;
  static const int colsX[3];
  static const int slotY;

  void setup();
  void drawRandomFrame();
  void drawStartFrame();
  void setBackground(const char* path);

 private:
  const char* UI_START_base = "/START_base.png";
  const char* UI_START = "/START.png";
  const char* UI_YOU_WIN = "/YOU_WIN.png";
  const char* UI_TRY_AGAIN = "/TRY_AGAIN.png";
  const char* UI_SPINNING = "/SPINNING.png";

  String slotPaths[N_SLOTS];

  M5Canvas bgSprite{&M5.Display};
  M5Canvas slotSprite[N_SLOTS];
  M5Canvas frameSprite{&M5.Display};
};
