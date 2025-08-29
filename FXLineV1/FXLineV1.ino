#include <Adafruit_NeoPixel.h>

#include "anim_doomfire.h"

DoomFire* doomFire;

#define PIN            5
#define NUM_LEDS       150
Adafruit_NeoPixel strip(NUM_LEDS, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  strip.begin();
  strip.show();
  strip.setBrightness(255);
  doomFire = new DoomFire(&strip, NUM_LEDS);
  doomFire -> begin();
}

int animation = 0;

uint32_t light = 0;
float counter = 0;

uint32_t getLight(){
  counter += 1;
  return strip.Color(
    constrain( (sin(counter/30)*7-6)   *255 , 0, 255),
    constrain( (sin(counter/30)*8-7)   *128 , 0, 255),
    constrain( (sin(counter/30)*12-11) *50  , 0, 255)
  );
}

uint32_t fire(float intensity){
  return strip.Color(
    constrain( intensity *255, 0, 255),
    constrain( intensity *128, 0, 255),
    constrain( intensity *50, 0, 255)
  );
}

float buffer[NUM_LEDS];

void loop() {
  /*buffer[0] = getLight();
  for(int i = NUM_LEDS-1; i > 0; i--){
    buffer[i] = buffer[i-1];
  }

  */
  run();
  for(int i = 0; i < NUM_LEDS; i++){
    strip.setPixelColor(i, fire(buffer[i]));
  }
  strip.show();
  delay(15);
  //doomFire->run();
}

const int FIRE_HEIGHT = 150;  // The buffer size
float l = 0;
void run() {
  for(int i = FIRE_HEIGHT-1; i >= 0; i--) {
    buffer[i] = buffer[i-1] * 0.99;
  }
  buffer[0] = 1.0f/((sin(l)+1.1)*127);
  l += (rand()%5)/10;
}



