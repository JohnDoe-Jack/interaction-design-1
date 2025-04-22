//// filepath:
////home/iwasakim/Documents/PlatformIO/Projects/interaction-design-1/src/main.cpp
#include "imu.hpp"
#include "lgfx/v1/platforms/esp32/common.hpp"
#include "slot_machine.hpp"
#include <M5Unified.h>
#include <cstdlib>

static IMU myIMU;
static SlotMachine slot(myIMU);

void setup() { slot.begin(); }

void loop() {
  srand((unsigned)millis());

  slot.update();
}
