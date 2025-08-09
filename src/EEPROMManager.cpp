#include "EEPROMManager.h"
#include "Constants.h" // Include the constants header for EEPROM address offsets

extern String storedSSID, storedPassword, storedIP;
extern String storedEmailAddress, storedEndpoint, storedDeviceUUID;
// extern String storedDeviceUUID;
extern int storedHostPort, storedPostInterval;


void initializeEEPROM() {
  // EEPROM.begin(512);
  EEPROM.begin(1024);
}

void storeCredentials() {
  writeStringToEEPROM(SSID_ADDR, storedSSID);
  writeStringToEEPROM(PASSWORD_ADDR, storedPassword);
  writeStringToEEPROM(SSID_IP, storedIP);
  // Store location and timezone info
  writeStringToEEPROM(LAT_ADDR, storedLatitude);
  writeStringToEEPROM(LON_ADDR, storedLongitude);
  writeStringToEEPROM(TZ_ADDR, storedTimeZone);
  writeStringToEEPROM(CITY_ADDR, storedCity);
  writeStringToEEPROM(COUNTRY_ADDR, storedCountry);

  // Store host and data transfer parameters
  writeStringToEEPROM(HOST_ADDR, storedEmailAddress);
  EEPROM.put(HOST_PORT, storedHostPort);
  writeStringToEEPROM(ENDPOINT_ADDR, storedEndpoint);
  writeStringToEEPROM(DEVICE_UUID, storedDeviceUUID);
  EEPROM.put(POST_INTERVAL, storedPostInterval);

  EEPROM.write(0, 1);  // Set a flag indicating valid data is stored
  EEPROM.commit();
}

void loadCredentials() {
  if (EEPROM.read(0) == 1) {  // Check if data is valid
    storedSSID = readStringFromEEPROM(SSID_ADDR);
    storedPassword = readStringFromEEPROM(PASSWORD_ADDR);
    storedIP = readStringFromEEPROM(SSID_IP);

    // Load location and timezone info
  storedLatitude  = readStringFromEEPROM(LAT_ADDR);
  storedLongitude = readStringFromEEPROM(LON_ADDR);
  storedTimeZone  = readStringFromEEPROM(TZ_ADDR);
  storedCity      = readStringFromEEPROM(CITY_ADDR);
  storedCountry   = readStringFromEEPROM(COUNTRY_ADDR);

    // Load host and data transfer parameters
    storedEmailAddress = readStringFromEEPROM(HOST_ADDR);
    EEPROM.get(HOST_PORT, storedHostPort);
    EEPROM.get(POST_INTERVAL, storedPostInterval);
    storedEndpoint = readStringFromEEPROM(ENDPOINT_ADDR);
  } else {
    Serial.println("No valid data in EEPROM");
  }
}


String readStringFromEEPROM(int startAddr) {
  int length = EEPROM.read(startAddr);
  char data[length + 1];
  for (int i = 0; i < length; i++) {
    data[i] = EEPROM.read(startAddr + 1 + i);
  }
  data[length] = '\0';
  return String(data);
}


void writeStringToEEPROM(int startAddr, const String &data) {
  int length = data.length();
  EEPROM.write(startAddr, length);
  for (int i = 0; i < length; i++) {
    EEPROM.write(startAddr + 1 + i, data[i]);
  }
}
