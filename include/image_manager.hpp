#include <M5Unified.h>
#include <SD.h>

class ImageManager {
private:
  static constexpr int MAX_FILES = 100;
  String fileList[MAX_FILES];
  int fileCount = 0;

public:
  // 画像リストを読み込む
  void loadImages() {
    fileCount = 0;
    File root = SD.open("/");
    if (!root) {
      return;
    }

    File file = root.openNextFile();
    while (file) {
      if (!file.isDirectory()) {
        String fName = String("/") + file.name();
        if (fName.indexOf(".jpg") != -1 || fName.indexOf(".png") != -1) {
          fileList[fileCount++] = fName;
          if (fileCount >= MAX_FILES)
            break;
        }
      }
      file = root.openNextFile();
    }
  }

  // 指定ファイルを座標(x,y)に表示
  void drawImage(const String &filename, int x, int y) {
    if (filename.indexOf(".jpg") != -1) {
      M5.Lcd.drawJpgFile(SD, filename.c_str(), x, y);
    } else if (filename.indexOf(".png") != -1) {
      M5.Lcd.drawPngFile(SD, filename.c_str(), x, y);
    }
  }
};