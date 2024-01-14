# Temperature Monitor

This device can be used to monitor the environmental temperature, and optionally post the values online for remote monitoring or show them on a OLED display in real time.

The circuit is based on the ESP8266 board (Arduino like), that handles the temperature sampling (A/D conversion), network communication via WiFi, and drives the OLED display.

Server side code (PHP) is also provided, to receive the data from the board and show it in a chart.

The device supports:

- multiple WiFi networks: it loops among them trying to connect to any of them

- a trimmer is used for fine tuning

- two sensor IDs supported (selected via a switch)

- optional alerts can be sent via Telegram when the temperature rises above or falls below the allowed range (when online data post is enabled)

The device is battery powered (18650 Li-ion 3.7V battery), but can be powered via any standard 5V USB battery charger, just connecting it to the ESP8266 board.



### Credits

WizLab.it



### References

WizLab.it: https://www.wizlab.it/

GitHub: https://github.com/wizlab-it/temperature-monitor/



### License

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

To get a copy of the GNU General Public License, see [http://www.gnu.org/licenses/](http://www.gnu.org/licenses/).
