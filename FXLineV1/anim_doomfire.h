#ifndef ANIM_DOOMFIRE

#include "animation.h"
#include <Adafruit_NeoPixel.h>

class DoomFire : public Animation {
  private:
    Adafruit_NeoPixel* strip;
    int numLeds;
    uint8_t *firePixels;

    uint32_t heatColor(uint8_t temp) {
      uint8_t t = temp;
      uint8_t r, g, b;
      if (t > 200) {
        r = 255;
        g = 255;
        b = t - 200;
      } else if (t > 150) {
        r = 255;
        g = t - 150;
        b = 0;
      } else if (t > 100) {
        r = t - 100;
        g = 0;
        b = 0;
      } else {
        r = 0;
        g = 0;
        b = 0;
      }
      return strip->Color(r, g, b);
    }

  public:
    DoomFire(Adafruit_NeoPixel* s, int n) {
      strip = s;
      numLeds = n;
      firePixels = new uint8_t[numLeds];
    }

    void begin() {
      for (int i = 0; i < numLeds; i++) firePixels[i] = 0;
    }

    void run() {
      // Step 1: Cool down every cell a little
      for (int i = 0; i < numLeds; i++) {
          int cooldown = random(0, 3);
          if (firePixels[i] > cooldown) firePixels[i] -= cooldown;
      }

      // Step 2: Heat from the bottom
      firePixels[numLeds - 1] = random(160, 255);

      // Step 3: Propagate heat upward
      for (int i = numLeds - 2; i >= 0; i--) {
          int below = firePixels[i + 1];
          int belowLeft = firePixels[min(numLeds - 1, i + 2)]; // optional small randomization
          firePixels[i] = (below + belowLeft + random(0, 2)) / 2; // add tiny flicker
      }

      // Step 4: Map heat to LED colors
      for (int i = 0; i < numLeds; i++) {
          strip->setPixelColor(i, heatColor(firePixels[i]));
      }

      strip->show();
  }
};


#endif