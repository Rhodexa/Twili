#include <WiFi.h>
#include <esp_now.h>

/* Unicast settings */
const int PEERS_COUNT = 2;
uint8_t devices[2][6] = 
{
  {0x38, 0x18, 0x2B, 0x8B, 0x4E, 0x94},
  {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}
};

/* ///////////////////////////////////////////////////// */
uint16_t write_head = 0;
uint16_t read_head = 0;
const int BUFF_BITWIDTH = 8; // 256 bytes
const int BUFF_SIZE = 1 << BUFF_BITWIDTH;
const int BUFF_MASK = (1 << BUFF_BITWIDTH) - 1;

uint8_t buffer[BUFF_SIZE];

uint8_t bufferRead()
{
  read_head = read_head & BUFF_MASK;
  return buffer[read_head];
}

void bufferWrite(uint8_t data)
{
  write_head++;
  write_head = write_head & BUFF_MASK;
  buffer[write_head] = data;
}

/* ///////////////////////////////////////////////////// */
esp_now_peer_info_t peerInfo = {};

void setup()
{
  /* ///////////////////////////////////////////////////// */
    Serial.begin(250000);
    pinMode(LED_BUILTIN, OUTPUT);

  /* ///////////////////////////////////////////////////// */
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();

    while (esp_now_init() != ESP_OK)
    {
      digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
      delay(200);
    }

    for (int i = 0; i < 2; i++)
    {
      memset(&peerInfo, 0, sizeof(esp_now_peer_info_t));
      memcpy(peerInfo.peer_addr, devices[i], 6);      
      peerInfo.channel = 1;
      peerInfo.encrypt = false;
      while (esp_now_add_peer(&peerInfo) != ESP_OK)
      {
        digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
        delay(200);
      }
    }

    for(int i = 0; i < 4; i++)
    {
      digitalWrite(LED_BUILTIN, 0);
      delay(100);
      digitalWrite(LED_BUILTIN, 1);
      delay(100);
    }

  Serial.println("[GILDA] The game is on!");
}

void loop()
{
  while(Serial.available() > 0)
  {
    char data = Serial.read();
    bufferWrite(data);
    if(data == '\n') parse();
  }
}

const int SEND_BUFFER_SIZE = 250;
uint8_t send_buffer[SEND_BUFFER_SIZE];
uint8_t data_length = 0;

uint8_t parse()
{
  digitalWrite(LED_BUILTIN, HIGH); // We should use timers for this...

  // Step 1, sync heads and go to the start of data
  read_head = write_head;                     // sync heads
  read_head--;                                // go back one byte to get packet length (...<length:1>\n)
  data_length = bufferRead();                 // get length

  // check 'length' byte is a valid "number" (0x80 - 0xFF, 0 - 127)
  // if not, abort and throw error 1: "length is not a number"
  if(data_length & 0x80) data_length -= 0x80;
  else return 1;

  // Go back 'length' + 'lengthOf("GL")' to get start of frame
  read_head -= (data_length);
  read_head -= 2;

  // check the string 'GL' is at positions 0 and 1, if not, return error 2 or 3: "invalid frame start signature"
  if(bufferRead() != 'G') return 2;  read_head++;
  if(bufferRead() != 'L') return 3;  read_head++;
  
  // If everything went OK, we should be able to parse frame data... if a channel is corrupted and became an invalid number, we skip it.
  send_buffer[0] = 'G';
  send_buffer[1] = 'I';
  send_buffer[2] = 'L';
  send_buffer[3] = 'D';
  send_buffer[4] = 'A';
  send_buffer[5] = 0x80;
  for(int i = 0; i < data_length; i++){
    char data = bufferRead(); read_head++;

    if(data & 0x80)
    {
      send_buffer[i + 6] = data - 0x80;
    }
  }

  // Got frame data correctly... Time to send!
  if (data_length > 250) data_length = 250;
  if (data_length > 0)
  {
    esp_err_t result = esp_now_send(devices[0], send_buffer, data_length);
    if (result == ESP_OK) digitalWrite(LED_BUILTIN, LOW);
    else digitalWrite(LED_BUILTIN, HIGH);
  }
  return 0;
}





