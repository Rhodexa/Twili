#include "ledchan.h"

#include <WiFi.h>
#include <WiFiUdp.h>

const char *ssid = "Gilda";
const char *password = "gabbygriff";

WiFiUDP udp;
const int port = 12345;

byte incoming_frame[16];

void setup() {
  Serial.begin(115200);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to ESP8266 AP!");

  udp.begin(port);
}

void loop() {
  int packet_size = udp.parsePacket();
  if (packet_size) {
    udp.read(framebuffer, sizeof(framebuffer));
  }
  delay(10);
}

void loop() {
}