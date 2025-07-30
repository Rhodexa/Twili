/*
  Twili Classic:
  |-- CPU only cares about this --| |---- Other CPUs' business  ----|
  **************** **************** **************** **************** 
  TW<dddddddddddd> ccte------------ TW<dddddddddddd> ccte------------

  A Twili packets are variable in size. However, it is the Galaxy's job to discern the packet length it expects
  and it is the controller's job to size packets properly for each galaxy.
  A packet can be one of two types:

  A Data Packet:
    'TW'<data><cc><target_id>\n
    1. Start-of-packet (SOP) string. 'TW'
    2. Followed by a stream of channel values in base128
    3. Next comes two chars of checksum. Literal sum of all channels
    4. Target byte in base128 too. Defines which galaxy address this packet is targeted to.
    5. End-of-packet (EOP) byte. Literal Line Feed character '\n'

    Parsing:
      1. A Galaxy can check if the packet is targeted to it by checking the char one-less than the EOP char
      2. Then, the parser may go back 2 + channel_count to reach the packet start.
      3. At this point, there should be the 'TW' string present
      4. Keep moving forward to read channel data
      5. Compare sum with <cc> (checksum)
      6. We should be back at \n now

  A Swap Packet:
    'SWAP'\n
    1. Literally the string 'SWAP' ending with '\n' (EOP)

    A SWAP is an End of Frame packet. Signalling the end of transmissions for that video frame.
    A Swap Packet tells the Galaxy to swap buffers, which essentially means to show the last packet received.
    Motorized lights might want to ignore these and update on packet arrival.
    Swap is mostly for lighting only, it allows lights to sync up and fire at the same time.
*/

#include "ledchan.h"
#include "jumpers.h"

//#define PUKE

// This device's target index (Which packet index are we listening for)
// Maybe this can be set up using the cutting-edge technology of DIP switches or Jumpers.
// This can be configured using jumpers inside the box.
int target_id = 0;

void setup()
{
  Serial.begin(57600);
  ledchan_begin();
  jumpers_begin();
  delay(1000);
  //target_id = jumpers_get();
  Serial.print("Booted! ID:");
  Serial.println(target_id);
}

class NumberParser {
public:
  bool failed = true;

  int parse(char c) {
    if (!(c & 0x80)) {
      failed = true;
      #ifdef PUKE
        Serial.print("Number Error  ");
        Serial.print(c);
      #endif
      return -1;
    }
    failed = false;
    return c & 0x7F;  // Return the 7-bit value
  }
};
NumberParser number;

class Buffer {
public:
  static const int BUFFER_SIZE = 256;
  uint8_t data[BUFFER_SIZE];
  uint8_t write_head = 0;
  uint8_t read_head = 0;
  void write(char c){
    data[write_head++] = c;
  }
  char read(){
    return data[read_head++];
  }
};
Buffer buffer;

const int CHANNEL_COUNT = 15;
const int PACKET_LENGTH = 6 + CHANNEL_COUNT; // Total length of a Twili packet for this galaxy including target ID (1), checksum (2), SOP (2) and EOP (1) = 1+2+2+1 = 6 => +15 channels

auto curtime = millis();
auto lastime = curtime;
void loop()
{
  while(Serial.available())
  {
    char c = Serial.read();
    buffer.write(c);
    if(c == '\n') {
      int error = parseData();
      #ifdef PUKE
        if(error) {
          Serial.print("Error ");
          Serial.println(error);
        }
      #endif
    }
  }

  curtime = millis();
  if(curtime - lastime >= 5){
    lastime = curtime;
    run();
  }
}

int framebuffer_temp[CHANNEL_COUNT];
int incoming_framebuffer[CHANNEL_COUNT];

int parseData()
{
  // Sync heads
  buffer.read_head = buffer.write_head-1; 

  // Is this packet for us?
  if(target_id != number.parse(buffer.data[buffer.read_head - 1])) // position(EOP) - 1 <target_id> char
  {
    // Looks like it isn't our ID, could it be a SWAP, or worse, a char error?
    if(!number.failed) return 8; // Just not our ID. No problem!
    if(buffer.data[buffer.read_head - 1] != 'P') return 1;
    if(buffer.data[buffer.read_head - 2] != 'A') return 1;
    if(buffer.data[buffer.read_head - 3] != 'W') return 1;
    if(buffer.data[buffer.read_head - 4] != 'S') return 1;

    // Looks like it was a SWAP!
    for(int i = 0; i < CHANNEL_COUNT; i++)
    {
      framebuffer[i] = framebuffer_temp[i];
    }
    ledchan_update(); // Update PWM
    return 0;
  }

  // Data Packet for us!
  buffer.read_head -= PACKET_LENGTH - 1; // Go back in time to the start of the command

  // Is the start code here?
  if(buffer.read() != 'T') return 2;
  if(buffer.read() != 'W') return 2;

  // Start code was there! Let's read the data... also let's take in the sum
  int data_sum = 0;
  for(int i = 0; i < CHANNEL_COUNT; i++)
  {
    int data = number.parse(buffer.read());
    data_sum += data;
    incoming_framebuffer[i] = data;
  }

  // Get the checksum we expect
  int checksum = number.parse(buffer.read()) * 128;
  checksum += number.parse(buffer.read());
  if(number.failed) return 5; // we probably don't need this, but oh well

  // Is our checksum the same as expected?
  if(data_sum != checksum) return 3;

  // Also not needed! But useful for debugging (look at me defending my overengineered nonsense)
  buffer.read_head ++; //dummy ID read
  if(buffer.read() != '\n') return 4;

  // Time to update buffers
  for(int i = 0; i < CHANNEL_COUNT; i++)
  {
    // If there was a rogue character error.. lets ignore it!
    // Update everything else instead (I know, checksums, this should never happen)
    // And yes, I realise i could use a pointer swap instead, but its 15 items come on!

    // I'm not paranoid! You are!
    if(incoming_framebuffer[i] != -1)
      framebuffer_temp[i] = incoming_framebuffer[i];
  }
  return 0;
}