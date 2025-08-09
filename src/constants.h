// Constants.h
#ifndef CONSTANTS_H
#define CONSTANTS_H

extern const char* AP_SSID; // optional; keep if used

// EEPROM addresses (keep non-overlapping)
const int SSID_ADDR     = 2;
const int PASSWORD_ADDR = 52;
const int SSID_IP       = 102;
const int DEVICE_UUID   = 256;

const int LAT_ADDR      = 310;
const int LON_ADDR      = 360;
const int TZ_ADDR       = 410;
const int CITY_ADDR     = 460;
const int COUNTRY_ADDR  = 510;

// Shared variables (location/time only)
extern String storedLatitude;
extern String storedLongitude;
extern String storedTimeZone;
extern String storedCity;
extern String storedCountry;

#endif
