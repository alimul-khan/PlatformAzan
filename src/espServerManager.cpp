#include "espServerManager.h"
#include "Constants.h"

// Initialize the server
ESP8266WebServer server(80);

// Variables for configuration
String storedSSID = "Not Connected";
String storedPassword = "None";
String storedIP = "None";
String storedEmailAddress = "None";
String storedEndpoint = DEFAULT_ENDPOINT;
int storedPostInterval = 10000; // Default post interval
int storedHostPort = 80;        // Default port
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
    server.on("/update_email", handleUpdateEmail);
    server.on("/update_post_interval", handleUpdatePostInterval);
    server.on("/update_endpoint", handleUpdateEndpoint);

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
void handleWiFiConfig() {
    if (server.hasArg("ssid") && server.hasArg("password")) {
        storedSSID = server.arg("ssid");
        storedPassword = server.arg("password");

        Serial.println("Attempting to connect to Wi-Fi...");
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
            server.send(200, "text/plain", "Successfully connected to Wi-Fi! IP: " + storedIP);
        } else {
            Serial.println("\nFailed to connect.");
            storedSSID = "Connection Failed";
            storedIP = "None";
            server.send(200, "text/plain", "Failed to connect to Wi-Fi.");
        }

        storeCredentials(); // Save to EEPROM
    } else {
        server.send(400, "text/plain", "Invalid input. Please provide both SSID and password.");
    }
}


// Handle data configuration form submission
// Handle data configuration form submission
void handleDataConfig() {
    // Legacy fields (keep for now; safe to remove later)
    if (server.hasArg("email"))          storedEmailAddress = server.arg("email");
    if (server.hasArg("post_interval"))  storedPostInterval = server.arg("post_interval").toInt();
    if (server.hasArg("endpoint"))       storedEndpoint     = server.arg("endpoint");

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


// Handle email update
void handleUpdateEmail() {
    if (server.hasArg("email")) {
        storedEmailAddress = server.arg("email");
        storeCredentials(); // Save to EEPROM
        server.send(200, "text/plain", "Email updated successfully!");
    } else {
        server.send(400, "text/plain", "Invalid email input.");
    }
}

// Handle post interval update
void handleUpdatePostInterval() {
    if (server.hasArg("post_interval")) {
        storedPostInterval = server.arg("post_interval").toInt();
        storeCredentials(); // Save to EEPROM
        server.send(200, "text/plain", "Post Interval updated successfully!");
    } else {
        server.send(400, "text/plain", "Invalid post interval input.");
    }
}

// Handle endpoint update
void handleUpdateEndpoint() {
    if (server.hasArg("endpoint")) {
        String endpoint = server.arg("endpoint");
        if (endpoint == "default") {
            storedEndpoint = DEFAULT_ENDPOINT;
        } else if (server.hasArg("custom_endpoint")) {
            storedEndpoint = server.arg("custom_endpoint");
        }
        storeCredentials(); // Save to EEPROM
        server.send(200, "text/plain", "Endpoint updated successfully!");
    } else {
        server.send(400, "text/plain", "Invalid endpoint input.");
    }
}
