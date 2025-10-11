import javax.sound.midi.*;
import processing.serial.*;

Serial myPort;

final int NUM_CHANNELS = 15;
int[] twiliChannels = new int[NUM_CHANNELS];
int[] lastSentChannels = new int[NUM_CHANNELS];

boolean packetDirty = false;
Receiver dummyOut;
Transmitter midiIn;

void setup() {
  size(400, 100);
  printArray(Serial.list());
  myPort = new Serial(this, "COM5", 115200);

  for (int i = 0; i < NUM_CHANNELS; i++) {
    twiliChannels[i] = 0;
    lastSentChannels[i] = -1;
  }

  frameRate(60);

  println("=== Available MIDI Devices ===");
  MidiDevice.Info[] infos = MidiSystem.getMidiDeviceInfo();
  for (int i = 0; i < infos.length; i++) {
    println("[" + i + "] " + infos[i].getName() + " — " + infos[i].getDescription());
  }

  try {
    // Change index as needed: find your "loopMIDI" from the list above
    MidiDevice device = MidiSystem.getMidiDevice(infos[5]); // adjust index!
    device.open();

    midiIn = device.getTransmitter();
    midiIn.setReceiver(new TwiliReceiver());

    println("MIDI input set up successfully.");
  } catch (Exception e) {
    println("Failed to init MIDI:");
    e.printStackTrace();
  }
}

float t = 0;
void draw() {
  if (packetDirty) {
    sendTwiliPacket();
    packetDirty = false;
  }
}

void sendTwiliPacket() {
  byte[] packet = new byte[2 + NUM_CHANNELS + 1 + 1]; // 'GILDA' + 15 channels + NUM_CHANNELS '\n'
  int idx = 0;

  // Start-of-packet
  packet[idx++] = (byte)'G';
  packet[idx++] = (byte)'L';

  for (int i = 0; i < NUM_CHANNELS; i++) {
    int val = (twiliChannels[i] & 0x7F) + 0x80;
    packet[idx++] = (byte)(val & 0xFF);
  }
  
  packet[idx++] = (byte)(0x80 + NUM_CHANNELS);
  packet[idx] = (byte)'\n';

  myPort.write(packet);
}


class TwiliReceiver implements Receiver {
  public void send(MidiMessage message, long timeStamp) {
    byte[] data = message.getMessage();
    
    if ((data[0] & 0xF0) == 0xB0) { // CC message
      int cc = data[1] & 0x7F;
      int value = data[2] & 0x7F;

      if (cc >= 1 && cc <= NUM_CHANNELS) {
        int index = cc - 1;
        twiliChannels[index] = value;
        packetDirty = true;
        //println("CC#" + cc + " > Twili[" + index + "] = " + value);
      }
    }
    
  }

  public void close() {}
}

void serialEvent(Serial p) {
  String incoming = p.readStringUntil('\n');
  if (incoming != null) {
    println("[Galaxy] " + incoming.trim());
  }
}
