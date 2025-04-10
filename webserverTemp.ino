#include <ESP8266WiFi.h>
#include <Hash.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <NTPClient.h>
#include <TimeLib.h>
#include <WiFiUdp.h>
#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is connected to GPIO 4
#define ONE_WIRE_BUS 2

// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);

// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);

// Variables to store temperature values
String temperatureF = "";
String temperatureC = "";

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "time.nist.gov", 3600);

// Timer variables
unsigned long lastTime1 = 0;  
unsigned long lastTime2 = 0;  
unsigned long timerDelay1 = 1000;
unsigned long timerDelay2 = 10000;

String tempsC[50];
String tempsF[50];
String times[50];
int savedTemps = 0;

// Replace with your network credentials
const char* ssid = "SSID";
const char* password = "PASSWORD";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

String readDSTemperatureC() {
  // Call sensors.requestTemperatures() to issue a global temperature and Requests to all devices on the bus
  sensors.requestTemperatures(); 
  float tempC = sensors.getTempCByIndex(0);

  if(tempC == -127.00) {
    Serial.println("Failed to read from DS18B20 sensor");
    return "--";
  }
  return String(tempC);
}

String readDSTemperatureF() {
  // Call sensors.requestTemperatures() to issue a global temperature and Requests to all devices on the bus
  sensors.requestTemperatures(); 
  float tempF = sensors.getTempFByIndex(0);

  if(int(tempF) == -196){
    Serial.println("Failed to read from DS18B20 sensor");
    return "--";
  }
  return String(tempF);
}

const char index_html_part1[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css">
  <style>
    html {
     font-family: Arial;
     display: inline-block;
     margin: 0px auto;
     text-align: center;
    }
    h2 { font-size: 3.0rem; }
    p { font-size: 3.0rem; }
    .units { font-size: 1.2rem; }
    .ds-labels{
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }
  </style>
</head>
<body>
  <h2>ESP DS18B20 Server</h2>
  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
    <span class="ds-labels">Temperature Celsius</span> 
    <span id="temperaturec">%TEMPERATUREC%</span>
    <sup class="units">&deg;C</sup>
  </p>
)rawliteral";

const char index_html_part2[] PROGMEM = R"rawliteral(
  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i> 
    <span class="ds-labels">Temperature Fahrenheit</span>
    <span id="temperaturef">%TEMPERATUREF%</span>
    <sup class="units">&deg;F</sup>
  </p>
  <p> 
    <span class="ds-labels">Uhrzeit:</span>
    <span id="time">%TIME%</span>
  </p>
  <p>
    <button onclick="downloadTemps()">Download Temperaturen</button>
  </p>
  <p>
    <button onclick="downloadTempsDB()">Download Temperaturen DB</button>
  </p>
  <script>
    function downloadTempsDB() {
  fetch('http://192.168.137.1/log_data.php?get=temps')
    .then(response => response.json())
    .then(data => {
      // Format each entry: tempC, tempF, timestamp
      const formatted = data.map(entry => {
      const tempC = parseFloat(entry.tempC).toFixed(2);
      const tempF = parseFloat(entry.tempF).toFixed(2);
      const time = entry.time;
      return `${tempC}, ${tempF}, ${time}`;
    });

    const text = formatted.join('\n'); // Each reading on a new line
    const blob = new Blob([text], { type: 'text/plain' });
    const url = URL.createObjectURL(blob);

    const a = document.createElement('a');
    a.href = url;
    a.download = 'tempsDB.txt';
    a.click();
    URL.revokeObjectURL(url);
  })
}
function downloadTemps() {
  fetch('/temps')
    .then(response => response.json())
    .then(data => {
      // Format each entry: tempC, tempF, timestamp
      const formatted = data.map(entry => {
        const tempC = parseFloat(entry[0]).toFixed(2);
        const tempF = parseFloat(entry[1]).toFixed(2);
        const time = entry[2];
        return `${tempC}, ${tempF}, ${time}`;
      });

      const text = formatted.join('\n'); // Each reading on a new line
      const blob = new Blob([text], { type: 'text/plain' });
      const url = URL.createObjectURL(blob);

      const a = document.createElement('a');
      a.href = url;
      a.download = 'temps.txt';
      a.click();
      URL.revokeObjectURL(url)}
    )
}
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temperaturec").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/temperaturec", true);
  xhttp.send();
}, 1000) ;
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temperaturef").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/temperaturef", true);
  xhttp.send();
}, 1000) ;
var time;
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("time").innerHTML = this.responseText.replace("_", " ").slice(11, 19);
      time = this.responseText;
    }
  };
  xhttp.open("GET", "/time", true);
  xhttp.send();
}, 1000); 
setInterval(function () {
  const tempCEl = document.getElementById("temperaturec");
  const tempFEl = document.getElementById("temperaturef");
  if (tempCEl && tempFEl) {
    const tempC = tempCEl.textContent.trim();
    const tempF = tempFEl.textContent.trim();
    if (!isNaN(tempC) && !isNaN(tempF)) {
      fetch(`http://192.168.137.1/log_data.php?tempC=${encodeURIComponent(tempC)}&tempF=${encodeURIComponent(tempF)}&time=${encodeURIComponent(time)}`)
        .then(res => {
          if (!res.ok) throw new Error('Network response was not ok');
          console.log("Temperature logged:", tempC, tempF);
        })
        .catch(err => console.error("Logging error:", err));
    }
  }
}, 30000);
  </script>
</body>
</html>
)rawliteral";

// Then combine them in the handler like this:
String buildPage() {
  String page = FPSTR(index_html_part1);
  page += FPSTR(index_html_part2);
  return page;
}


String processor(const String& var){
  if(var == "TEMPERATUREC"){
    return temperatureC;
  }
  else if(var == "TEMPERATUREF"){
    return temperatureF;
  }
  return String();
}

String getTempsJson() {
  String json = "[";
  for (int i = 0; i < savedTemps; i++) {
    json += "[" + tempsC[i] + "," + tempsF[i] + ",\"" + times[i] + "\"]";
    if (i < savedTemps - 1) {
      json += ",";
    }
  }
  json += "]";
  return json;
}



String getFullFormattedDateTime() {
  time_t rawTime = timeClient.getEpochTime();
  struct tm * timeinfo = gmtime(&rawTime); // UTC-Zeit

  char buffer[25];
  sprintf(buffer, "%04d-%02d-%02d %02d:%02d:%02d",
          timeinfo->tm_year + 1900,
          timeinfo->tm_mon + 1,
          timeinfo->tm_mday,
          timeinfo->tm_hour,
          timeinfo->tm_min,
          timeinfo->tm_sec);

  String result = String(buffer);
  result.replace(' ', '_');  // Ersetze Leerzeichen mit Unterstrich
  return result;
}

void setup(){
  Serial.begin(115200);
  Serial.println();
  
  // Start up the DS18B20 library
  sensors.begin();

  temperatureC = readDSTemperatureC();
  temperatureF = readDSTemperatureF();

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.println("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  
  // Print ESP Local IP Address
  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", buildPage());
  });
  server.on("/temperaturec", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", temperatureC.c_str());
  });
  server.on("/temperaturef", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", temperatureF.c_str());
  });
  server.on("/time", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", getFullFormattedDateTime().c_str());
  });
  server.on("/hello", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", "Hello World!");
  });
  server.on("/temps", HTTP_GET, [](AsyncWebServerRequest *request){
    String json = getTempsJson();
    request->send(200, "application/json", json);
  });

  // Start server
  server.begin();

  timeClient.begin();
  timeClient.setTimeOffset(7200); // GMT+1

  int retries = 0;
  while (!timeClient.update() && retries < 10) {
    Serial.println("Trying to sync time...");
    timeClient.forceUpdate();
    retries++;
    delay(500);
  }

  if (!timeClient.isTimeSet()) {
    Serial.println("Failed to get time from NTP.");
  } else {
    Serial.println("Time synchronized.");
  }
}

 
void loop(){
  if ((millis() - lastTime1) > timerDelay1) {
    temperatureC = readDSTemperatureC();
    temperatureF = readDSTemperatureF();
    timeClient.update();
    lastTime1 = millis();
  }  
  if ((millis() - lastTime2) > timerDelay2 || times[0] == "") {
    if (savedTemps < 50) {
      tempsC[savedTemps] = temperatureC;
      tempsF[savedTemps] = temperatureF;
      times[savedTemps] = getFullFormattedDateTime().c_str();
      savedTemps++;
    } else {
      for (int i = 0; i < 49; i++) {
        tempsC[i] = tempsC[i + 1];
        tempsF[i] = tempsF[i + 1];
        times[i] = times[i + 1];
      }
      tempsC[49] = temperatureC;
      tempsF[49] = temperatureF;
      times[49] = getFullFormattedDateTime().c_str();
    }
    lastTime2 = millis();
  }  
}
