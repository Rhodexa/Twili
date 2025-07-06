#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

// Set up AP details
const char *ssid = "Gilda";
const char *password = "gabbygriff";

// Create UDP object
WiFiUDP udp;
const int port = 12345;  // Port to send to

// Frame with 16 channels (raw bytes from 0-255)
byte frame[16] = {63, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

void setup() {
  Serial.begin(115200);

  // Start ESP8266 as an AP
  WiFi.softAP(ssid, password);
  Serial.println("ESP8266 AP started");

  // Set up UDP
  udp.begin(port);

  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());  // Print the AP IP address
}

void loop() {
  // Send the frame via UDP broadcast to the ESP32s
  IPAddress broadcastIP = WiFi.localIP();  // Get the local IP address of the AP
  broadcastIP[3] = 255;  // Set the last byte to 255 to broadcast

  udp.beginPacket(broadcastIP, port);
  udp.write(frame, sizeof(frame));
  udp.endPacket();

  Serial.println("Frame sent to ESP32s!");
  delay(1000);  // Wait 1 second before sending the next frame
}
