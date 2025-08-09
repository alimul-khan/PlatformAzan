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
extern String storedEmailAddress;
extern String storedEndpoint;
extern int storedPostInterval;
extern int storedHostPort;
extern String storedDeviceUUID;

// Function Declarations
void initializeServer();
void handleRoot();
void handleWiFiConfig();
void handleDataConfig();
void handleUpdateEmail();
void handleUpdatePostInterval();
void handleUpdateEndpoint();

#endif // ESP_SERVER_MANAGER_H
