/**
 * @package Temperature Monitor
 * @author WizLab.it
 * @board Generic ESP8266
 * @version 20250312.125
 */

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
#include <Wire.h>
#include <OneWire.h>
#include <Adafruit_SSD1306.h>
#include <DallasTemperature.h>
#include "TemperatureMonitor-DS18B20.h"


/**
 * System Parameters
 */

//DS18B20 Temperature sensor
#define SENSOR_ID_SELECTOR_PIN_1 12 //GPIO12, D6
#define SENSOR_ID_SELECTOR_PIN_2 13 //GPIO13, D7
#define SENSOR_TEMP_SAMPLE_PIN 2 //GPIO2, D4

//Low-battery
#define LOWBATTERY_PIN            A0
#define LOWBATTERY_LEVEL          3.6

//LED
#define LED_PIN 2 //GPIO2, D4
#define LED_ON LOW //Led is on when the pin is low
#define LED_OFF HIGH //Led is off when the pin is high

//OLED
#define OLED_ADDRESS 0x3C //OLED I2C Address
#define OLED_WIDTH 128 //OLED display width, in pixels
#define OLED_HEIGHT 32 //OLED display height, in pixels


/**
 * Structs
 */

//Temperature data structure
typedef struct {
  DeviceAddress address;
  char addressString[17];
  uint8_t resolution = 0;
  int16_t raw = 0;
  float celsius = 0.0;
  float batteryVoltage = 0.0;
} TemperatureData;


/**
 * Global Variables
 */

int8_t SENSOR_ID = 0;
int8_t WIFI_ACCESSPOINT_ID = -1;
bool WIFI_STATUS = false;
Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);
OneWire oneWire(SENSOR_TEMP_SAMPLE_PIN);
DallasTemperature ds18b20(&oneWire);


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
  Serial.begin(115200);
  Serial.println(F("\n\n\n\n\n"));
  Serial.println(F("************************************************************************"));
  Serial.println(F("***             ~  Temperature Monitor - by WizLab.it  ~             ***"));
  Serial.println(F("************************************************************************"));
  Serial.println(F("[~~~~~] Setup:"));

  //Low battery pin
  pinMode(LOWBATTERY_PIN, INPUT);

  //DS18B20 temperature sensor
  ds18b20.begin();

  //Sensor ID
  pinMode(SENSOR_ID_SELECTOR_PIN_1, INPUT_PULLUP);
  pinMode(SENSOR_ID_SELECTOR_PIN_2, INPUT_PULLUP);
  SENSOR_ID = digitalRead(SENSOR_ID_SELECTOR_PIN_1) | (digitalRead(SENSOR_ID_SELECTOR_PIN_2) << 1);
  Serial.printf(" [i] Sensor ID: %d\n", SENSOR_ID);

  //Led
  if(USE_LED) {
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LED_OFF);
  }

  //OLED
  if(display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    Serial.println(F(" [+] OLED activated"));
  } else {
    Serial.println(F(" [-] OLED init failed"));
  }
  //Splash screen
  display.clearDisplay();
  delay(250);
  display.cp437(true); // Use full 256 char 'Code Page 437'
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(35, 0);
  display.print(F("Temperature"));
  display.setCursor(48, 10);
  display.print(F("monitor"));
  display.setCursor(32, 24);
  display.print(F("by WizLab.it"));
  display.display();
  delay(2000);

  //Standard interface
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 2);
  display.print(F("Temperature:"));

  //Setup completed
  Serial.println(F("[~~~~~] Setup complete."));
  Serial.println(F("[~~~~~] Running:"));
}

/**
 * Loop
 */
void loop() {
  //Read temperature
  TemperatureData temperature;
  readTemperature(&temperature);

  //Print temperature on OLED
  display.fillRect(0, 16, 98, 16, BLACK);
  display.setTextSize(2);
  display.setCursor(0, 16);
  display.printf("%4.1f %cC", temperature.celsius, (char)248);
  display.display();

  //Check if battery is low
  long batteryVoltageRaw = analogRead(LOWBATTERY_PIN);
  temperature.batteryVoltage = (map(batteryVoltageRaw, 0, 1023, 0, 3100) / 1000.0) * 2.0; //Map is 0-1023 to 0-3.1V; voltage is doubled because of the voltage divider
  printBitmap(BITMAP_LOW_BATTERY, BITMAP_LOW_BATTERY_META, ((temperature.batteryVoltage < LOWBATTERY_LEVEL) ? true : false));
  Serial.printf(" [i] Battery voltage: %0.3f V\n", temperature.batteryVoltage);

  //Post temperature online (if required)
  if(POST_TEMPERATURE_ONLINE) {
    WIFI_STATUS = wifiConnect();

    //If connected to WiFi, then show WiFi icon and post temperature
    if(WIFI_STATUS) {
      printBitmap(BITMAP_WIFI, BITMAP_WIFI_META, true); //WiFi icon
      postTemperature(&temperature); //Post temperature online
    }
  }

  //Sleep
  if(USE_DEEP_SLEEP) {
    //Turn off WiFi
    if(WIFI_STATUS) {
      WiFi.mode(WIFI_OFF);
      WIFI_STATUS = false;
      Serial.printf(" [-] WiFi disabled.\n");
      printBitmap(BITMAP_WIFI, BITMAP_WIFI_META, false);
    }

    //Go to deep sleep
    Serial.printf("[~~~~~] Going to low power for %ds.\n", SLEEP_DURATION);
    ESP.deepSleep(SLEEP_DURATION * 1000000); //deepSleep is in microseconds
  } else {
    Serial.printf("[~~~~~] Delay %ds.\n", SLEEP_DURATION);
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
    Serial.printf(" [*] Connecting to %s: ", WIFI_ACCESSPOINTS[WIFI_ACCESSPOINT_ID][0]);
    uint8_t i = 0;
    while(WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
      if(i++ > WIFI_CONNECTION_TIMEOUT) break;
      delay(1000);
    }

    //Check connection status
    if(WiFi.status() == WL_CONNECTED) {
      Serial.printf(" OK (%s)\n", WiFi.localIP().toString().c_str());
      if(USE_LED) blinkLed(5);
      return true;
    } else {
      Serial.println(" failed");
      if(USE_LED) blinkLed(2);
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
  //Start temperature conversion on DS18B20
  ds18b20.requestTemperatures();

  //Get DS18B20 address and resolution
  ds18b20.getAddress(temperature->address, 0);
  sprintf(temperature->addressString, "%X%X%X%X%X%X%X%X\0", temperature->address[0], temperature->address[1], temperature->address[2], temperature->address[3], temperature->address[4], temperature->address[5], temperature->address[6], temperature->address[7]);
  temperature->resolution = ds18b20.getResolution(temperature->address);

  //Get temperature
  temperature->raw = ds18b20.getTemp(temperature->address);
  temperature->celsius = ds18b20.getTempC(temperature->address);

  //Print data
  Serial.printf(" [i] DS18B20 address: %s\n", temperature->addressString);
  Serial.printf(" [i] Resolution: %d\n", temperature->resolution);
  Serial.printf(" [i] Raw Temperature: %d\n", temperature->raw);
  Serial.printf(" [i] Temperature: %0.3f °C\n", temperature->celsius);
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
      "\"sensorId\":\"" + String(SENSOR_ID) + "\"," +
      "\"sensorType\":\"DS18B20\"," +
      "\"wifi\":\"" + String(WIFI_ACCESSPOINTS[WIFI_ACCESSPOINT_ID][0]) + "\"," +
      "\"batteryVoltage\":\"" + String(temperature->batteryVoltage) + "\"," +
      "\"lowbattery\":" + ((temperature->batteryVoltage < LOWBATTERY_LEVEL) ? "true" : "false") + "," +
      "\"temperature\":{" +
        "\"address\":\"" + String(temperature->addressString) + "\"," +
        "\"resolution\":\"" + String(temperature->resolution) + "\"," +
        "\"raw\":\"" + String(temperature->raw) + "\"," +
        "\"celsius\":\"" + String(temperature->celsius) + "\"" +
      "}" +
    "}";

    //Send request
    int httpCode = httpsClient.POST(httpPayload);
    httpsClient.end();

    if(httpCode == HTTP_CODE_OK) {
      //Serial.printf("HTTP raw payload (%s): %s\n", WIFI_ACCESSPOINTS[WIFI_ACCESSPOINT_ID][0], httpPayload.c_str());
      Serial.printf(" [+] Temperature posted online successfully: %0.2f °C\n", temperature->celsius);
      if(USE_LED) blinkLed(10);
      return true;
    }
  }

  Serial.println(" [-] Error posting temperature online");
  return false;
}

/**
 * Blink led
 */
void blinkLed(uint8_t numberOfFlash) {
  for(uint8_t i=0; i<numberOfFlash; i++) {
    digitalWrite(LED_PIN, LED_ON);
    delay(20);
    digitalWrite(LED_PIN, LED_OFF);
    delay(250);
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