#include "TimeManager.h"
#include "Constants.h"   // for extern String storedTimeZone

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 0, 60000); // UTC offset 0, update every 60s

void initializeTimeClient() {
    // storedTimeZone is a String like "-6.00" or "UTC"
    float tz = 0.0f;
    if (storedTimeZone.length() && storedTimeZone != "UTC") {
        tz = storedTimeZone.toFloat();   // e.g., -6.00
    }
    timeClient.setTimeOffset((long)(tz * 3600));  // seconds
    timeClient.begin();
}


String currentTime() {
    timeClient.update();

    int hours = timeClient.getHours();
    int minutes = timeClient.getMinutes();
    int seconds = timeClient.getSeconds();

    time_t epochTime = timeClient.getEpochTime();
    struct tm *ptm = gmtime((time_t *)&epochTime);

    int year  = ptm->tm_year + 1900;
    int month = ptm->tm_mon + 1;
    int day   = ptm->tm_mday;

    char buffer[25];
    snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d %02d:%02d:%02d",
             year, month, day, hours, minutes, seconds);

    return String(buffer);
}
