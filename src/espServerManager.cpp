#include "espServerManager.h"
#include "Constants.h"
#include <ESP8266mDNS.h>

// Initialize the server
ESP8266WebServer server(80);

// Variables for configuration
String storedSSID = "Not Connected";
String storedPassword = "None";
String storedIP = "None";
String storedDeviceUUID = "None"; // Example UUID

// New location-related fields
String storedLatitude  = "0.0000";
String storedLongitude = "0.0000";
String storedTimeZone  = "UTC";
String storedCity      = "Unknown";
String storedCountry   = "Unknown";



// Initialize the server and define routes
void initializeServer() {
    server.on("/", handleRoot);
    server.on("/data", handleWiFiConfig);
    server.on("/config", handleDataConfig);

    server.begin();
    Serial.println("HTTP server started.");
}

// Handle root page
// Handle root page
void handleRoot() {
    String htmlContent = indexHTML;

    // Inject CSS
    htmlContent.replace("{{CSS}}", mainCSS);

    // Scan for available Wi-Fi networks
    int n = WiFi.scanNetworks();
    String options = "";
    if (n == 0) {
        options = "<option>No networks found</option>";
    } else {
        for (int i = 0; i < n; ++i) {
            options += "<option>" + WiFi.SSID(i) + "</option>";
        }
    }

    // Replace placeholders for Wi-Fi info
    htmlContent.replace("{{OPTIONS}}", options);
    htmlContent.replace("{{SSID}}", storedSSID);
    htmlContent.replace("{{IP}}", storedIP);

    // Prefill City/Country
    htmlContent.replace("{{CITY}}",    storedCity);
    htmlContent.replace("{{COUNTRY}}", storedCountry);

    // ---- Latitude (N/S) ----
    float latF = storedLatitude.length() ? storedLatitude.toFloat() : 0.0f;
    bool  latNeg = latF < 0.0f;
    String latAbs = String(fabs(latF), 6);  // keep precision

    htmlContent.replace("{{LAT_VAL}}",   latAbs);
    htmlContent.replace("{{LAT_N_SEL}}", latNeg ? "" : "selected");
    htmlContent.replace("{{LAT_S_SEL}}", latNeg ? "selected" : "");

    // ---- Longitude (E/W) ----
    float lonF = storedLongitude.length() ? storedLongitude.toFloat() : 0.0f;
    bool  lonNeg = lonF < 0.0f;
    String lonAbs = String(fabs(lonF), 6);

    htmlContent.replace("{{LON_VAL}}",   lonAbs);
    htmlContent.replace("{{LON_E_SEL}}", lonNeg ? "" : "selected");
    htmlContent.replace("{{LON_W_SEL}}", lonNeg ? "selected" : "");

    // ---- Timezone (GMT +/-) ----
    float tzF = (storedTimeZone == "UTC" || storedTimeZone.length() == 0)
                ? 0.0f : storedTimeZone.toFloat();
    bool  tzNeg = tzF < 0.0f;
    String tzAbs = String(fabs(tzF), 2);   // e.g., 6.5

    htmlContent.replace("{{TZ_VAL}}",       tzAbs);
    htmlContent.replace("{{TZ_PLUS_SEL}}",  tzNeg ? "" : "selected");
    htmlContent.replace("{{TZ_MINUS_SEL}}", tzNeg ? "selected" : "");

    server.send(200, "text/html", htmlContent);
}

// Handle Wi-Fi configuration form submission
// Handle Wi-Fi configuration form submission
void handleWiFiConfig() {
    if (server.hasArg("ssid") && server.hasArg("password")) {
        storedSSID     = server.arg("ssid");
        storedPassword = server.arg("password");

        Serial.println("Attempting to connect to Wi-Fi...");

        // Set hostname before connecting
        WiFi.mode(WIFI_STA);
        WiFi.hostname("azan");
        WiFi.begin(storedSSID.c_str(), storedPassword.c_str());

        int timeout = 30; // 30 seconds timeout
        while (WiFi.status() != WL_CONNECTED && timeout > 0) {
            delay(1000);
            Serial.print(".");
            timeout--;
        }

        if (WiFi.status() == WL_CONNECTED) {
            Serial.println("\nConnected successfully!");
            storedIP = WiFi.localIP().toString();

            // Start mDNS responder for azan.local
            if (MDNS.begin("azan")) {
                MDNS.addService("http", "tcp", 80);
                Serial.println("mDNS: http://azan.local");
            } else {
                Serial.println("mDNS start failed");
            }

            server.send(200, "text/plain",
                        "Successfully connected to Wi-Fi! IP: " + storedIP);

            // Save only on success
            storeCredentials();

        } else {
            Serial.println("\nFailed to connect.");
            storedSSID = "Connection Failed";
            storedIP   = "None";
            server.send(200, "text/plain", "Failed to connect to Wi-Fi.");
        }

    } else {
        server.send(400, "text/plain",
                    "Invalid input. Please provide both SSID and password.");
    }
}



// Handle data configuration form submission
// Handle data configuration form submission
void handleDataConfig() {

    // Latitude: N = positive, S = negative
    if (server.hasArg("lat_val") && server.hasArg("lat_hem")) {
        float v = server.arg("lat_val").toFloat();
        String hem = server.arg("lat_hem");
        v = (hem == "S") ? -fabs(v) : fabs(v);
        storedLatitude = String(v, 6);
    }

    // Longitude: E = positive, W = negative
    if (server.hasArg("lon_val") && server.hasArg("lon_hem")) {
        float v = server.arg("lon_val").toFloat();
        String hem = server.arg("lon_hem");
        v = (hem == "W") ? -fabs(v) : fabs(v);
        storedLongitude = String(v, 6);
    }

    // Time zone: GMT + / -
    if (server.hasArg("tz_val") && server.hasArg("tz_sign")) {
        float v = server.arg("tz_val").toFloat();
        String sign = server.arg("tz_sign");
        v = (sign == "-") ? -fabs(v) : fabs(v);
        storedTimeZone = (v == 0.0f) ? "UTC" : String(v, 2);
    }

    // City / Country
    if (server.hasArg("city"))    storedCity    = server.arg("city");
    if (server.hasArg("country")) storedCountry = server.arg("country");

    // Debug
    Serial.println("Configuration Received:");
    Serial.println("Latitude: "  + storedLatitude);
    Serial.println("Longitude: " + storedLongitude);
    Serial.println("Time Zone: " + storedTimeZone);
    Serial.println("City: "      + storedCity);
    Serial.println("Country: "   + storedCountry);

    storeCredentials();
    server.send(200, "text/plain", "Configuration saved successfully!");
}


