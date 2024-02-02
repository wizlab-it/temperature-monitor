<?php
/**
 * @package Temperature Monitor
 * @author WizLab.it
 * @version 20240131.011
 */


/*--- Load configuration --------------------------------------------------*/
require_once("config.php");


/*--- Database connection -------------------------------------------------*/
$DBL = new mysqli(DB_HOST, DB_USER, DB_PASS, DB_DB);
$DBL->query("SET CHARACTER SET utf8");
$DBL->query("SET NAMES utf8");


/*--- Process payload -----------------------------------------------------*/
$payload = file_get_contents("php://input");
$payloadParsed = json_decode($payload);
if(!$payloadParsed || !is_object($payloadParsed) || !is_numeric($payloadParsed->sensorId) || !$payloadParsed->temperature || !is_object($payloadParsed->temperature) || !is_numeric($payloadParsed->temperature->celsius)) die("OK");

//Save data
$DBL->query("INSERT INTO temperatures SET date=NOW(), sensorId='" . dbEsc($payloadParsed->sensorId) . "', temperature='" . dbEsc($payloadParsed->temperature->celsius) . "', lowBattery='" . (($payloadParsed->lowbattery) ? "1" : "0") . "', rawPayload='" . dbEsc($payload) . "'");

//Check if to send alert
if(SENSORS[$payloadParsed->sensorId]["alert"]) {
  $alertDetails = "Date: " . date("Y-m-d H:i:s") . "\n" .
    "Sensor ID: " . SENSORS[$payloadParsed->sensorId]["name"] . " (" . $payloadParsed->sensorId . ")\n" .
    "Temperature: " . $payloadParsed->temperature->celsius . " C\n" .
    "Range: " . SENSORS[$payloadParsed->sensorId]["alert"]["min"] . "-" . SENSORS[$payloadParsed->sensorId]["alert"]["max"] . " C";
  if($payloadParsed->temperature->celsius > SENSORS[$payloadParsed->sensorId]["alert"]["max"]) {
    sendTelegram("<b>HIGH Temperature Warning!</b>\n" . $alertDetails);
  }
  if($payloadParsed->temperature->celsius < SENSORS[$payloadParsed->sensorId]["alert"]["min"]) {
    sendTelegram("<b>LOW Temperature Warning!</b>\n" . $alertDetails);
  }
}

//End
die("OK");


//===========================================================================


/*--- Functions -----------------------------------------------------------*/
function dbEsc($string) {
  return $GLOBALS["DBL"]->real_escape_string($string);
}

function sendTelegram($text) {
  $command = "sendMessage?chat_id=" . TELEGRAM_CHAT_ID . "&parse_mode=HTML&text=" . urlencode($text);
  file_get_contents("https://api.telegram.org/bot" . TELEGRAM_BOT_TOKEN . "/" . $command);
}
