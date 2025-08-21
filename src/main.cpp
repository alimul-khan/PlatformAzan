// src/main.cpp
#include <Arduino.h>

// Explicitly set pins (match your wiring)
#define AZAN_RX_PIN D5  // ESP RX  <- Module TX
#define AZAN_TX_PIN D6  // ESP TX  -> Module RX
#include "AzanPlay.h"

void setup() {
  Serial.begin(115200);
  delay(50);
  Serial.println("\n[main] AzanPlay debug runner");
}

void loop() {
  Serial.println("[main] Play Fajr (0001)...");
  playFajr();     // blocks until 0001.mp3 finishes

  Serial.println("[main] Play Others (0002)...");
  playOthers();   // blocks until 0002.mp3 finishes
}
