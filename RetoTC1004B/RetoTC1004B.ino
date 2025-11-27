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
#include "Adafruit_CCS811.h"

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
const char* mqtt_topic_sub = "home/sensor/setpoint";

WiFiClient espClient;
PubSubClient client(espClient);

// ===== vairables recibidos =====
  float temperature = 0;
  float humidity = 0;
  float height = 0;
  float pressure = 0;
  
  bool pmsRead = false;
  
  float p03 = 0;
  float p05 = 0;
  float p10 = 0;
  float p25 = 0;
  float p50 = 0;
  float p100 = 0;

// ===== Setpoints recibidos =====
float set_temp = 0;
float set_hum  = 0;
float set_pres = 0;
float set_alt = 0;
float set_TVOC = 0;
float set_CO2 = 0;
float set_particulas03 = 0;
float set_particulas05 = 0;
float set_particulas10 = 0;

/*
// ====================
//     CALLBACK MQTT
// ====================
void callback(char* topic, byte* payload, unsigned int length) {
  String msg = "";

  for (int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }

  Serial.print("Mensaje recibido: ");
  Serial.println(msg);

  StaticJsonDocument<128> doc;
  auto error = deserializeJson(doc, msg);

  if (!error) {
    if (doc.containsKey("set_temp")) {
      set_temp = doc["set_temp"];
      Serial.print("Nuevo set_temp: ");
      Serial.println(set_temp);
    }

    if (doc.containsKey("set_hum")) {
      set_hum = doc["set_hum"];
      Serial.print("Nuevo set_hum: ");
      Serial.println(set_hum);
    }

    if (doc.containsKey("set_pres")) {
      set_pres = doc["set_pres"];
      Serial.print("Nuevo set_pres: ");
      Serial.println(set_pres);
    }

        if (doc.containsKey("set_alt")) {
      set_alt = doc["set_alt"];
      Serial.print("Nuevo set_alt: ");
      Serial.println(set_alt);
    }

        if (doc.containsKey("set_TVOC")) {
      set_TVOC = doc["set_TVOC"];
      Serial.print("Nuevo set_TVOC: ");
      Serial.println(set_TVOC);
    }

        if (doc.containsKey("set_CO2")) {
      set_CO2 = doc["set_CO2"];
      Serial.print("Nuevo set_CO2: ");
      Serial.println(set_CO2);
    }

        if (doc.containsKey("set_particulas03")) {
      set_particulas03 = doc["set_particulas03"];
      Serial.print("Nuevo set_particulas03: ");
      Serial.println(set_particulas03);
    }

        if (doc.containsKey("set_particulas10")) {
      set_particulas10 = doc["set_particulas10"];
      Serial.print("Nuevo set_particulas10: ");
      Serial.println(set_particulas10);
    }
  } else {
    Serial.println("Error al leer JSON");
  }
}
//*/

//SETUP
void setup() {
  Serial.begin(115200);

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
  Serial.print("\n.");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
  // Configurar MQTT
  client.setServer(mqtt_server, mqtt_port);
  //client.setCallback(callback);
  reconnect();
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Intentando conectar MQTT...");
    
    if (client.connect("ESP8266Client")) {
      Serial.println("Conectado!");
      client.subscribe(mqtt_topic_sub);
    } else {
      Serial.print("Fallo, rc=");
      Serial.println(client.state());
      delay(5000);
    }
  }
}


void alertas(){
  if (temperatura > set_temp) {
      Serial.println(" ¡ Revasó el setpoint de Temperatura! ");
  }
  if (humedad > set_hum) {
      Serial.println(" ¡ Revasó el setpoint de Humedad! ");
  }
  if (presion > set_pres) {
      Serial.println(" ¡ Revasó el setpoint de Presión! ");
  }
  if (altitud > set_alt) {
      Serial.println(" ¡ Revasó el setpoint de Altitud! ");
  }
  if (altitud > set_TVOC) {
      Serial.println(" ¡ Revasó el setpoint de TVOC! ");
  }
  if (altitud > set_CO2) {
      Serial.println(" ¡ Revasó el setpoint de CO2! ");
  }
  if (altitud > set_particulas03) {
      Serial.println(" ¡ Revasó el setpoint de particulas03! ");
  }
  if (altitud > set_particulas05) {
      Serial.println(" ¡ Revasó el setpoint de particulas05! ");
  }
  if (altitud > set_particulas10) {
      Serial.println(" ¡ Revasó el setpoint de particulas10! ");
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

float readPar03(bool dataRead){
  if (!dataRead) { return -1; }  
  return data.particles_03um;
}

float readPar05(bool dataRead){
  if (!dataRead) return -1; 
    return data.particles_05um;
}

float readPar10(bool dataRead){
  if (!dataRead) return -1; 
  return data.particles_10um;
}

float readPar25(bool dataRead){
  if (!dataRead) return -1; 
  return data.particles_25um;
}

float readPar50(bool dataRead){
  if (!dataRead) return -1; 
  return data.particles_50um;
}

float readPar100(bool dataRead){
  if (!dataRead) return -1; 
  return data.particles_100um;
}

/*/

Serial.print("Particles > 0.3um / 0.1L air:"); Serial.println(data.particles_03um);
Serial.print("Particles > 0.5um / 0.1L air:"); Serial.println(data.particles_05um);
Serial.print("Particles > 1.0um / 0.1L air:"); Serial.println(data.particles_10um);
Serial.print("Particles > 2.5um / 0.1L air:"); Serial.println(data.particles_25um);
Serial.print("Particles > 5.0um / 0.1L air:"); Serial.println(data.particles_50um);
Serial.print("Particles > 10.0 um / 0.1L air:"); Serial.println(data.particles_100um);

//*/

bool airQuality()
{
  if (readPMSdata(&pmsSerial))
  {
    ppm25 = ppm25 + data.pm25_standard; //Standard - pick one Stnd or Envir
    eventCount++;
    return true;
  }
  return false;
}

void sendData()
{
 
  // Leer 
  temperature = readTemperature();
  humidity = readHumidity();
  height = readHeight();
  pressure = readPressure();
  
  pmsRead = airQuality();
  
  p03 = readPar03(pmsRead);
  p05 = readPar05(pmsRead);
  p10 = readPar10(pmsRead);
  p25 = readPar25(pmsRead);
  p50 = readPar50(pmsRead);
  p100 = readPar100(pmsRead);
  
  if (!isnan(temperature)
      && !isnan(humidity)
      && !isnan(height)
      && !isnan(pressure)
      && pmsRead
      ) {
    // Crear y publicar JSON
    StaticJsonDocument<256> jsonDoc;
    jsonDoc["temperature"] = temperature;

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

void loop() {
  delay(500);
  if (!client.connected()) reconnect();
  client.loop();
  sendData();
}
