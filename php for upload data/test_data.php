<?php

$hostname = " "; //host name
$username = "  "; //username phpmyadmin
$password = "  "; //password phpmyadmin
$database = "  "; //database name

$conn = mysqli_connect($hostname, $username, $password, $database);

if (!$conn) {
    die("Connection Failed : " . mysqli_connect_error());
}

echo "Database Connection is OK";

if (isset($_POST["Temperature"]) && isset($_POST["Humidity"])) { // Corrected the syntax here
    $t = $_POST["Temperature"];
    $h = $_POST["Humidity"];

    $sql = "INSERT INTO dht11 (Temperature, Humidity) VALUES (" . $t . ", " . $h . ")";

    if (mysqli_query($conn, $sql)) {
        echo "New record created successfully";
    } else {
        echo "Error: " . $sql . "<br>" . mysqli_error($conn);
    }
}

?>
