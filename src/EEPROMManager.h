
#ifndef EEPROMMANAGER_H
#define EEPROMMANAGER_H

#include <Arduino.h>
#include <EEPROM.h>

// Function declarations
void initializeEEPROM();
void writeStringToEEPROM(int startAddr, const String &data);
String readStringFromEEPROM(int startAddr);

void storeCredentials();
void loadCredentials();

#endif
