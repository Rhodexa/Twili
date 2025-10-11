#include <WiFi.h>
#include <esp_now.h>
#include <FastLED.h>
#include "esp_wifi.h"  // for esp_wifi_set_channel

#define NUM_LEDS 150
#define DATA_PIN 13
CRGB leds[NUM_LEDS];

uint8_t gamma5[32]; // for 5-bit channels
uint8_t gamma6[64]; // for 6-bit green

void initGammaLUT(float gamma = 2.2) {
  for (int i = 0; i < 32; i++) gamma5[i] = pow(i/31.0, gamma) * 255;
  for (int i = 0; i < 64; i++) gamma6[i] = pow(i/63.0, gamma) * 255;
}

void flashBootSequence() {
  // all green
  fill_solid(leds, NUM_LEDS, CRGB::Green);
  FastLED.show();
  delay(500);

  // all red
  fill_solid(leds, NUM_LEDS, CRGB::Red);
  FastLED.show();
  delay(500);

  // all blue
  fill_solid(leds, NUM_LEDS, CRGB::Blue);
  FastLED.show();
  delay(500);

  // clear
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
}

void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *incomingData, int len) {
  int processLen = min(len, 75*2);

  for (int i = 0; i < processLen / 2; i++) {
    uint16_t raw = (incomingData[i*2] << 8) | incomingData[i*2 + 1];
    
    uint8_t r5 = (raw >> 11) & 0x1F;
    uint8_t g6 = (raw >> 5) & 0x3F;
    uint8_t b5 = raw & 0x1F;

    uint8_t r8 = gamma5[r5];
    uint8_t g8 = gamma6[g6];
    uint8_t b8 = gamma5[b5];

    leds[i*2]     = CRGB(r8, g8, b8);
    leds[i*2 + 1] = CRGB(r8, g8, b8);
  }

  FastLED.show();
}

void setup() {
  Serial.begin(115200);

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  // Force both ends to the same channel
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);

  initGammaLUT();
  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);

  flashBootSequence();  // prove LEDs are alive

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    while (true) {
      fill_solid(leds, NUM_LEDS, CRGB::Red);
      FastLED.show();
      delay(250);
      fill_solid(leds, NUM_LEDS, CRGB::Black);
      FastLED.show();
      delay(250);
    }
  }

  esp_now_register_recv_cb(OnDataRecv);
  Serial.println("Receiver ready.");
}

void loop() {
  // nothing needed; ESP-NOW callback drives LEDs
}
