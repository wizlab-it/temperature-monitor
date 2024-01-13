/**
 * @package Temperature Monitor
 * Configuration sample
 * @author WizLab.it
 * @version 20240113.006
 */


/*
 * Generic behaviour
 */

//If debug is active, the sensor name is set to DEBUG_1 or DEBUG_2, and logs are sent to the Serial Monitor
#define DEBUG true

//Use built-in led to inform about events:
// 2 blinks: connection to WiFi network faled
// 5 blinks: connection to WiFi network succeeded
// 10 blinks: sersor data posted online successfully
#define LED_ACTIVE true

//Use OLED display
#define OLED_ACTIVE true

//Seconds between samples
#define SLEEP_DURATION 10

//If true, go to deep sleep (extremely low current) between samples
//To use deep sleep, D0 and RST pins on ESP8266 must be connected
#define USE_DEEP_SLEEP false

//If true, post the sensor data online
#define POST_TEMPERATURE_ONLINE false


/*
 * Sensors
 */
#if(DEBUG == true)
  #define SENSOR_NAME_HIGH "TEMP_DEBUG_1"
  #define SENSOR_NAME_LOW "TEMP_DEBUG_2"
#else
  #define SENSOR_NAME_HIGH "TEMP_SENSOR_1"
  #define SENSOR_NAME_LOW "TEMP_SENSOR_2"
#endif


/*
 * WiFi networks
 */

//Number of available WiFi networks: update the value when adding or removing networks in the WIFI_ACCESSPOINTS array
#define WIFI_ACCESSPOINTS_COUNT 2

//Known WiFi networks
const char *WIFI_ACCESSPOINTS[][2] = {
  { "SSID Network 1", "foobar" }, //WiFi network #1
  { "SSID Network 2", "barfoo" }, //WiFi network #2
};

//Timeout (in seconds) before to give up when trying to connect to a WiFi network
#define WIFI_CONNECTION_TIMEOUT 15


/*
 * HTTPS
 */

//Sensor data online post endpoint
#define HTTPS_TEMP_ENDPOINT "https://www.example.com/temperaturesubmit.php"