
#include <ESP8266WiFi.h>
#include "espServerManager.h"  // Handles the web server and routes
#include "EEPROMManager.h"     // Manages EEPROM operations


#include "TimeManager.h"

#include "PrayerTimes.h"
#include "AzanPlay.h"


void printStoredConfiguration();
void printPrayerTimesToday();

WiFiClient wifiClient;

// Wi-Fi credentials for the ESP8266's Access Point
const char *ssidAP = "Azan";

// Timer for periodic Serial Monitor updates
unsigned long lastPrintTime = 0;
float frame[32*24];
String serialNumber;



void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    Serial.begin(115200);
    Serial.println("\nSmart Azan Player Configuration Starting...");

    initializeEEPROM();
    loadCredentials();

    WiFi.softAP(ssidAP);
    Serial.print("Access Point created. Connect to '");
    Serial.print(ssidAP);
    Serial.println("'.");

    Serial.print("Access Point IP Address: ");
    Serial.println(WiFi.softAPIP());

    initializeServer();
    // Start NTP client
    initializeTimeClient();

    printStoredConfiguration();

      Serial.println("\nAzanPlay debug test");

  initAzanPlay(); // uses LED_BUILTIN by default

}




void attemptWiFiConnection() {
//  printStoredConfiguration();
    // Try to connect using stored SSID and password
    WiFi.begin(storedSSID.c_str(), storedPassword.c_str());

    // Variables for blinking logic
    unsigned long blinkLastTime = 0;
    static bool ledState = false;

    // Print waiting message to Serial Monitor
    Serial.println("Attempting to connect to Wi-Fi using stored credentials...");

    // Give time for connection
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startTime < 10000) { // Wait for 10 seconds
        unsigned long currentMillis = millis();

        // Blink LED every 100ms
        if (currentMillis - blinkLastTime >= 100) {
            blinkLastTime = currentMillis;
            digitalWrite(LED_BUILTIN, ledState ? LOW : HIGH); // Toggle LED
            ledState = !ledState;
        }

        // Handle incoming HTTP requests to update credentials
        server.handleClient();

        // Small delay to avoid overloading the CPU
        delay(10);
    }

    // Check connection status
    if (WiFi.status() == WL_CONNECTED) {
        Serial.println("\nConnected successfully!");
        Serial.println("IP Address: " + WiFi.localIP().toString());
    } else {
        Serial.println("\nFailed to connect within the timeout period.");
    }
}


// Return minutes-from-midnight and seconds from the TimeManager's currentTime()
static void getNowMinSec(int& nowMin, int& nowSec) {
  String ts = currentTime();              // "YYYY-MM-DD HH:MM:SS"
  int hh = ts.substring(11, 13).toInt();
  int mm = ts.substring(14, 16).toInt();
  nowSec  = ts.substring(17, 19).toInt();
  nowMin  = hh * 60 + mm;
}

void handleConnectedOperations() {
  if (WiFi.status() != WL_CONNECTED) return;

  static unsigned long lastOperationTime = 0;
  static bool ledState = false;
  static int lastTriggered = -1;  // -1 = none, 0=Fajr,1=Sunrise,2=Dhuhr,3=Asr,4=Maghrib,5=Isha

  unsigned long currentMillis = millis();
  if (currentMillis - lastOperationTime >= 5000) {
    lastOperationTime = currentMillis;

    // Blink heartbeat
    ledState = !ledState;
    digitalWrite(LED_BUILTIN, ledState ? LOW : HIGH);

    // Get current time string
    String nowStr = currentTime();
    Serial.println("Check at " + nowStr);

    // Compute today's prayer times
    int year, month, day;
    getCurrentYMD(year, month, day);
    double lat = storedLatitude.toFloat();
    double lon = storedLongitude.toFloat();
    float  tz  = (storedTimeZone == "UTC" || storedTimeZone.length() == 0) ? 0.0f : storedTimeZone.toFloat();

    PrayerTimes pt;
    computePrayerTimes(year, month, day, lat, lon, tz, -15.0, -15.0, ASR_HANAFI, pt);

    // Match current time
    int nowMin, nowSec;
    getNowMinSec(nowMin, nowSec);

    bool matched = false;
    String matchName = "";

    auto checkAndTrigger = [&](int targetMin, int id, const char* name, bool fajr=false) {
      if (nowMin == targetMin && nowSec == 0 && lastTriggered != id) {
        matched = true;
        matchName = name;
        if (fajr) playFajr(); else playOthers();
        lastTriggered = id;
      }
    };

    checkAndTrigger(pt.fajrMin,    0, "Fajr", true);
    checkAndTrigger(pt.dhuhrMin,   2, "Dhuhr");
    checkAndTrigger(pt.asrMin,     3, "Asr");
    checkAndTrigger(pt.maghribMin, 4, "Maghrib");
    checkAndTrigger(pt.ishaMin,    5, "Isha");

    // Debug output
    if (matched) {
      Serial.println("Matching result: Matched with " + matchName);
    } else {
      Serial.println("Matching result: No match");
    }
    Serial.println();
  }
}




void printStoredConfiguration() {
    Serial.println("Stored Configuration:");
    Serial.println("SSID: " + storedSSID);
    Serial.println("Password: " + storedPassword);
    Serial.println("IP: " + storedIP);

    Serial.println("Serial Number: "  + String(serialNumber));
    Serial.println("Latitude: "  + storedLatitude);
    Serial.println("Longitude: " + storedLongitude);
    Serial.println("Time Zone: " + storedTimeZone);
    Serial.println("City: "      + storedCity);
    Serial.println("Country: "   + storedCountry);
    Serial.println("Printing Done: ....................................................");
   


}


void printPrayerTimesToday() {
    // Get today's date from NTP
    timeClient.update();
    time_t epoch = timeClient.getEpochTime();
    struct tm *ptm = gmtime((time_t *)&epoch);
    int year  = ptm->tm_year + 1900;
    int month = ptm->tm_mon + 1;
    int day   = ptm->tm_mday;

    // Inputs from your stored fields
    double lat = storedLatitude.toFloat();
    double lon = storedLongitude.toFloat();
    float  tz  = (storedTimeZone == "UTC" || storedTimeZone.length() == 0)
                   ? 0.0f : storedTimeZone.toFloat();

    // Compute (Fajr/Isha twilight = -15°, Asr = Hanafi here; change to ASR_SHAFII if you prefer)
    PrayerTimes pt;
    computePrayerTimes(year, month, day, lat, lon, tz, -15.0, -15.0, ASR_HANAFI, pt);

    // Print nicely
    Serial.println("Today's Prayer Times:");
    Serial.println("  Fajr:    " + toHHMM(pt.fajrMin));
    Serial.println("  Sunrise: " + toHHMM(pt.sunriseMin));
    Serial.println("  Dhuhr:   " + toHHMM(pt.dhuhrMin));
    Serial.println("  Asr:     " + toHHMM(pt.asrMin));
    Serial.println("  Maghrib: " + toHHMM(pt.maghribMin));
    Serial.println("  Isha:    " + toHHMM(pt.ishaMin));
}

void loop() {
    // Handle HTTP requests
  //  handleServerRequests();
    server.handleClient();

    // If not connected, attempt to connect with saved credentials
    if (WiFi.status() != WL_CONNECTED) {
        attemptWiFiConnection();
    } else {
        // If connected, proceed with normal operation
        handleConnectedOperations();
        

    }
}


