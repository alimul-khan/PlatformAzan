#include <Arduino.h>
#include <SoftwareSerial.h>

// Wiring:
// ESP D5 (GPIO14) <- MP3 TX
// ESP D6 (GPIO12) -> MP3 RX
// GND <-> GND, VCC 5V (module supports 3.2–5.2V)
// ⚠ If VCC=5V, use voltage divider on MP3 TX -> ESP RX.

#define RX_PIN D5
#define TX_PIN D6
SoftwareSerial mp3Serial(RX_PIN, TX_PIN);  // RX, TX

// --- YX5300 packet helpers ---
uint16_t checksum(uint8_t cmd, uint8_t fb, uint8_t p1, uint8_t p2) {
  uint32_t sum = 0xFF + 0x06 + cmd + fb + p1 + p2;
  return 0xFFFF - sum + 1;
}
void sendCmd(uint8_t cmd, uint16_t param, uint8_t fb = 1) {
  uint8_t p1 = (param >> 8) & 0xFF;
  uint8_t p2 = (param     ) & 0xFF;
  uint16_t cs = checksum(cmd, fb, p1, p2);
  uint8_t pkt[10] = {0x7E, 0xFF, 0x06, cmd, fb, p1, p2,
                     (uint8_t)(cs >> 8), (uint8_t)cs, 0xEF};
  mp3Serial.write(pkt, sizeof(pkt));
  mp3Serial.flush();
}

void setup() {
  Serial.begin(115200);
  mp3Serial.begin(9600);
  delay(1500);               // let module + SD card boot

  sendCmd(0x06, 20);          // set volume (0..30)
  Serial.println("MP3 module ready.");
}

void loop() {
  // play 0001.mp3 (index 1)
  sendCmd(0x03, 2);
  Serial.println("Playing 0001.mp3");
  delay(5000);                // adjust to track length

  // stop
  sendCmd(0x16, 0);
  delay(500);

  // play 0002.mp3 (index 2)
  sendCmd(0x03, 3);
  Serial.println("Playing 0002.mp3");
  delay(5000);

  // stop
  sendCmd(0x16, 0);
  delay(500);
}
