# Temperature Monitor

The device can be used to monitor the environmental temperature, and optionally post the values online for remote monitoring and/or show them on a OLED display in real time.

The circuit is based on a temperature sensor (NTC or DS18B20) and the ESP8266 board (Arduino like), that handles the temperature sampling, network communication via WiFi, and drives the OLED display.

The server-side code (PHP) is also provided, to receive the data from the board and show it in a chart (via Google charts).

The device is battery powered (18650 Li-ion 3.7V battery), but can be powered via any standard 5V USB battery charger, just connecting it to the ESP8266 board.



## Main features

- Multiple WiFi networks support: it loops through all of them, connecting to the first available

- Fine tuning via trimmer (NTC version)

- Four sensor IDs supported, selectable via a 2-bit dip-switch (Sensor ID from 0 to 3)

- Optional alerts via Telegram when the temperature rises above or falls below the defined range (when online data post is enabled)

- Low-battery level detection (notification shown online on the chart or on the OLED display)



## Configuration

A sample configuration file is provided (*TemperatureMonitor-Sample.h*): it must be renamed as *TemperatureMonitor.h* and parameters changed based on the needs.

| Parameter | Description |
| --------- | ----------- |
| **SENSOR_TEMP_BETA** | NTC sensor beta value (only for NTC version) |
| **SENSOR_TEMP_NOMINAL_OHM** | NTC sensor nominal value (only for NTC version) |
| **SENSOR_TEMP_NOMINAL_TEMP** | NTC sensor nominal temperature (only for NTC version) |
| **SENSOR_TEMP_SERIES_OHM** | value of the resistor in series with the NTC sensor (only for NTC version) |
| **USE_LED** | if true, the build-in ESP8266 led is used to inform about events (2 blinks: connection to WiFi network failed; 5 blinks: connection to WiFi network succeeded; 10 blinks: sersor data posted online). Set to false to save battery |
| **SLEEP_DURATION** | how long to sleep between samples (in seconds) |
| **USE_DEEP_SLEEP** | if true, deep-sleep (extremely low current) is used. To use deep-sleep, the 1-bit dip-switch on ESP8266's **D0** and **RST** pins must be closed. If false, a normal delay is used |
| **POST_TEMPERATURE_ONLINE** | if true, temperature is posted online |
| **HTTPS_TEMP_ENDPOINT** | endpoint where to post temperatures |
| **WIFI_ACCESSPOINTS_COUNT** | number of configured Access Points (must match the number of WIFI_ACCESSPOINTS elements) |
| **WIFI_ACCESSPOINTS** *(array)* | list of Access Points |
| **WIFI_CONNECTION_TIMEOUT** | timeount (in seconds) after which to give up when trying to connect to a WiFi network |



### Deep-Sleep notes

The device can be put in deep-sleep status to save battery. When in deep-sleep, the current usage is extremely low and battery can last weeks or months.
To wake up from deep-sleep, the ESP8266's **D0** and **RST** pins must be connected (1-bit SW2 dip-switch must be closed).
When flashing the firmware on the ESP8266 module, D0 and RST pin **MUST NOT** be connect (1-bit SW2 dip-switch must be open), or the code upload will fail.



## Temperature monitor sections


#### OLED display

The device can use an SSD1306 128x32 pixels OLED display.

When present, the OLED is used to show the temperature, the Wi-Fi status (icon) and the low-battery (icon) notification.


#### Low-battery detection

Low battery detection is achieved via R4-R5, R6-R7 and IC4-A.

R4-R5 voltage divider halves the 3.3V power supplies (always constant, via the MCP1700-3300 LDO voltage regulator), applying about 1.65V on the non-inverting input of the IC4-A operational amplifier.

R6-R7 voltage divider divides the voltage from the battery by about 2.2; this voltage is applied on the IC4-A inverting input.
When the battery is full it provides about 4.2V, with 1.91V on the IC4-A inverting input. As soon as the battery gets discharged, the voltage on the IC4-A inverting input drops, falling below the voltage on the non-inverting input when the battery voltage is below 3.63V.

When the voltage on the non-inverting input gets higher than the voltage on the inverting input, the IC4-A operational amplifier output level rises to the maximum value allowed (about 1.95V when powered with 3.3V, see LM358 specifications).
This voltage is detected as a high logic level by the ESP8266, setting the low-battery level flag.


#### Server-side application

The server-side application needs PHP to run the code, and a MySQL/MariaDB database to store data.

A sample configuration file is provided (*config-sample.php*): it must be renamed as *config.php* and parameters changed based on the needs.

The database structure is included in the sample configuration file.

Sensors name and alert levels can be customized. If alerts are set (at least for one sensor), a Telegram bot is required to send the notification.
Refer to [https://www.shellhacks.com/telegram-api-send-message-personal-notification-bot/](https://www.shellhacks.com/telegram-api-send-message-personal-notification-bot/) (or any other tutorial available online) for the Telegram bot creation.



## Credits and references

Created by [WizLab.it](https://www.wizlab.it/)

GitHub: [https://github.com/wizlab-it/temperature-monitor/](https://github.com/wizlab-it/temperature-monitor/)

[ESP8266 Documentation](https://arduino-esp8266.readthedocs.io/)



## License

This program is free software: you can redistribute it and/or modify it under the terms of the GNU General Public License as published by the Free Software Foundation, either version 3 of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

To get a copy of the GNU General Public License, see [http://www.gnu.org/licenses/](http://www.gnu.org/licenses/)
