/**
 * @package Temperature Monitor
 * @author WizLab.it
 * @version 20240125.091
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
#define SENSOR_TEMP_SAMPLE_PIN A0
#define SENSOR_TEMP_POWER_PIN 13 //GPIO13, D7
#define SENSOR_TEMP_SAMPLES 5 //Multiple samples, then calculates average to increase precision

//Low battery sensor
#define LOWBATTERY_PIN 14 //GPIO14, D5

//LED
#define LED_PIN 2 //GPIO2, D4
#define LED_ON LOW //Led is on when the pin is low
#define LED_OFF HIGH //Led is off when the pin is high

//OLED
#define OLED_POWER_PIN 15 //GPIO15, D8
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
bool WIFI_STATUS = false;
Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);


/**
 * Bitmaps
 */

//Low Battery
static const unsigned char BITMAP_LOW_BATTERY[] = { 0xff, 0xfc, 0x80, 0x04, 0x80, 0x07, 0x80, 0x05, 0x80, 0x05, 0x80, 0x07, 0x80, 0x04, 0xff, 0xfc };
static const uint8_t BITMAP_LOW_BATTERY_META[] = { (OLED_WIDTH - 19), (OLED_HEIGHT - 10), 16, 8 }; //x, y, w, h

//WiFi
static const unsigned char BITMAP_WIFI[] = {
	0x07, 0xc0, 0x3f, 0xf8, 0xf8, 0x3e, 0xc0, 0x06, 0x8f, 0xe2, 0x3f, 0xf8, 0x78, 0x3c, 0x20, 0x08,
	0x07, 0xe0, 0x0f, 0xe0, 0x00, 0x00, 0x01, 0x00, 0x03, 0x80, 0x07, 0xc0, 0x03, 0x80, 0x01, 0x00
};
static const uint8_t BITMAP_WIFI_META[] = { (OLED_WIDTH - 19), 2, 15, 16 }; //x, y, w, h


//===================================================================================


/**
 * Setup
 */
void setup() {
  if(DEBUG) Serial.begin(115200);

  //Sensor pin
  pinMode(SENSOR_TEMP_SAMPLE_PIN, INPUT);

  //Low battery pin
  pinMode(LOWBATTERY_PIN, INPUT);

  //Sensor power IO
  pinMode(SENSOR_TEMP_POWER_PIN, OUTPUT);

  //Sensor ID
  pinMode(SENSOR_ID_SELECTOR_PIN, INPUT_PULLUP);
  if(digitalRead(SENSOR_ID_SELECTOR_PIN) == HIGH) {
    SENSOR_ID = SENSOR_ID_HIGH;
  } else {
    SENSOR_ID = SENSOR_ID_LOW;
  }

  //Led
  if(LED_ACTIVE) {
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LED_OFF);
  }

  //OLED
  if(OLED_ACTIVE) {
    pinMode(OLED_POWER_PIN, OUTPUT);
    digitalWrite(OLED_POWER_PIN, HIGH);
    delay(500);
    if(display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
      OLED_IS_ACTIVE = true;
      if(DEBUG) Serial.println("OLED activated");
    } else {
      if(DEBUG) Serial.println("OLED init failed");
    }
    if(OLED_IS_ACTIVE) {
      //Splash screen
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

      //Standard interface
      display.clearDisplay();
      display.setTextSize(1);
      display.setCursor(0, 2);
      display.print("Temperature:");
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

  //If OLED is active, use it
  if(OLED_IS_ACTIVE) {
    //Print temperature
    display.fillRect(0, 16, 98, 16, BLACK);
    display.setTextSize(2);
    display.setCursor(0, 16);
    display.printf("%4.1f %cC", temperature.celsius, (char)248);
    display.display();

    //Check if battery is low
    printBitmap(BITMAP_LOW_BATTERY, BITMAP_LOW_BATTERY_META, ((digitalRead(LOWBATTERY_PIN) == HIGH) ? true : false));
  }

  //Post temperature online (if required)
  if(POST_TEMPERATURE_ONLINE) {
    WIFI_STATUS = wifiConnect();

    //If connected to WiFi, then show WiFi icon and post temperature
    if(WIFI_STATUS) {

      //Print WiFi icon
      if(OLED_IS_ACTIVE) {
        printBitmap(BITMAP_WIFI, BITMAP_WIFI_META, true);
      }

      //Post temperature
      postTemperature(&temperature);
    }
  }

  //Sleep
  if(USE_DEEP_SLEEP) {

    //Turn off WiFi
    if(WIFI_STATUS) {
      WiFi.mode(WIFI_OFF);
      WIFI_STATUS = false;
      if(DEBUG) Serial.printf("WiFi disabled.\n");
      if(OLED_IS_ACTIVE) {
        printBitmap(BITMAP_WIFI, BITMAP_WIFI_META, false);
      }
    }

    //If oled is active, then wait some time before to go to deep sleep
    if(OLED_IS_ACTIVE) {
      delay(2500);
    }

    //Go to deep sleep
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
    temp_analog[i] = analogRead(SENSOR_TEMP_SAMPLE_PIN);
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
    String httpPayload = String("{") +
      "\"sensorId\":\"" + SENSOR_ID + "\"," +
      "\"wifi\":\"" + String(WIFI_ACCESSPOINTS[WIFI_ACCESSPOINT_ID][0]) + "\"," +
      "\"lowbattery\":" + ((digitalRead(LOWBATTERY_PIN) == HIGH) ? "true" : "false") + "," +
      "\"temperature\":{" +
        "\"analog\":\"" + String(temperature->analog) + "\"," +
        "\"voltage\":\"" + String(temperature->voltage) + "\"," +
        "\"celsius\":\"" + String(temperature->celsius) + "\"" +
      "}" +
    "}";

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
    digitalWrite(LED_PIN, LED_ON);
    delay(10);
    digitalWrite(LED_PIN, LED_OFF);
    delay(100);
  }
}

/**
 * Display icon
 */
void printBitmap(const unsigned char icon[], const uint8_t iconMeta[], bool active) {
  if(active == true) {
    display.drawBitmap(iconMeta[0], iconMeta[1], icon, iconMeta[2], iconMeta[3], WHITE);
  } else {
    display.fillRect(iconMeta[0], iconMeta[1], iconMeta[2], iconMeta[3], BLACK);
  }
  display.display();
}