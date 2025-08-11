
#ifndef AZANPLAY_H
#define AZANPLAY_H

#include <Arduino.h>

// Call once in setup (ledPin defaults to LED_BUILTIN)
void initAzanPlay(int ledPin = LED_BUILTIN);

// Debug placeholders (blink patterns)
void playFajr();    // 1s ON / 1s OFF, 5 times
void playOthers();  // 100ms ON / 100ms OFF, 20 times

#endif
