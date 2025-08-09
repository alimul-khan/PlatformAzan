// Constants.h
#ifndef CONSTANTS_H
#define CONSTANTS_H
// Default endpoint
#define DEFAULT_ENDPOINT "http://thermo-cam.com/receive_data.php"

extern const char* AP_SSID; // Declare as extern here

// Other constants
const int SSID_ADDR = 2;
const int PASSWORD_ADDR = 52;
const int SSID_IP = 102;
const int HOST_ADDR = 152;
const int HOST_PORT = 202;
const int ENDPOINT_ADDR = 206;
const int DEVICE_UUID = 256;
const int POST_INTERVAL = 306;
const int DEFAULT_PORT = 8080;
const int DEFAULT_POST_INTERVAL = 10000;


const int LAT_ADDR     = 310;
const int LON_ADDR     = 360;
const int TZ_ADDR      = 410;
const int CITY_ADDR    = 460;
const int COUNTRY_ADDR = 510;



extern String storedLatitude;
extern String storedLongitude;
extern String storedTimeZone;
extern String storedCity;
extern String storedCountry;



extern String storedEndpoint;
extern String storedEmailAddress;




#endif
