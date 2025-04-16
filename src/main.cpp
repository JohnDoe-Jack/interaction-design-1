//// filepath:
////home/iwasakim/Documents/PlatformIO/Projects/interaction-design-1/src/main.cpp
#include "imu.hpp"
#include "slot_machine.hpp"
#include <M5Unified.h>

static IMU myIMU;
static SlotMachine slot(myIMU);

void setup() { 
  auto cfg = M5.config();
  cfg.serial_baudrate = 115200;
  M5.begin(cfg);
  delay(1000);
  M5.Speaker.setVolume(255);
  slot.begin(); 
}

void loop() { slot.update(); }