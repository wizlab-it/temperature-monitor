<?php
/**
 * @package Temperature Monitor
 * Configuration sample
 * @author WizLab.it
 * @version 20240113.001
 */

//Database configuration
define("DB_HOST", "localhost");
define("DB_USER", "user");
define("DB_PASS", "pass");
define("DB_DB", "db");

//Sensor names (IDs must match the IDs on the ESP8266 board)
define("SENSORS", [
  "TEMP_SENSOR_1" => [ "name"=>"Sensor #1", "position"=>1, "color"=>"F50057" ],
  "TEMP_SENSOR_2" => [ "name"=>"Sensor #2", "position"=>2, "color"=>"2979FF" ],
  "TEMP_DEBUG_1" => [ "name"=>"Debug #1", "position"=>3, "color"=>"00E676" ],
  "TEMP_DEBUG_2" => [ "name"=>"Debug #2", "position"=>4, "color"=>"EAEA1B" ],
]);
