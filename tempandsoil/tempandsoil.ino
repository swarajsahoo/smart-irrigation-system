#include <DHT.h>

// Define pins and sensor types
#define DHTPIN 5          // DHT11 sensor connected to GPIO 5 (D1)
#define DHTTYPE DHT11     // DHT11 type sensor
#define SOIL_MOISTURE_PIN A0  // Soil moisture sensor connected to analog pin A0

// Initialize the DHT sensor
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  // Start serial communication
  Serial.begin(115200);
  
  // Initialize DHT sensor
  dht.begin();
  
  // Allow time for the sensors to stabilize
  delay(2000);
}

void loop() {
  // Reading soil moisture sensor value
  int soilMoistureValue = analogRead(SOIL_MOISTURE_PIN);
  // Map the soil moisture value to a percentage (0 - 100)
  int soilMoisturePercent = map(soilMoistureValue, 1023, 0, 0, 100);
  
  // Reading temperature and humidity from DHT11 sensor
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();  // Temperature in Celsius
  
  // Check if any readings failed
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Failed to read from DHT sensor!");
  } else {
    // Print DHT11 sensor readings
    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.print(" %\t");
    Serial.print("Temperature: ");
    Serial.print(temperature);
    Serial.println(" °C");
  }
  
  // Print soil moisture sensor readings
  Serial.print("Soil Moisture Value (Raw): ");
  Serial.println(soilMoistureValue);
  
  Serial.print("Soil Moisture (Percentage): ");
  Serial.print(soilMoisturePercent);
  Serial.println(" %");

  // Wait for 2 seconds before next reading
  delay(2000);
}
