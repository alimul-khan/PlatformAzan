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

    // Replace placeholders
    htmlContent.replace("{{OPTIONS}}", options);
    htmlContent.replace("{{SSID}}", storedSSID);
    htmlContent.replace("{{IP}}", storedIP);

    htmlContent.replace("{{LAT}}",      storedLatitude);
    htmlContent.replace("{{LON}}",      storedLongitude);
    htmlContent.replace("{{TZ}}",       storedTimeZone);
    htmlContent.replace("{{CITY}}",     storedCity);
    htmlContent.replace("{{COUNTRY}}",  storedCountry);


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
void handleDataConfig() {
    // Accept any subset of fields
    if (server.hasArg("email"))          storedEmailAddress = server.arg("email");
    if (server.hasArg("post_interval"))  storedPostInterval = server.arg("post_interval").toInt();
    if (server.hasArg("endpoint"))       storedEndpoint     = server.arg("endpoint");

    if (server.hasArg("latitude"))       storedLatitude  = server.arg("latitude");
    if (server.hasArg("longitude"))      storedLongitude = server.arg("longitude");
    if (server.hasArg("timezone"))       storedTimeZone  = server.arg("timezone");
    if (server.hasArg("city"))           storedCity      = server.arg("city");
    if (server.hasArg("country"))        storedCountry   = server.arg("country");

    Serial.println("Configuration Received:");
    Serial.println("Email: " + storedEmailAddress);
    Serial.println("Post Interval: " + String(storedPostInterval));
    Serial.println("Endpoint: " + storedEndpoint);
    Serial.println("Latitude: " + storedLatitude);
    Serial.println("Longitude: " + storedLongitude);
    Serial.println("Time Zone: " + storedTimeZone);
    Serial.println("City: " + storedCity);
    Serial.println("Country: " + storedCountry);

    storeCredentials(); // Save to EEPROM

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
