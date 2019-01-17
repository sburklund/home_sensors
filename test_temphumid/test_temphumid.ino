#include <DHT.h>

// Set the pin the DHT sensor is on
#define DHTPIN 4
// Set the type of DHT sensor
#define DHTTYPE DHT11

// Initialize DHT sensor
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);

  // Initialize the DHT sensor
  dht.begin();

}

void loop() {
  // put your main code here, to run repeatedly:
  float f = dht.readTemperature(true);
  float h = dht.readHumidity();
  Serial.print("Temp: "); Serial.print(f);
  Serial.print(" Humid: "); Serial.println(h);
}
