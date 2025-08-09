#ifndef ESP_SERVER_MANAGER_H
#define ESP_SERVER_MANAGER_H

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "webFile.h"
#include "EEPROMManager.h"

// Global Variables
extern ESP8266WebServer server;
extern String storedSSID;
extern String storedPassword;
extern String storedIP;
extern String storedDeviceUUID;
extern String storedLatitude;
extern String storedLongitude;
extern String storedTimeZone;
extern String storedCity;
extern String storedCountry;


// Function Declarations
void initializeServer();
void handleRoot();
void handleWiFiConfig();
void handleDataConfig();


#endif // ESP_SERVER_MANAGER_H
