#include "Image.hpp"

Image slotImage;

void setup() {
  slotImage.setup();
}

void loop() {
  slotImage.drawRandomFrame(-1, -1, 1);
  // slotImage.drawStartFrame();
  delay(100);
}
