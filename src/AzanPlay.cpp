#include "AzanPlay.h"

static int s_ledPin = LED_BUILTIN;

void initAzanPlay(int ledPin) {
  s_ledPin = ledPin;
  pinMode(s_ledPin, OUTPUT);
  digitalWrite(s_ledPin, HIGH); // off initially (active-low on many ESP8266 boards)
}

void playFajr() {
  Serial.println("Playing Fajr (debug)...");
  for (int i = 0; i < 5; i++) {
    digitalWrite(s_ledPin, LOW);   // ON
    delay(1000);
    digitalWrite(s_ledPin, HIGH);  // OFF
    delay(1000);
  }
}

void playOthers() {
  Serial.println("Playing Other Prayer (debug)...");
  for (int i = 0; i < 20; i++) {
    digitalWrite(s_ledPin, LOW);   // ON
    delay(100);
    digitalWrite(s_ledPin, HIGH);  // OFF
    delay(100);
  }
}
