/**
 * @package Temperature Monitor
 * Configuration sample
 * @author WizLab.it
 * @version 20250129.016
 */


/*
 * Generic behaviour
 */

//Use built-in led to identify some events
#define USE_LED true

//Seconds between samples
#define SLEEP_DURATION 10

//If true, goes to deep sleep (extremely low current) between samples
//To use deep sleep, D0 and RST pins on ESP8266 must be connected
#define USE_DEEP_SLEEP false

//If true, post the sensor data online
#define POST_TEMPERATURE_ONLINE false

//Sensor data online post endpoint
#define HTTPS_TEMP_ENDPOINT "https://www.example.com/temperaturesubmit.php"


/*
 * WiFi networks
 */

//Number of available WiFi networks: update the value when adding or removing networks from the WIFI_ACCESSPOINTS array
#define WIFI_ACCESSPOINTS_COUNT 2

//Known WiFi networks
const char PROGMEM *WIFI_ACCESSPOINTS[][2] = {
  { "SSID Network 1", "foobar" }, //WiFi network #1
  { "SSID Network 2", "barfoo" }, //WiFi network #2
};

//Timeout (in seconds) before to give up when trying to connect to a WiFi network
#define WIFI_CONNECTION_TIMEOUT 15