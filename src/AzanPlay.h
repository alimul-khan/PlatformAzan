#pragma once
#include <Arduino.h>

void azanSetup();                                // init UART + module
void playMP3Index(uint16_t i);                   // play /MP3/000x.mp3
bool waitForFinish(unsigned long timeout_ms = 120000); // wait for 0x3D
