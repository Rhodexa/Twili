/*
  Twili Classic:
  |-- CPU only cares about this --| |---- Other CPUs' business  ----|
  **************** **************** **************** **************** 
  ssstdddddddddddd ddddccce-------- ssstdddddddddddd ddddccce--------

  A Twili packet consists of 24 bytes carrying 16 bytes worth of payload
  s: Start bytes  - String '<=>'
  t: Target Index - Index this packet is addressed to. In this galaxy values from 0 to 15 are allowed (256 channels). But this can go all the way up to 63 (1024 channels)
  d: Data byte
  c: Checksum
  e: End byte
  -: Optional pause time
*/

#include "ledchan.h"
#include "jumpers.h"

// This device's target index (Which packet index are we listening for)
// Maybe this can be set up using the cutting-edge technology of DIP switches or Jumpers.
// This can be configured using jumpers inside the box.
int device_target_index = 0;

void setup()
{
  Serial.begin(57600);
  ledchan_begin();
  jumpers_begin();
  delay(1000);
  device_target_index = jumpers_get();
  Serial.print("Booted! ID:");
  Serial.println(device_target_index);
}

int from64_error = 0;
int from64(char c)
{
  if (c >= 'A' && c <= 'Z') return c - 'A';      // A-Z -> 0-25
  if (c >= 'a' && c <= 'z') return c - 'a' + 26; // a-z -> 26-51
  if (c >= '0' && c <= '9') return c - '0' + 52; // 0-9 -> 52-61
  if (c == '+') return c - '+' + 62;
  if (c == '/') return c - '/' + 63;
  from64_error = 1;
  return -1;  // Default to -1 if invalid character
}

const int PACKET_LENGTH = 24;
const int SERIAL_BUFFER_LENGTH = 256; // Don't change this!! Lmao
char serial_buffer[SERIAL_BUFFER_LENGTH];
uint8_t buffer_write_head = 0; // Loops at 255. This makes the most crude circular buffer
uint8_t buffer_read_head = 0;  // Loops at 255. This makes the most crude circular buffer
bool data_error = true;        // used for tracking... doesn't really play a role in functionality

auto curtime = millis();
auto lastime = curtime;
void loop()
{
  while(Serial.available())
  {
    char c = Serial.read();
    serial_buffer[buffer_write_head++] = c;
    if(c == '\n')
    {
      buffer_read_head = buffer_write_head; // store current localtion in the buffer
      buffer_read_head -= PACKET_LENGTH;    // Revert cursor to data start
      parseData();
    }
  }
  curtime = millis();
  if(curtime - lastime >= 5){
    lastime = curtime;
    run();
  }
}

int data_framebuffer[16];
void parseData()
{
  data_error = true;
  if(serial_buffer[buffer_read_head++] != '<') return;
  if(serial_buffer[buffer_read_head++] != '=') return;
  if(serial_buffer[buffer_read_head++] != '>') return;
  if(device_target_index != from64(serial_buffer[buffer_read_head++]))
  {
    data_error = false; // We don't really know... but this packet wasn't ours anyway.
    return;
  } 
  if(from64_error) return; // This should really never fire... I believe.

  int data_sum = 0;
  for(int i = 0; i < 16; i++)
  {
    int data = from64(serial_buffer[buffer_read_head++]);
    data_sum += data;
    data_framebuffer[i] = data;
  }

  int             checksum  = from64(serial_buffer[buffer_read_head++]); if(from64_error) return;
  checksum *= 64; checksum += from64(serial_buffer[buffer_read_head++]); if(from64_error) return;
  checksum *= 64; checksum += from64(serial_buffer[buffer_read_head++]); if(from64_error) return;

  if(data_sum != checksum) return;
  if(serial_buffer[buffer_read_head++] != '\n') return;

  /* Woohoo! Everything passed the tests! */
  data_error = false;

  // Time to update the timers 
  for(int i = 0; i < 16; i++)
  {
    // If there was a rogue character error.. lets ignore it.
    // Update everything else instead
    if(data_framebuffer[i] != -1) 
    {
      framebuffer[i] = data_framebuffer[i];
    }
  }
  ledchan_update();
}