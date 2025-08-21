#include "AzanPlay.h"
#include <SoftwareSerial.h>

// Wiring (ESP8266):
//   Module TX -> D5 (ESP RX)
//   Module RX <- D6 (ESP TX)
//   GND <-> GND, VCC 3.2–5.2 V  (level-shift module TX if VCC=5V)

#ifndef AZAN_RX_PIN
#define AZAN_RX_PIN D5
#endif
#ifndef AZAN_TX_PIN
#define AZAN_TX_PIN D6
#endif

static SoftwareSerial mp3Serial(AZAN_RX_PIN, AZAN_TX_PIN); // RX, TX
static bool initDone = false;
static uint8_t volumeLevel = 20; // 0..30

// -------- Debug helpers (ALWAYS ON) --------
static void dumpHex(const char* prefix, const uint8_t* p, size_t n) {
  Serial.print(prefix);
  for (size_t i = 0; i < n; i++) { if (p[i] < 0x10) Serial.print('0'); Serial.print(p[i], HEX); Serial.print(' '); }
  Serial.println();
}
static void dbg(const char* msg) { Serial.println(msg); }

// -------- YX5300 helpers --------
static uint16_t yxChecksum(uint8_t cmd, uint8_t fb, uint8_t p1, uint8_t p2) {
  uint32_t s = 0xFF + 0x06 + cmd + fb + p1 + p2;
  return (uint16_t)(0xFFFF - s + 1);
}
static void yxSend(uint8_t cmd, uint16_t param, uint8_t fb = 1) {
  uint8_t p1 = (param >> 8) & 0xFF, p2 = param & 0xFF;
  uint16_t cs = yxChecksum(cmd, fb, p1, p2);
  uint8_t pkt[10] = {0x7E,0xFF,0x06,cmd,fb,p1,p2,(uint8_t)(cs>>8),(uint8_t)cs,0xEF};
  dumpHex("TX: ", pkt, sizeof(pkt));
  mp3Serial.write(pkt, sizeof(pkt));
  mp3Serial.flush();
}
static bool yxReadPacket(uint8_t &type, uint16_t &param, unsigned long timeout_ms = 200) {
  enum { WAIT, COLLECT } st = WAIT;
  uint8_t buf[16]; size_t idx = 0;
  unsigned long t0 = millis();
  while (millis() - t0 < timeout_ms) {
    if (!mp3Serial.available()) { delay(1); yield(); continue; }
    uint8_t b = mp3Serial.read();
    if (st == WAIT) { if (b == 0x7E) { st = COLLECT; idx = 0; buf[idx++] = b; } }
    else {
      buf[idx++] = b;
      if (idx >= 10 && b == 0xEF) {
        dumpHex("RX: ", buf, idx);
        type  = buf[3];
        param = ((uint16_t)buf[5] << 8) | buf[6];
        return true;
      }
      if (idx >= sizeof(buf)) return false; // malformed
    }
  }
  return false;
}

static void waitForFinish(unsigned long safetyMs = 10UL * 60UL * 1000UL) {
  dbg("Waiting for finish (0x3D)...");
  unsigned long t0 = millis();
  for (;;) {
    uint8_t t; uint16_t p;
    if (yxReadPacket(t, p, 80)) {
      // Common types: 0x41=ACK, 0x40=ERROR, 0x3F=DEVICE/TF, 0x3D=FINISHED
      if (t == 0x3D) { Serial.print("Finished index "); Serial.println(p); return; }
      if (t == 0x40) { Serial.print("ERROR type 0x40 param="); Serial.println(p); } // log
    }
    if (millis() - t0 > safetyMs) { dbg("Safety timeout, returning."); return; }
    delay(2); yield();
  }
}

static void ensureInit() {
  if (initDone) return;
  Serial.begin(115200); // ensure Serial is up for debug
  dbg("\n[AzanPlay] init...");
  mp3Serial.begin(9600);
  delay(1500);                // let module + SD mount
  yxSend(0x09, 0x0002);       // select TF
  yxSend(0x06, volumeLevel);  // set volume
  initDone = true;
  dbg("[AzanPlay] init done.");
}

static void playIndex(uint16_t idx) {
  ensureInit();
  Serial.print("Play index "); Serial.println(idx);
  yxSend(0x06, volumeLevel);  // ensure volume
  yxSend(0x12, idx);          // play by index (0001 -> 1, etc.)
  waitForFinish();            // block until finished (or timeout)
}

// -------- Public APIs --------
void playFajr()   { playIndex(1); } // 0001.mp3
void playOthers() { playIndex(2); } // 0002.mp3
