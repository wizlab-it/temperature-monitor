<?php
/**
 * @package Temperature Monitor
 * Configuration sample
 * @author WizLab.it
 * @version 20240126.003
 */

/*
 * Database structure
 *
 * CREATE TABLE `temperatures` (
 *   `id` int(10) UNSIGNED AUTO_INCREMENT PRIMARY KEY NOT NULL,
 *   `date` datetime NOT NULL,
 *   `sensorId` varchar(30) NOT NULL,
 *   `temperature` decimal(5,2) NOT NULL,
 *   `lowBattery` tinyint(1) NOT NULL,
 *   `rawPayload` text NOT NULL
 * ) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4;
 *
 */

//Database configuration
define("DB_HOST", "localhost");
define("DB_USER", "user");
define("DB_PASS", "pass");
define("DB_DB", "db");

//Sensor names (IDs must match the IDs on the ESP8266 board)
define("SENSORS", [
  "TEMP_SENSOR_1" => [ "name"=>"Sensor #1", "position"=>1, "color"=>"F50057", "alert"=>["min"=>7, "max"=>11] ],
  "TEMP_SENSOR_2" => [ "name"=>"Sensor #2", "position"=>2, "color"=>"2979FF", "alert"=>["min"=>12, "max"=>18] ],
  "TEMP_DEBUG_1" => [ "name"=>"Debug #1", "position"=>3, "color"=>"00E676" ],
  "TEMP_DEBUG_2" => [ "name"=>"Debug #2", "position"=>4, "color"=>"EAEA1B" ],
]);

//Telegram (https://www.shellhacks.com/telegram-api-send-message-personal-notification-bot/)
define("TELEGRAM_BOT_TOKEN", "xxx");
define("TELEGRAM_CHAT_ID", "123");
