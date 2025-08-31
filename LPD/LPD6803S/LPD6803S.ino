#include <SPI.h>

const int LED_SCK = 18;
const int LED_MOSI = 23;

void send16(uint16_t data)
{
  SPI.transfer(data >> 8);
  SPI.transfer(data & 0xFF);
}

void setup() {
  SPI.begin(LED_SCK, -1, LED_MOSI, -1);
  SPI.beginTransaction(SPISettings(20000, MSBFIRST, SPI_MODE0));
}


void frame(){
  send16(0);
  send16(0);
}

void color(int r, int g, int b){
  r = constrain(r, 0, 255) >> 3;
  g = constrain(g, 0, 255) >> 3;
  b = constrain(b, 0, 255) >> 3;
  uint16_t packet = r;
  packet = packet << 5; packet |= g;
  packet = packet << 5; packet |= b;
  packet |= 0x8000;
  send16(packet);
}

uint16_t m;
void loop() {
  for(int i = 0; i < 255; i++){
    color(i*2, i, i*.5);
    frame();
    delay(2);
  }
  for(int i = 0; i < 255; i++){
    color((255-i)*2, (255-i), (255-i)*0.5);
    frame();
    delay(2);
  }
}
