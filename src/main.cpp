



#include <Arduino.h>
#include <ESP8266WiFi.h>
#include "espServerManager.h"  // Handles the web server and routes
#include "EEPROMManager.h"     // Manages EEPROM operations
#include <ESP8266mDNS.h>


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

    delay(5000);
    azanSetup();

    initializeEEPROM();
    loadCredentials();

    WiFi.softAP(ssidAP);
    Serial.print("Access Point created. Connect to '");
    Serial.print(ssidAP);
    Serial.println("'.");

    Serial.print("Access Point IP Address: ");
    Serial.println(WiFi.softAPIP());
    WiFi.mode(WIFI_AP_STA);
    WiFi.hostname("azan");

    initializeServer();
    // Start NTP client
    initializeTimeClient();

    printStoredConfiguration();

      Serial.println("\nAzanPlay debug test");


}




void attemptWiFiConnection() {
    // Ensure we're in STA mode and hostname is set before connecting
    WiFi.mode(WIFI_STA);
    WiFi.hostname("azan");   // use azan.local

    // Try to connect using stored SSID and password
    Serial.println("Attempting to connect to Wi-Fi using stored credentials...");
    WiFi.begin(storedSSID.c_str(), storedPassword.c_str());

    // Blink LED while waiting
    unsigned long blinkLastTime = 0;
    static bool ledState = false;

    // Wait up to 10 seconds
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED && (millis() - startTime) < 10000UL) {
        unsigned long now = millis();

        // Blink every 100 ms
        if (now - blinkLastTime >= 100UL) {
            blinkLastTime = now;
            ledState = !ledState;
            digitalWrite(LED_BUILTIN, ledState ? LOW : HIGH);
        }

        // Keep serving the config page while waiting
        server.handleClient();
        delay(10);
    }

    if (WiFi.status() == WL_CONNECTED) {
        // Stop blinking, set LED off (HIGH on ESP8266 built-in)
        digitalWrite(LED_BUILTIN, HIGH);

        Serial.println("\nConnected successfully!");
        Serial.println("IP Address: " + WiFi.localIP().toString());

        // Start mDNS: http://azan.local
        if (MDNS.begin("azan")) {
            MDNS.addService("http", "tcp", 80);
            Serial.println("mDNS: http://azan.local");
        } else {
            Serial.println("mDNS start failed");
        }
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

  // Recompute prayer times once per day; run matching once per minute
  static int lastY = 0, lastM = 0, lastD = 0;
  static int lastMinuteChecked = -1;
  static PrayerTimes pt;  // cached for the day

  unsigned long currentMillis = millis();
  if (currentMillis - lastOperationTime < 5000) return;
  lastOperationTime = currentMillis;

  // 1) Heartbeat LED
  ledState = !ledState;
  digitalWrite(LED_BUILTIN, ledState ? LOW : HIGH);

  // 2) Print current timestamp
  String nowStr = currentTime();  // "YYYY-MM-DD HH:MM:SS"
  Serial.println("Check at " + nowStr);

  // 3) Prepare inputs & recompute if the date changed
  int y, m, d;
  getCurrentYMD(y, m, d);

  double lat = storedLatitude.toFloat();
  double lon = storedLongitude.toFloat();
  float  tz  = (storedTimeZone == "UTC" || storedTimeZone.length() == 0) ? 0.0f
                                                                         : storedTimeZone.toFloat();

  if (y != lastY || m != lastM || d != lastD) {
    computePrayerTimes(y, m, d, lat, lon, tz, -15.0, -15.0, ASR_HANAFI, pt);
    printPrayerTimesToday();  // print the schedule for the new day
    lastY = y; lastM = m; lastD = d;
  }

  // 4) Minute-level matching (ignore seconds)
  int nowMin, nowSec;
  getNowMinSec(nowMin, nowSec);   // you already have this; we ignore nowSec

  // Only evaluate once when the minute flips
  if (nowMin == lastMinuteChecked) {
    Serial.println("Matching result: No match");
    Serial.println();
    return;
  }
  lastMinuteChecked = nowMin;

  bool matched = false;
  String matchName = "";

  auto trigger = [&](int targetMin, const char* name, bool fajr = false) {
    if (nowMin == targetMin) {
      matched = true;
      matchName = name;
      if (fajr) {
        playMP3Index(1);           // /MP3/0001.mp3 (Fajr)
        waitForFinish(180000); // 166s
        delay(100);
      } else {
        playMP3Index(2);           // /MP3/0002.mp3 (others)
        waitForFinish(180000); //130s
        delay(100);
      }
    }
  };

  trigger(pt.fajrMin,    "Fajr",    true);
  trigger(pt.dhuhrMin,   "Dhuhr");
  trigger(pt.asrMin,     "Asr");
  trigger(pt.maghribMin, "Maghrib");
  trigger(pt.ishaMin,    "Isha");

  if (matched) {
    Serial.println("Matching result: Matched with " + matchName);
  } else {
    Serial.println("Matching result: No match");
  }
  Serial.println();
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
MDNS.update();
    // If not connected, attempt to connect with saved credentials
    if (WiFi.status() != WL_CONNECTED) {
        attemptWiFiConnection();
    } else {
        // If connected, proceed with normal operation
        handleConnectedOperations();
        

    }
}


