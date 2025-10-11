#include "dimmer.h"
#include <WiFi.h>
#include <esp_now.h>
#include "esp_wifi.h"

// ID 0 and 1 is reserved for general lighting banks 0 and 1 in GILDA v1.0
// For most cases Bank 0 will be enough -> Each ESP32-based light has 15 channels -> 3xRGBCW lights. So a single Bank can control up to 16 light-clusters.
// Or just about 48 Single RGBCW Spot lights!! That is a freaking lot for our use case...
// This means two reserved banks is enough for about 96 lights if they are RGBCW or more if they are only RGB or only CW.
// If you ever need more (which i doubt you will) you can use a non reserved range, like ID 4
// Bank 10+ is often used for PixelLED chains
// Bank 5 would often be motor control, but that's not reserved.
uint8_t GILDA_ID = 0; 

const unsigned long HEART_INTERVAL = 2000;
volatile bool heart = false;
unsigned long heart_last_check = 0;

void nextChannel() {
  const int channels[] = {1, 6, 11};
  static int channel = 0;
  channel++;
  if (channel > 2) channel = 0;
  int new_channel = channels[channel];

  for (int i = 0; i < 15; i++) dimmer_inputs[i] = 0;
  switch(channel){
    case 0: dimmer_inputs[0]  = 2; break;
    case 1: dimmer_inputs[5]  = 2; break;
    case 2: dimmer_inputs[10] = 2; break;
  }
  dimmer_updateBuffer();
  dimmer_immediate();
  delay(250);
  for (int i = 0; i < 15; i++) dimmer_inputs[i] = 0;
  dimmer_updateBuffer();
  dimmer_immediate();

  esp_err_t err = esp_wifi_set_channel(new_channel, WIFI_SECOND_CHAN_NONE);
  if (err != ESP_OK) {
    Serial.print("Failed to set channel: ");
    Serial.println(err);
    return;
  }

  delay(500);
}

void OnDataRecv(const esp_now_recv_info_t *info, const uint8_t *data, int length)
{
  int packet_length = min(length, 6 + 15);
  if(data[0] != 'G') return;
  if(data[1] != 'I') return;
  if(data[2] != 'L') return;
  if(data[3] != 'D') return;
  if(data[4] != 'A') return;
  if(data[5] != (GILDA_ID | 0x80)) return;
  for (int i = 0; i < packet_length; i++) dimmer_inputs[i] = data[i + 6];
  dimmer_updateBuffer();
  heart = true;
}

void setup() {
  dimmer_begin();
  
  Serial.begin(115200);  
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);

  if (esp_now_init() != ESP_OK) {Serial.println("ESP-NOW init failed");}

  esp_now_register_recv_cb(OnDataRecv);
  Serial.println("Receiver ready.");
}

void loop()
{
  dimmer_loop();
  unsigned long now = millis();
  if (now - heart_last_check >= HEART_INTERVAL)
  {
    heart_last_check = now;
    if (!heart)
      nextChannel();
    heart = false;
  }
}




