#include "Image.hpp"

Image slotImage;

void setup() {
  slotImage.setup();
}

void loop() {
  slotImage.drawRandomFrame();
  // slotImage.drawStartFrame();
  delay(100);
}
