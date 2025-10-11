#include <WiFi.h>
#include "esp_wifi.h"     // for esp_wifi_start() and low-level control
#include "esp_system.h"   // for esp_read_mac()

void setup() {
  Serial.begin(115200);
  delay(1000);

  WiFi.mode(WIFI_STA);          // bring up the Wi-Fi stack
  esp_wifi_start();             // ensure the driver is running

  delay(500);                   // tiny wait just to be sure
  
  uint8_t mac[6];
  esp_wifi_get_mac(WIFI_IF_STA, mac); // guaranteed to return the actual STA MAC

  Serial.printf("ESP-NOW / WiFi MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
}

void loop() {}
