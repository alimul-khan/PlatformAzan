#include <ESP8266WiFi.h>
#include "espServerManager.h"  // Handles the web server and routes
#include "EEPROMManager.h"     // Manages EEPROM operations
// #include "Constants.h"         // Stores constants like DEFAULT_ENDPOINT

#include "sendData.h"

void printStoredConfiguration();


WiFiClient wifiClient;

// Wi-Fi credentials for the ESP8266's Access Point
const char *ssidAP = "Azan";

// Timer for periodic Serial Monitor updates
unsigned long lastPrintTime = 0;
float frame[32*24];
String serialNumber;


void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
    // Start Serial Monitor
    Serial.begin(115200);
    Serial.println("\nThermoCam Configuration Starting...");

    // Initialize EEPROM
    initializeEEPROM();

    // Load stored credentials
    loadCredentials();

    // Start the Wi-Fi Access Point
    WiFi.softAP(ssidAP);
    Serial.print("Access Point created. Connect to '");
    Serial.print(ssidAP);
    Serial.println("'.");

    // Print the AP's IP Address
    Serial.print("Access Point IP Address: ");
    Serial.println(WiFi.softAPIP());

    // Initialize the web server
    initializeServer();
    printStoredConfiguration();
}



void attemptWiFiConnection() {
 printStoredConfiguration();
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



void handleConnectedOperations() {
    // Check if Wi-Fi is disconnected
    if (WiFi.status() != WL_CONNECTED) {
        // Exit this function immediately when disconnected
        return;
    }

    // Variables for blinking and logging
    static unsigned long lastOperationTime = 0;
    static bool ledState = false;

    // Use the same timer for both LED blinking and Serial logging
    unsigned long currentMillis = millis();
    if (currentMillis - lastOperationTime >= storedPostInterval) {
        lastOperationTime = currentMillis;

        // Toggle LED state
        ledState = !ledState;
        digitalWrite(LED_BUILTIN, ledState ? LOW : HIGH);
//        Serial.print("LED state: ");
//        Serial.println(ledState ? "ON" : "OFF");




        // Print thermal data
//        Serial.println("Thermal Data:");
//        Serial.println(thermalData);

        // Print stored configuration to Serial Monitor
        printStoredConfiguration();
    }
}




void printStoredConfiguration() {
    Serial.println("Stored Configuration:");
    Serial.println("SSID: " + storedSSID);
    Serial.println("Password: " + storedPassword);
    Serial.println("IP: " + storedIP);
    Serial.println("Email: " + storedEmailAddress);
    Serial.println("Endpoint: " + storedEndpoint);
    Serial.println("Post Interval: " + String(storedPostInterval));
    Serial.print("Serial Number: "  + String(serialNumber));
    Serial.println();
}



void loop() {
    // Handle HTTP requests
//    handleServerRequests();
    server.handleClient();

    // If not connected, attempt to connect with saved credentials
    if (WiFi.status() != WL_CONNECTED) {
        attemptWiFiConnection();
    } else {
        // If connected, proceed with normal operation
        handleConnectedOperations();
    }
}
