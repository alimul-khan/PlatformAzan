#ifndef TIMEMANAGER_H
#define TIMEMANAGER_H

#include <WiFiUdp.h>
#include <NTPClient.h>

extern WiFiUDP ntpUDP;
extern NTPClient timeClient;

void initializeTimeClient();
String currentTime();

void getCurrentYMD(int& year, int& month, int& day);

#endif
