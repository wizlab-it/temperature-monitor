<?php
/**
 * @package Temperature Monitor
 * @author WizLab.it
 * @version 20240113.004
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
if(!$payloadParsed || !is_object($payloadParsed) || !$payloadParsed->sensorId || !$payloadParsed->temperature || !is_object($payloadParsed->temperature) || !is_numeric($payloadParsed->temperature->celsius)) die("OK");
$DBL->query("INSERT INTO temperatures SET date=NOW(), sensorId='" . dbEsc($payloadParsed->sensorId) . "', temperature='" . dbEsc($payloadParsed->temperature->celsius) . "', rawPayload='" . dbEsc($payload) . "'");
die("OK");


//===========================================================================


/*--- Functions -----------------------------------------------------------*/
function dbEsc($string) {
  return $GLOBALS["DBL"]->real_escape_string($string);
}
