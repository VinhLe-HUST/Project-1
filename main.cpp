#include <Arduino.h>
#include "WiFi.h"
#include "ESPAsyncWebServer.h"
#include <Adafruit_Sensor.h>
#include <DHT.h>
const char *ssid = "esp";
const char *password = "12345678";
#define DHTPIN 17 
#define DHTTYPE DHT11 
DHT dht(DHTPIN, DHTTYPE);
AsyncWebServer server(80);
const char* loginUsername = "ESP32"; 
const char* loginPassword = "12345678"; 
String readDHTTemperature() {
  float t = dht.readTemperature();
  if (isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return "--";
  } else {
    Serial.println(t);
    return String(t);
  }
}
String readDHTHumidity() {
  float h = dht.readHumidity();
  if (isnan(h)) {
    Serial.println("Failed to read from DHT sensor!");
    return "--";
  } else {
    Serial.println(h);
    return String(h);
  }
}
const char login_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <style>
    body {
      font-family: Arial, sans-serif;
      text-align: center;
      margin-top: 50px;
      background-image: url('https://phunuvietnam.mediacdn.vn/179072216278405120/2022/9/8/1dai-hoc-bach-khoa-ha-no-1662551195810678616357-1662628589326-1662628590575152609796.jpg');
      background-size: 100%;
      background-position: center;
    }
    .login-container {
      display: inline-block;
      padding: 20px;
      border: 1px solid #ccc;
      border-radius: 10px;
      background: #f9f9f9;
    }
    input {
      display: block;
      margin: 10px auto;
      padding: 10px;
      width: 80%;
    }
    button {
      padding: 10px 20px;
      background: #059e8a;
      border: none;
      color: white;
      cursor: pointer;
    }
  </style>
</head>
<body>
  <div class="login-container">
    <h2>Login</h2>
    <input type="text" id="username" placeholder="Username">
    <input type="password" id="password" placeholder="Password">
    <button onclick="login()">Login</button>
    <p id="error" style="color:red;"></p>
  </div>
<script>
function login() {
  var username = document.getElementById('username').value;
  var password = document.getElementById('password').value;
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      if (this.responseText == 'success') {
        window.location.href = '/home';
      } else {
        document.getElementById('error').innerText = 'Invalid login. Please try again.';
      }
    }
  };
  xhttp.open('POST', '/login', true);
  xhttp.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
  xhttp.send('username=' + username + '&password=' + password);
}
</script>
</body>
</html>)rawliteral";
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <style>
    html {
      font-family: Arial;
      display: inline-block;
      margin: 0px auto;
      text-align: center;
      background-image: url('https://www.didongmy.com/vnt_upload/news/03_2024/hinh-nen-4k-la-gi-Didongmy.jpg');
      background-size: cover;
    }
    h2 { 
      font-size: 3.0rem; 
      color: white;
    }
    p { 
      font-size: 3.0rem; 
      color: white;
    }
    .units { 
      font-size: 1.2rem; 
      color: white;
    }
    .dht-labels {
      font-size: 1.5rem;
      vertical-align: middle;
      padding-bottom: 15px;
      color: white;
    }
    .container {
      background: rgba(0, 0, 0, 0.5);
      padding: 20px;
      border-radius: 10px;
      display: inline-block;
    }
  </style>
</head>
<body>
  <h2>ESP32 DHT Server</h2>
  <div class="container">
    <p>
      <i class="fas fa-thermometer-half" style="color:#059e8a;"></i>
      <span class="dht-labels">Temperature</span> 
      <span id="temperature">%TEMPERATURE%</span>
      <sup class="units">&deg;C</sup>
    </p>
    <p>
      <i class="fas fa-tint" style="color:#00add6;"></i> 
      <span class="dht-labels">Humidity</span>
      <span id="humidity">%HUMIDITY%</span>
      <sup class="units">&percnt;</sup>
    </p>
  </div>
</body>
<script>
setInterval(function () {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temperature").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/temperature", true);
  xhttp.send();
}, 10000);

setInterval(function () {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("humidity").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/humidity", true);
  xhttp.send();
}, 10000);
</script>
</html>)rawliteral";
String processor(const String &var) {
  if (var == "TEMPERATURE") {
    return readDHTTemperature();
  } else if (var == "HUMIDITY") {
    return readDHTHumidity();
  }
  return String();
}
void setup() {
  Serial.begin(115200);
  Serial.println("Connecting to WiFi..");
  dht.begin();
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  Serial.println(WiFi.localIP());
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", login_html);
  });
  server.on("/login", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("username", true) && request->hasParam("password", true)) {
      String username = request->getParam("username", true)->value();
      String password = request->getParam("password", true)->value();
      if (username == loginUsername && password == loginPassword) {
        request->send(200, "text/plain", "success");
      } else {
        request->send(200, "text/plain", "fail");
      }
    }
  });
  server.on("/home", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", index_html, processor);
  });
  server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/plain", readDHTTemperature().c_str());
  });
  server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/plain", readDHTHumidity().c_str());
  });
  server.begin();
}
void loop() {
}