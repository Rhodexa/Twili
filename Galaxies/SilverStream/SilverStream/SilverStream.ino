#include "ledchan.h"

#include <WiFi.h>
#include <WiFiUdp.h>

const char *ssid = "Gilda";
const char *password = "gabbygriff";

WiFiUDP udp;
const int port = 12345;

byte incoming_frame[16];
int brightness = 0;
int down = 0;
void setup() {
  Serial.begin(115200);
  pinMode(2, OUTPUT);

  ledchan_begin();

  digitalWrite(2, HIGH);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    /*delay(500);
    Serial.print(".");*/
    if(down){
      brightness--;
      if(brightness == 0) down = 0;
    }
    else {
      brightness++;
      if(brightness == 127) down = 1;
    }
    framebuffer[3] = brightness;
    ledchan_update();
    delay(41);
  }
  Serial.println("Connected to ESP8266 AP!");
  digitalWrite(2, LOW);


  udp.begin(port);
}


void loop() {
  /*int packet_size = udp.parsePacket();
  if (packet_size) {
    udp.read(incoming_frame, sizeof(incoming_frame));
    for(int i = 0; i < 16; i++)
    {
      framebuffer[i] = incoming_frame[i];
    }
  }*/
}