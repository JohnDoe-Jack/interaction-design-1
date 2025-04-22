#include "image.hpp"
#include "imu.hpp"
#include "sound.hpp"
// #include "lgfx/v1/platforms/esp32/common.hpp"
#include "slot.hpp"
#include <M5Unified.h>
#include <WiFi.h>
// #include "AudioFileSourceSD.h"
// #include <cstdlib>

#define TFCARD_CS_PIN 4  // SDカードのCSピン

static IMU myIMU;
static Sound sound;
static Image slotImage;
static Slot slot(myIMU, sound, slotImage);

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);

  Serial.begin(115200);
  WiFi.mode(WIFI_OFF);  // Wi-Fiは使わないのでオフにしておく

  if (!SD.begin(TFCARD_CS_PIN)) {
    Serial.println("SD card mount failed!");
    M5.Display.println("SD card mount failed!");
    while (1) delay(1000);
  }

  slot.begin();
}

void loop() {
  slot.update();
}
