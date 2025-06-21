/*
  This is part of Rhodune!'s Twili Echosystem.
  A Galaxy is a single Star controller, it is supposed to have 16 control channels.

  The Twili Protocol actually allows for more (or less) than 16 channels to be sent in a single packet.
  It is much like DMX. Except it runs on actual UART protocols with a single stop bit, no Break signal and actual error redundancy checks.

  A Galaxy is supposed to handle only 16 channels. However, the reason more than 16 is allowed is because a single Twili bus may drive more than one galaxy
  and each galaxy can be addressed (Again... alike DMX). Twili does not specify a baud rate. But by convention, 38400baud is recommended, although 57600 is the next target.

  The lower the speed, the better, however, in order to send more data (more galaxies, more frames, error checking), higher bandwith is needed
  Since Twili is supposed to be isolated with optocouplers, we shouldn't push baud rates too high.
  Also, Twili is designed to run on standard UTP wire, which has no shielding.

  38.4kbps gives us enough bandwith to send 3.84kB/s. At 30FPS that equates to a continuous stream of max. 128 bytes
  That's 128 channels or 8 galaxies.

  However some galaxies are sacrificed for
  * Error checking
  * Processing time

  Twili v1.0 Uses two galaxies worth of data to complete the acket for a single galaxy:
  Everything is ASCII formatted.
  Twili's numbers are represented in a base64-like format, which uses the following char-to-value mapping:
  'ABCDEFGHIJKLMNOPRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789/+' >>> 0...63

  Each byte is a character (char). Numbers are sent in this format (base64-like). But communication is not limited to these charaters.
  Thus, other, non-base64-like, characters are used for other kinds of data
  
  [ Packet Start ][ Index ][ 16x Data ][ Check Sum ][ Packet End ][ Processing Breather ]
  [       3      ][   1   ][    16    ][     3     ][      2     ][          7          ] 32 bytes total
  [     '<=>'    ][ 0..63 ][ -------- ][    nnn    ][    '<>'    ][         ---         ]

  * Packet Start is literally the string '<=>' (These characters are not part of the numbers charset)
  * Index is a base64-like byte that tells to which address this frame is destined to, since more than one Galaxy can be address on a single frame
  * Data is just a stream of base64-like bytes. Each byte represents a 0 to 63 value (Each byte is a channel)
  * Checksum is the sum of the entirety of the packet's bytes (excluding breather bytes) in base64-like limited to 3 characters. This should be enough of a check sum for 4096-byte-long streams!
  * Packet End: '<>' string
  * Breather: no data is sent.

  During the breather time, no actual data is sent. But since it is a pause that has to be accounted for
  when calculating the ideal baudrate for optimal framerate, it is measured in bytes rather than time.
  Not sending any data is more of a courtesy from the controller though... Twili defines no data should be sent.
  Galaxy Controllers should be able to ignore this wthout impacting performance. Ideally just polling for START signal

  Twi-lite (Essentially Twili-II, still under analysis)

  Twi-lite (now TL) uses a different format that is NOT backwards compatible.
  It implements a variable-length packet. Much like DMX, each Galaxy can be addressed to individual channels now

  [ Start ][  Length  ][ Data ][ Checksum ][  End  ][ Pause ]
  [   3   ][     2    ][  ??  ][     4    ][   1   ][ undef ] 10 to 4106 bytes (Excluding pause... which is optional) Still... the maximum allowed transmission packet is 1024 bytes
  [ '<=>' ][ 0...4095 ][ ---- ][   nnnn   ][  '\n' ][ ----- ]

  An error on any of these should cause the packet to be dropped...
  Although... you may want to maybe use a more statistical approach... like count how many checks pass the test and then see if its more than...say 80%
  is yes, then let the packet in. Up to you.

  Packets have the following checks:
  * '<' matches. Start of frame
  * '=' matches. Double check start of frame
  * '>' matches. Tripple check start of frame.
  * Length contains only Base64 charset 
  * Data contains only Base64 charset
  * Data length equals length (only data)
  * Data sum matches checksum
  * '\n' came after length + 9 bytes

  Maybe this is a bit too much. Since this isn't exactly a place where accuracy is _that_ important.
  And dropped packets or errors can be corrected fast simply due to the volume of data being sent.
*/

/*
  The present implementation will attempt to interpret Twili-II streams up to 16 channels (which is the max PWM channels supported in ESP32-WROOM)
  Speed is 38400Hz at 30FPS that's 128 bytes per frame
  Since a Twili-II packet is 10 + data bytes long
  we are talking up to 118 channels on a single burst
  We could do 16*7 channels (112) leaving 6bytes worth of pause... that's about 1.56ms of quiet time for devices to catch up
  So... automagically, a single Twili packet can control up to 7 galaxies. That's fancy.
  
  Actually, the front lights requiere 30 channels total (15 each). Which means 19200 baud is already enough!
  19200baud means 64-bytes per frame. Subtract 10 for the format overhead and we get 54 channels!
  Subtract 30 for us and we get 24 left-over channels that can be totally used for whatever else on the string or just be a pause.

  19200Hz is so low it is not even ultrasonic!
  BEAUTIFUL!
*/

#include "ledchan.h"

enum class State {
  AWAIT_DATA,
  READ_DATA
};
State state = State::AWAIT_DATA;

int offset = 0; // listen to the first 16 channels.

int head = 0;
char buff[1024];

int data_length = 0;
int data_sum = 0;
int temp_framebuffer[16] = {
  0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0
};

int no_errors = 0;

void setup() {
  Serial.begin(19200);
  ledchan_begin();
}

void dataError(){
  head = 0;
  state = State::AWAIT_DATA;
}

int from64_error = 0;
int from64(char c) {
  if (c >= 'A' && c <= 'Z') return c - 'A';      // A-Z -> 0-25
  if (c >= 'a' && c <= 'z') return c - 'a' + 26; // a-z -> 26-51
  if (c >= '0' && c <= '9') return c - '0' + 52; // 0-9 -> 52-61
  if (c == '+') return c - '+' + 62;
  if (c == '/') return c - '/' + 63;
  from64_error = 1;
  return 0;  // Default to 0 if invalid character
}

void loop()
{
  if(!no_errors)
  {
    head = 0;
    state = State::AWAIT_DATA;
  }

  switch(state)
  {
    case State::AWAIT_DATA:
      while(Serial.available())
      {
        char c = Serial.read();
        buff[head++] = c;
        if(c == '\n') {
          state = State::READ_DATA;
          head = 0;
        }
      }
      break;
    
    case State::READ_DATA:
      no_errors = 0; // assume everything's wrong until otherwise noted lol
      from64_error = 0; // If at any point from64 failed, we'll know

      // Check if anything looks weird
      if (buff[head++] != '<') break;
      if (buff[head++] != '=') break;
      if (buff[head++] != '>') break;

      data_length  = from64(buff[head++])*64;
      data_length += from64(buff[head++]);

      // Ok, let's assume we got everything right so far
      head += offset;

      data_sum = 0;
      for(int i = 0; i < 16; i++)
      {
        int data = from64(buff[head++]);
        data_sum += data;
        temp_framebuffer[i] = data;
      }

      head = 5 + data_length; // '<=>' + 'data_length' + Data Length
      int checksum = from64(buff[head++]);
      checksum *= 64; checksum += from64(buff[head++]); 
      checksum *= 64; checksum += from64(buff[head++]); 
      checksum *= 64; checksum += from64(buff[head++]); 
      checksum *= 64; checksum += from64(buff[head++]);
      
      if (data_sum != checksum) break;
      if (buff[head] != '\n') break;
      if(from64_error) break;
      no_errors = 1;

      for(int i = 0; i < 16; i++)
      {
        framebuffer[i] = temp_framebuffer[i];
      }
      ledchan_update();
      break;
  }
}





