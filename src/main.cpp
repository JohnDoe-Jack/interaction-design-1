#include "SD.h"
#include <M5Unified.h>

const int maxFile = 100;
String fileList[maxFile];
int fileCount = 0;

void setup() {
  auto cfg = M5.config();
  cfg.serial_baudrate = 115200;
  M5.begin(cfg);
  delay(500);

  // SDカードマウント待ち
  while (false == SD.begin(GPIO_NUM_4, SPI, 15000000)) {
    Serial.println("SD Wait...");
    delay(500);
  }

  File root = SD.open("/");
  if (root) {
    File file = root.openNextFile();
    while (file) {
      if (file.isDirectory()) {
        // Dir skip
      } else {
        // File
        String filename = String("/") + file.name();
        Serial.println(filename.indexOf(".jpg"));
        if (filename.indexOf(".jpg") != -1 || filename.indexOf(".png") != -1) {
          // Find
          fileList[fileCount] = filename;
          fileCount++;
          if (maxFile <= fileCount) {
            break;
          }
        }
      }
      file = root.openNextFile();
    }
  }

  Serial.println("File List");
  for (int i = 0; i < fileCount; i++) {
    Serial.println(fileList[i]);
  }
}

void loop() {
  int index = random(0, fileCount);
  String filename = fileList[index];
  int x = random(0, M5.Lcd.width() * 3 / 4);
  int y = random(0, M5.Lcd.height() * 3 / 4);

  Serial.printf("Index=%d, File=%s, %d, %d\n", index, filename.c_str(), x, y);
  if (filename.indexOf(".jpg") != -1) {
    M5.Lcd.drawJpgFile(SD, filename, x, y);
  } else {
    M5.Lcd.drawPngFile(SD, filename, x, y);
  }

  delay(1000);
}