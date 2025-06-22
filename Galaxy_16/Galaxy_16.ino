/*
  Twili Classic:
  |-- CPU only cares about this --| |---- Other CPUs' business  ----|
  **************** **************** **************** **************** 
  ssstdddddddddddd ddddccce-------- ssstdddddddddddd ddddccce--------

  A Twili packet consists of 24 bytes carrying 16 bytes worth of payload
  s: Start bytes
  t: Target Index
  d: Data byte
  c: Checksum
  e: End byte
  -: Optional pause time
*/

#include "ledchan.h"

// This device's target index (Which packet index are we listening for)
// Maybe this can be set up using the cutting-edge technology of DIP switches or Jumpers.
int device_target_index = 0;

void setup()
{
  Serial.begin(19200);
  ledchan_begin();
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
uint8_t serial_buffer_head = 0; // Loops at 255. This makes the most crude circular buffer
uint8_t serial_buffer_head_temp = 0; // used to revert back if an error occurred (kind of like a cancel)
bool data_error = true;

void loop()
{
  while(Serial.available())
  {
    char c = Serial.read();
    serial_buffer[serial_buffer_head++] = c;
    if(c == '\n')
    {
      serial_buffer_head_temp = serial_buffer_head; // store current localtion in the buffer
      serial_buffer_head -= PACKET_LENGTH; // Revert cursor to data start
      parseData();
      if (data_error)
      { 
        // Packet wasn't for us or was broken.
        // Undo cursor positioning and keep listening.
        serial_buffer_head = serial_buffer_head_temp;        
      }
    }
  }
}

int data_target = 0;
int data_framebuffer[16];
void parseData()
{
  data_error = true;
  if(serial_buffer[serial_buffer_head++] != '<') return;
  if(serial_buffer[serial_buffer_head++] != '=') return;
  if(serial_buffer[serial_buffer_head++] != '>') return;
  data_target = from64(serial_buffer[serial_buffer_head++]);
  if(from64_error) return;

  int data_sum = 0;
  for(int i = 0; i < 16; i++)
  {
    int data = from64(serial_buffer[serial_buffer_head++]);
    data_sum += data;
    data_framebuffer[i] = data;
  }

  int             checksum  = from64(serial_buffer[serial_buffer_head++]); if(from64_error) return;
  checksum *= 64; checksum += from64(serial_buffer[serial_buffer_head++]); if(from64_error) return;
  checksum *= 64; checksum += from64(serial_buffer[serial_buffer_head++]); if(from64_error) return;

  if(data_sum != checksum) return;
  if(serial_buffer[serial_buffer_head++] != '\n') return;

  /* Woohoo! Everything passed the tests! */
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