//#include <DHT.h>

#include <Wire.h>                 // <- libreria para cambiar los pines I2C
#include <Adafruit_Sensor.h>      // <- libreria adafruit pt1
#include <Adafruit_BME280.h>      // <- libreria adafruit pt2
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>
#include "src/PMS5003.h"
#include "src/config.h"

SoftwareSerial pmsSerial(5, 4); //conectar TX (cable naranja) a D1 es el GPIO 5 // conectar RX (cable amarillo) a D2 es el GPIO 4
#define SEALEVELPRESSURE_HPA (1010.80)    

//#define DHTPIN 2 //pin D4 del esp8266
//#define DHTTYPE DHT11
//DHT dht(DHTPIN, DHTTYPE);

Adafruit_BME280 bme;   // <- definimos el objeto bme

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;
const char* mqtt_server = MQTT_IP;
const int mqtt_port = 1883;
const char* mqtt_topic = "home/sensor/temperature";

WiFiClient espClient;
PubSubClient client(espClient);

//SETUP
void setup() {
  Serial.begin(115200);
  //dht.begin();

  // Activar BME
  // Cambiar los pines I2C aquí 👇
  Wire.begin(12, 14);  // SDA = GPIO12(D6), SCL = GPIO14(D5)

  if (!bme.begin(0x76)) {  // <- direccion para comunicarse con el sensor.
    Serial.println("No se detecta el BME280");  // <- por si no se comunica
    while (1);
  }

  //Activar PMS
  pmsSerial.begin(9600);
  
  // Conectar a WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
  // Configurar MQTT
  client.setServer(mqtt_server, mqtt_port);
  reconnect();
}

void reconnect() {
  while (!client.connected()) {
    if (client.connect("ESP8266Client")) {
      Serial.println("MQTT connected");
    } else {
      delay(5000);
    }
  }
}

float readTemperature(){
   return bme.readTemperature();
}

float readHumidity()
{
  return bme.readHumidity();
}

float readHeight()
{
  return bme.readAltitude(SEALEVELPRESSURE_HPA);
}

float readPressure()
{
  return bme.readPressure() / 100.0F;
}

float readPM10()
{
  if (readPMSdata(&pmsSerial)) {
    return data.pm10_standard;
  }
  return 0;
}

float readPM25()
{
  if (readPMSdata(&pmsSerial)) {
    return data.pm25_standard;
  }
  return 0;
}

float readPar03(){
  if (readPMSdata(&pmsSerial)) {
    return data.particles_03um;
  }
  return 0;
}

float readPar05(){
  if (readPMSdata(&pmsSerial)) {
    return data.particles_05um;
  }
  return 0;
}

float readPar10(){
  if (readPMSdata(&pmsSerial)) {
    return data.particles_05um;
  }
  return 0;
}

float readPar25(){
  if (readPMSdata(&pmsSerial)) {
    return data.particles_25um;
  }
  return 0;
}

float readPar50(){
  if (readPMSdata(&pmsSerial)) {
    return data.particles_50um;
  }
  return 0;
}

float readPar100(){
  if (readPMSdata(&pmsSerial)) {
    return data.particles_100um;
  }
  return 0;
}

/*/

Serial.print("Particles > 0.3um / 0.1L air:"); Serial.println(data.particles_03um);
Serial.print("Particles > 0.5um / 0.1L air:"); Serial.println(data.particles_05um);
Serial.print("Particles > 1.0um / 0.1L air:"); Serial.println(data.particles_10um);
Serial.print("Particles > 2.5um / 0.1L air:"); Serial.println(data.particles_25um);
Serial.print("Particles > 5.0um / 0.1L air:"); Serial.println(data.particles_50um);
Serial.print("Particles > 10.0 um / 0.1L air:"); Serial.println(data.particles_100um);

//*/

void loop() {
  delay(2000);
  if (!client.connected()) reconnect();
  client.loop();
  
  // Leer 
  float temperature = readTemperature();
  float humidity = readHumidity();
  float height = readHeight();
  float pressure = readPressure();

  float p03 = readPar03();
  float p05 = readPar05();
  float p10 = readPar10();
  float p25 = readPar25();
  float p50 = readPar50();
  float p100 = readPar100();
  
  if (!isnan(temperature)
      || !isnan(humidity)
      || !isnan(height)
      || !isnan(pressure)
      || !isnan(p03)
      || !isnan(p05)
      || !isnan(p10)
      || !isnan(p25)
      || !isnan(p50)
      || !isnan(p100)
      ) {
    // Crear y publicar JSON
    StaticJsonDocument<256> jsonDoc;
    jsonDoc["temperature"] = temperature;
    jsonDoc["unit"] = "Celsius";

    jsonDoc["pressure"] = pressure;
    jsonDoc["height"] = height;
    jsonDoc["humidity"] = humidity;

    jsonDoc["par03"]  = p03;
    jsonDoc["par05"]  = p05;
    jsonDoc["par10"]  = p10;
    jsonDoc["par25"]  = p25;
    jsonDoc["par50"]  = p50;
    jsonDoc["par100"] = p100;
     
    char jsonBuffer[256];
    serializeJson(jsonDoc, jsonBuffer);
    client.publish(mqtt_topic, jsonBuffer);
    Serial.println(jsonBuffer);
  } else {
    Serial.println("Failed to read from sensors");
  }
}
