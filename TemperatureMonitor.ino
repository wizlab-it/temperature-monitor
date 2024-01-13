/**
 * @package Temperature Monitor
 * @author WizLab.it
 * @version 20240113.074
 */

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "TemperatureMonitor.h"


/**
 * System Parameters
 */

//Temperature sensor
#define SENSOR_ID_SELECTOR_PIN 12 //GPIO12, D6
#define SENSOR_TEMP_PIN A0
#define SENSOR_TEMP_POWER_PIN 13 //GPIO13, D7
#define SENSOR_TEMP_BETA 4050.0 //3950
#define SENSOR_TEMP_NOMINAL_OHM 10000.0
#define SENSOR_TEMP_NOMINAL_TEMP 25.0
#define SENSOR_TEMP_SERIES_OHM 10000.0
#define SENSOR_TEMP_SAMPLES 5

//LED
#define LED_PIN 2 //GPIO2, D4

//OLED
#define OLED_ADDRESS 0x3C //OLED I2C Address
#define OLED_WIDTH 128 //OLED display width, in pixels
#define OLED_HEIGHT 32 //OLED display height, in pixels


/**
 * Structs
 */

//Temperature data structure
typedef struct {
  float analog = 0.0;
  float voltage = 0.0;
  float celsius = 0.0;
} TemperatureData;


/**
 * Global Variables
 */

String SENSOR_ID;
int8_t WIFI_ACCESSPOINT_ID = -1;
bool OLED_IS_ACTIVE = false;
Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);


//===================================================================================


/**
 * Setup
 */
void setup() {
  if(DEBUG) Serial.begin(115200);

  //Sensor pin
  pinMode(SENSOR_TEMP_PIN, INPUT);

  //Sensor power IO
  pinMode(SENSOR_TEMP_POWER_PIN, OUTPUT);

  //Sensor ID
  pinMode(SENSOR_ID_SELECTOR_PIN, INPUT_PULLUP);
  if(digitalRead(SENSOR_ID_SELECTOR_PIN) == HIGH) {
    SENSOR_ID = SENSOR_NAME_HIGH;
  } else {
    SENSOR_ID = SENSOR_NAME_LOW;
  }

  //Led
  if(LED_ACTIVE) {
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH); //Led is off when the output is high
  }

  //OLED
  if(OLED_ACTIVE) {
    if(display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
      OLED_IS_ACTIVE = true;
      if(DEBUG) Serial.println("OLED activated");
    } else {
      if(DEBUG) Serial.println("OLED init failed");
    }
    if(OLED_IS_ACTIVE) {
      display.clearDisplay();
      delay(250);
      display.cp437(true); // Use full 256 char 'Code Page 437'
      display.clearDisplay();
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.setCursor(35, 0);
      display.print("Temperature");
      display.setCursor(48, 10);
      display.print("monitor");
      display.setCursor(32, 24);
      display.print("by WizLab.it");
      display.display();
      delay(5000);
    }
  }
}

/**
 * Loop
 */
void loop() {
  if(DEBUG) Serial.print("============================================================================================================\n");

  //Read temperature
  TemperatureData temperature;
  readTemperature(&temperature);

  //If OLED is active, print temperature
  if(OLED_IS_ACTIVE) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 2);
    display.print("Temperature:");
    display.setTextSize(2);
    display.setCursor(0, 16);
    display.printf("%0.1f %cC", temperature.celsius, (char)248);
    display.display();
  }

  //Post temperature online (if required)
  if(POST_TEMPERATURE_ONLINE) {
    bool wifiStatus = wifiConnect();

    //If connected to WiFi, then post temperature
    if(wifiStatus) {
      postTemperature(&temperature);
    }
  }

  //Sleep
  if(USE_DEEP_SLEEP) {
    if(DEBUG) Serial.printf("Going to low power for %ds.\n", SLEEP_DURATION);
    ESP.deepSleep(SLEEP_DURATION * 1000000); //deepSleep is in microseconds
  } else {
    if(DEBUG) Serial.printf("Delay %ds.\n", SLEEP_DURATION);
    delay(SLEEP_DURATION * 1000); //delay is in milliseconds
  }
}


//===================================================================================


/**
 * WiFi Connection
 */
bool wifiConnect() {
  if((WIFI_ACCESSPOINT_ID >= 0) && (WIFI_ACCESSPOINT_ID < WIFI_ACCESSPOINTS_COUNT)) {
    //If already connected, then exists
    if(WiFi.status() == WL_CONNECTED) return true;
  }

  //If here, it's not connected: reset Access Point ID
  WIFI_ACCESSPOINT_ID = -1;
  WiFi.mode(WIFI_STA);

  //Loop among available Access Points
  while(++WIFI_ACCESSPOINT_ID < WIFI_ACCESSPOINTS_COUNT) {
    //Configure WiFi
    WiFi.disconnect();
    delay(1000);
    WiFi.begin(WIFI_ACCESSPOINTS[WIFI_ACCESSPOINT_ID][0], WIFI_ACCESSPOINTS[WIFI_ACCESSPOINT_ID][1]);

    //Try to connect
    if(DEBUG) Serial.printf("Connecting to %s: ", WIFI_ACCESSPOINTS[WIFI_ACCESSPOINT_ID][0]);
    uint8_t i = 0;
    while(WiFi.status() != WL_CONNECTED) {
      if(DEBUG) Serial.print(".");
      if(i++ > WIFI_CONNECTION_TIMEOUT) break;
      delay(1000);
    }

    //Check connection status
    if(WiFi.status() == WL_CONNECTED) {
      if(DEBUG) Serial.printf(" OK (%s)\n", WiFi.localIP().toString().c_str());
      if(LED_ACTIVE) blinkLed(5);
      return true;
    } else {
      if(DEBUG) Serial.println(" FAILED");
      if(LED_ACTIVE) blinkLed(2);
      delay(1000);
    }
  }

  //If here, connection failed
  return false;
}

/**
 * Read temperature
 */
void readTemperature(TemperatureData* temperature) {
  //Power on sensor
  digitalWrite(SENSOR_TEMP_POWER_PIN, HIGH);
  delay(1000);

  //Get ADC temperature samples
  int temp_analog[SENSOR_TEMP_SAMPLES];
  for(uint8_t i=0; i<SENSOR_TEMP_SAMPLES; i++) {
    temp_analog[i] = analogRead(SENSOR_TEMP_PIN);
    delay(50);
  }

  //Power off sensor
  digitalWrite(SENSOR_TEMP_POWER_PIN, LOW);

  //Calculate temperature voltage based on samples average
  for(uint8_t i=0; i<SENSOR_TEMP_SAMPLES; i++) {
    temperature->analog += temp_analog[i];
  }
  temperature->analog /= SENSOR_TEMP_SAMPLES;
  temperature->voltage = temperature->analog * 3.3 / 1024.0;

  //Convert analog to resistance
  float temp_ohm = 1023.0 / temperature->analog - 1.0;
  temp_ohm = SENSOR_TEMP_SERIES_OHM / temp_ohm;

  //Calculate temperature
  temperature->celsius = temp_ohm / SENSOR_TEMP_NOMINAL_OHM;
  temperature->celsius = log(temperature->celsius);
  temperature->celsius /= SENSOR_TEMP_BETA;
  temperature->celsius += 1.0 / (SENSOR_TEMP_NOMINAL_TEMP + 273.15);
  temperature->celsius = 1.0 / temperature->celsius;
  temperature->celsius -= 273.15;

  if(DEBUG) Serial.printf("Analog: %0.2f - Voltage: %0.3f V - Temperature: %0.3f °C\n", temperature->analog, temperature->voltage, temperature->celsius);
}

/**
 * Post temperature
 */
bool postTemperature(TemperatureData* temperature) {
  //WiFi Client, no certificate check
  std::unique_ptr<BearSSL::WiFiClientSecure>wifiClient(new BearSSL::WiFiClientSecure);
  wifiClient->setInsecure();

  //HTTP Client
  HTTPClient httpsClient;
  if(httpsClient.begin(*wifiClient, HTTPS_TEMP_ENDPOINT)) {
    httpsClient.addHeader("Content-Type", "application/json");

    //Build payload
    String httpPayload = "{\"sensorId\":\"" + SENSOR_ID + "\",\"temperature\":{" +
      "\"wifi\":\"" + String(WIFI_ACCESSPOINTS[WIFI_ACCESSPOINT_ID][0]) + "\"," +
      "\"analog\":\"" + String(temperature->analog) + "\"," +
      "\"voltage\":\"" + String(temperature->voltage) + "\"," +
      "\"celsius\":\"" + String(temperature->celsius) + "\"" +
    "}}";

    //Send request
    int httpCode = httpsClient.POST(httpPayload);
    httpsClient.end();

    if(httpCode == HTTP_CODE_OK) {
      if(DEBUG) Serial.printf("HTTP raw payload (%s): %s\n", WIFI_ACCESSPOINTS[WIFI_ACCESSPOINT_ID][0], httpPayload.c_str());
      if(LED_ACTIVE) blinkLed(10);
      return true;
    }
  }

  return false;
}

/**
 * Blink led
 */
void blinkLed(uint8_t numberOfFlash) {
  for(uint8_t i=0; i<numberOfFlash; i++) {
    digitalWrite(LED_PIN, LOW);
    delay(10);
    digitalWrite(LED_PIN, HIGH);
    delay(100);
  }
}