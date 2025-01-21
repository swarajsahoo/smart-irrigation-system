#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <DHT.h>

// Define pins and sensor types
#define DHTPIN 5          // DHT11 sensor connected to GPIO 5 (D1)
#define DHTTYPE DHT11     // DHT11 type sensor
#define SOIL_MOISTURE_PIN A0  // Soil moisture sensor connected to analog pin A0
#define RELAY_PIN 4       // Relay for water pump connected to GPIO 4 (D2)

// Threshold for soil moisture level (0 to 100)
int moistureThreshold = 40; // Adjust this value as per your requirement

// Initialize DHT sensor
DHT dht(DHTPIN, DHTTYPE);

// WiFi credentials
const char* ssid = "your-SSID";
const char* password = "your-PASSWORD";

// Web server running on port 80
ESP8266WebServer server(80);

// HTML content (served to the browser)
const char HTML_PAGE[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>AgroBEE - IoT Irrigation Monitoring</title>
    <style>
        body {
            font-family: 'Roboto', sans-serif;
            background-color: #f4f8fb;
            margin: 0;
            padding: 0;
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
            text-align: center;
            color: #333;
        }

        .container {
            width: 80%;
            max-width: 600px;
            padding: 30px;
            background-color: #ffffff;
            border-radius: 15px;
            box-shadow: 0 8px 15px rgba(0, 0, 0, 0.1);
        }

        h1 {
            font-size: 2.5em;
            color: #2c3e50;
            margin-bottom: 20px;
        }

        .data-box {
            margin: 20px 0;
            padding: 15px;
            border-radius: 10px;
            background-color: #eaf0f6;
            display: flex;
            justify-content: space-between;
            align-items: center;
            box-shadow: 0 4px 8px rgba(0, 0, 0, 0.05);
        }

        .data-box h2 {
            margin: 0;
            font-size: 1.5em;
            color: #34495e;
        }

        .data-box span {
            font-size: 1.5em;
            font-weight: bold;
            color: #16a085;
        }

        #popup {
            display: none;
            position: fixed;
            top: 50%;
            left: 50%;
            transform: translate(-50%, -50%);
            padding: 20px;
            background-color: #ffffff;
            border-radius: 10px;
            box-shadow: 0 8px 15px rgba(0, 0, 0, 0.2);
            text-align: center;
            z-index: 999;
        }

        #popup h3 {
            color: #e74c3c;
            font-size: 1.8em;
            margin: 0;
        }

        #popup button {
            margin-top: 15px;
            padding: 10px 20px;
            background-color: #16a085;
            color: white;
            border: none;
            border-radius: 5px;
            cursor: pointer;
            font-size: 1em;
            transition: background-color 0.3s ease;
        }

        #popup button:hover {
            background-color: #1abc9c;
        }

        footer {
            margin-top: 40px;
            font-size: 0.9em;
            color: #95a5a6;
        }
    </style>
</head>
<body>

    <div class="container">
        <h1>AgroBEE</h1>
        <h2>IoT Irrigation Monitoring System</h2>
        
        <div class="data-box">
            <h2>Temperature:</h2>
            <span id="temp">--</span> °C
        </div>
        
        <div class="data-box">
            <h2>Humidity:</h2>
            <span id="humidity">--</span> %
        </div>
        
        <div class="data-box">
            <h2>Soil Moisture:</h2>
            <span id="soil">--</span> %
        </div>

        <div id="popup">
            <h3 id="popup-text"></h3>
            <button onclick="closePopup()">OK</button>
        </div>
        
        <footer>
            <p>© 2024 AgroBEE | Precision Farming Technology</p>
        </footer>
    </div>

    <script>
        function fetchSensorData() {
            fetch('/api/data')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('temp').textContent = data.temperature;
                    document.getElementById('humidity').textContent = data.humidity;
                    document.getElementById('soil').textContent = data.soilMoisture;

                    if (data.soilMoisture < 40) {
                        showPopup("Water is needed for irrigation!");
                    }
                })
                .catch(error => console.error('Error fetching sensor data:', error));
        }

        function showPopup(message) {
            document.getElementById('popup-text').textContent = message;
            document.getElementById('popup').style.display = "block";
        }

        function closePopup() {
            document.getElementById('popup').style.display = "none";
        }

        window.onload = fetchSensorData;
    </script>

</body>
</html>
)=====";

void setup() {
  // Start serial communication
  Serial.begin(115200);

  // Initialize DHT sensor
  dht.begin();

  // Set the relay pin as output
  pinMode(RELAY_PIN, OUTPUT);
  digitalWrite(RELAY_PIN, LOW);

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi..");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("Connected!");

  // Define the route for data and serving the HTML page
  server.on("/", handleRoot);
  server.on("/api/data", handleSensorData);

  // Start the web server
  server.begin();
  Serial.println("HTTP server started");
}

void loop() {
  // Handle client requests
  server.handleClient();

  // Reading soil moisture sensor value
  int soilMoistureValue = analogRead(SOIL_MOISTURE_PIN);
  int soilMoisturePercent = map(soilMoistureValue, 1023, 0, 0, 100);

  // Turn on/off the water pump based on soil moisture
  if (soilMoisturePercent < moistureThreshold) {
    digitalWrite(RELAY_PIN, HIGH);  // Pump ON
  } else {
    digitalWrite(RELAY_PIN, LOW);   // Pump OFF
  }
}

// Function to handle the root path and serve the HTML page
void handleRoot() {
  server.send_P(200, "text/html", HTML_PAGE);
}

// Function to handle HTTP requests and send sensor data
void handleSensorData() {
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();
  int soilMoistureValue = analogRead(SOIL_MOISTURE_PIN);
  int soilMoisturePercent = map(soilMoistureValue, 1023, 0, 0, 100);

  // Create a JSON object to send the data
  String json = "{";
  json += "\"temperature\":" + String(temperature) + ",";
  json += "\"humidity\":" + String(humidity) + ",";
  json += "\"soilMoisture\":" + String(soilMoisturePercent);
  json += "}";

  // Send the JSON object
  server.send(200, "application/json", json);
}
