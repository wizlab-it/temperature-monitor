<?php
/**
 * @package Temperature Monitor
 * @author WizLab.it
 * @version 20241106.021
 */


/*--- Load configuration --------------------------------------------------*/
require_once("config.php");


/*--- Database connection -------------------------------------------------*/
$DBL = new mysqli(DB_HOST, DB_USER, DB_PASS, DB_DB);
$DBL->query("SET CHARACTER SET utf8");
$DBL->query("SET NAMES utf8");


/*--- Open HTML page ------------------------------------------------------*/
echo("<html>
<head>
  <title>Temperature Monitor</title>
  <meta http-equiv='Content-Type' content='text/html; charset=UTF-8'>
  <meta name='robots' content='noindex, nofollow'>
  <meta name='viewport' content='width=device-width, initial-scale=1.0'>
  <script type='text/javascript' src='https://www.gstatic.com/charts/loader.js'></script>
  <style>
    * { font-family:sans-serif; }
    .lowBatteryAlerts { text-align:center; }
      .lowBatteryAlerts div { display:inline-block; margin:10px; padding:10px; background-color:yellow; border-radius:10px; color:red; }
    pre { font-family:mono; font-size:10px; }
  </style>
</head>

<body>");


/*--- Builf charts --------------------------------------------------------*/
$sensorsLabel = $sensorColor = $sensorValuesCounter = $sensorsLowBattery = [];
foreach(SENSORS as $sId=>$sParams) {
  $isLowBattery = $DBL->query("SELECT lowBattery FROM temperatures WHERE sensorId='" . dbEsc($sId) . "' ORDER BY date DESC LIMIT 1")->fetch_object();
  $sensorsLabel[] = "'" . $sParams["name"] . " (ID:" . $sId . ((@$sParams["alert"]) ? ", Alarm:" . $sParams["alert"]["min"] . "-" . $sParams["alert"]["max"] . "°C" : "") . ")'";
  $sensorsColor[] = "'#" . $sParams["color"] . "'";
  $sensorValuesCounter[$sId] = 0;
  if($isLowBattery?->lowBattery) $sensorsLowBattery[] = $sId;
}

echo("<script type='text/javascript'>
  google.charts.load('current', {'packages':['corechart']});
  google.charts.setOnLoadCallback(drawChart);

  function drawChart() {
    var data = google.visualization.arrayToDataTable([
      ['Data', " . implode(", ", $sensorsLabel) . "],\n");

      $dataset = [];
      $rs = $DBL->query("SELECT * FROM (SELECT *, YEAR(date) AS datePartsYear, MONTH(date) AS datePartsMonth, DATE_FORMAT(date, '%d, %H, %i, %s, 0') AS datePartsDayAndTime FROM temperatures WHERE date>DATE_SUB(NOW(), INTERVAL 10 DAY) ORDER BY date) rsTmp ORDER BY date ASC");
      while($rc = $rs->fetch_object()) {
        $sensorValues = array_fill(0, count(SENSORS), "null");
        $sensorValues[SENSORS[$rc->sensorId]["position"] - 1] = $rc->temperature;
        $dataset[] = "[new Date(" . $rc->datePartsYear . ", " . ($rc->datePartsMonth - 1) . ", " . $rc->datePartsDayAndTime . "), " . implode(", ", $sensorValues) . "]";
        $sensorValuesCounter[$rc->sensorId]++;
      }

      //Add fake value for sensors with no data to avoid to break the chart
      foreach(SENSORS as $sId=>$sParams) {
        if($sensorValuesCounter[$sId] < 1) {
          $sensorValues = array_fill(0, count(SENSORS), "null");
          $sensorValues[SENSORS[$sId]["position"] - 1] = 0;
          echo("[new Date(" . date("Y, ") . (date("m") - 1) . date(", d, H, i, s, 0") . "), " . implode(", ", $sensorValues) . "],\n");
        }
      }

      //Output sensors data
      echo(implode(",\n", $dataset));
    echo("]);

    var options = {
      title: 'Temperatures chart (last 10 days)',
      curveType: 'function',
      legend: { position:'bottom', textStyle: { fontSize:12 } },
      vAxis: { format:'#,##0˚C', minValue:0, maxValue:20, textStyle: { fontSize:12 } },
      hAxis: { format:'dd/MM/YYYY, HH:mm', textStyle: { fontSize:12 } },
      interpolateNulls: true,
      chartArea: { top:60, bottom:60, left:70, right:20 },
      colors: [ " . implode(", ", $sensorsColor) . " ],
    };

    var chart = new google.visualization.LineChart(document.getElementById('temp_chart'));

    chart.draw(data, options);
  }
</script>

<div id='temp_chart' style='width:100%; height:600px;'></div>");

//Low battery alerts
echo("<div class='lowBatteryAlerts'>");
  foreach($sensorsLowBattery as $sId) {
    echo("<div>Low battery on <b>" . SENSORS[$sId]["name"] . "</b> (#" . $sId . ")</div>");
  }
echo("</div>");


/*--- Dump raw sernsors data ----------------------------------------------*/
echo("<hr>
<h3>Temperatures raw data from sensors (last 24 hours)</h3>
<pre>");
  $rs = $DBL->query("SELECT * FROM temperatures WHERE date>DATE_SUB(NOW(), INTERVAL 24 HOUR) ORDER BY date DESC");
  while($rc = $rs->fetch_object()) {
    echo($rc->id . "\t" . $rc->date . "\t" . $rc->sensorId . "\t" . $rc->temperature . "\t" . $rc->rawPayload . "\n");
  }
echo("</pre>");


/*--- Close HTML page -----------------------------------------------------*/
echo("</body>
</html>");


//===========================================================================


/*--- Functions -----------------------------------------------------------*/

function dbEsc($string) {
  return $GLOBALS["DBL"]->real_escape_string($string);
}
