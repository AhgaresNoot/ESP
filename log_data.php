<?php
header("Access-Control-Allow-Origin: *");

$servername = "localhost";
$username = "root";
$password = "Passw0rd";
$dbname = "esp_data";

// Create connection
$conn = new mysqli($servername, $username, $password, $dbname);

// Check for connection error
if ($conn->connect_error) {
  die("Connection failed: " . $conn->connect_error);
}

// Return JSON if ?get=temps is set
if (isset($_GET['get']) && $_GET['get'] === "temps") {
  header('Content-Type: application/json');
  $sql = "SELECT tempC, tempF, time FROM sensor_data ORDER BY id ASC";
  $result = $conn->query($sql);

  $data = [];

  if ($result->num_rows > 0) {
    while ($row = $result->fetch_assoc()) {
      $timestamp = strtotime($row["time"]);
      $formattedTime = date("H:i:s d.m.Y", $timestamp);

      $data[] = [
        "tempC" => $row["tempC"],
        "tempF" => $row["tempF"],
        "time"  => $formattedTime
      ];
    }
  }

  echo json_encode($data);
  $conn->close();
  exit;
}

// INSERT block
if (isset($_GET['tempC']) && isset($_GET['tempF']) && isset($_GET['time'])) {
  $tempC = $conn->real_escape_string($_GET['tempC']);
  $tempF = $conn->real_escape_string($_GET['tempF']);
  $time = $conn->real_escape_string($_GET['time']);

  $sql = "INSERT INTO sensor_data (tempC, tempF, time) VALUES ('$tempC', '$tempF', '$time')";

  if ($conn->query($sql) === TRUE) {
    echo "Data inserted successfully";
  } else {
    echo "Error: " . $conn->error;
  }
} else {
  echo "Missing tempC or tempF";
}

$conn->close();
?>
