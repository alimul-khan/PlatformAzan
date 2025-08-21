#include "AzanPlay.h"
#include <SoftwareSerial.h>

// Wiring:
//  MP3 TX -> ESP D5 (GPIO14) [ESP RX]
//  MP3 RX <- ESP D6 (GPIO12) [ESP TX]
//  GND <-> GND, VCC 3.2–5.2 V (level-shift MP3 TX if VCC=5V)

#define RX_PIN D5
#define TX_PIN D6
static SoftwareSerial mp3Serial(RX_PIN, TX_PIN);  // RX, TX

// ---- YX5300 helpers ----
static uint16_t checksum(uint8_t cmd, uint8_t fb, uint8_t p1, uint8_t p2) {
  uint32_t sum = 0xFF + 0x06 + cmd + fb + p1 + p2;
  return 0xFFFF - sum + 1;
}
static void sendCmd(uint8_t cmd, uint16_t param, uint8_t fb = 1) {
  uint8_t p1 = (param >> 8) & 0xFF, p2 = param & 0xFF;
  uint16_t cs = checksum(cmd, fb, p1, p2);
  uint8_t pkt[10] = {0x7E,0xFF,0x06,cmd,fb,p1,p2,(uint8_t)(cs>>8),(uint8_t)cs,0xEF};
  mp3Serial.write(pkt, sizeof(pkt));
  mp3Serial.flush();
}
// Read one framed packet 0x7E ... 0xEF (typical len=10)
static bool readPacket(uint8_t *buf, size_t &len, unsigned long timeout_ms = 600) {
  enum { WAIT_START, COLLECT } st = WAIT_START;
  size_t idx = 0;
  unsigned long t0 = millis();
  while (millis() - t0 < timeout_ms) {
    if (!mp3Serial.available()) { delay(1); yield(); continue; }
    uint8_t b = mp3Serial.read();
    if (st == WAIT_START) {
      if (b == 0x7E) { st = COLLECT; buf[idx++] = b; }
    } else {
      buf[idx++] = b;
      if (b == 0xEF && idx >= 10) { len = idx; return true; }
      if (idx >= 32) { len = idx; return true; }  // safety
    }
  }
  return false;
}

// ---- Public APIs ----
void azanSetup() {
  Serial.begin(115200);
  mp3Serial.begin(9600);
  delay(5000);              // let module + SD init

  sendCmd(0x06, 20, 1);     // volume 0..30
  sendCmd(0x09, 0x0002, 1); // select TF (microSD)
  Serial.println("MP3 is getting ready, it may take a while...");
}

void playMP3Index(uint16_t i) {        // /MP3/0001.mp3 -> i=1
  Serial.print("Playing music ");
  Serial.println(i);
  sendCmd(0x12, i, 1);                 // play from /MP3 folder
}

bool waitForFinish(unsigned long timeout_ms) {
  unsigned long t0 = millis();
  while (millis() - t0 < timeout_ms) {
    uint8_t pkt[32]; size_t n = 0;
    if (readPacket(pkt, n, 300)) {
      if (pkt[3] == 0x3D) {                 // play finished
        Serial.println("Playing music completed");
        return true;
      }
    }
    delay(1); yield();
  }
  Serial.println("Timed out waiting for finish");
  return false;  // timed out
}
