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

SoftwareSerial pmsSerial(0, 2); //conectar TX (cable naranja) a D1 es el GPIO 5 // conectar RX (cable amarillo) a D2 es el GPIO 4
#define SEALEVELPRESSURE_HPA (1010.80)    

#define D1 5
#define D2 4
#define SDA_PIN D2  // GPIO4
#define SCL_PIN D1  // GPIO5
#define LED_OUT 5

//#define DHTPIN 2 //pin D4 del esp8266
//#define DHTTYPE DHT11
//DHT dht(DHTPIN, DHTTYPE);

Adafruit_BME280 bme;   // <- definimos el objeto bme
Adafruit_CCS811 ccs;  // <- definimos el objeto ccs

const char* ssid = WIFI_SSID;
const char* password = WIFI_PASSWORD;
const char* mqtt_server = MQTT_IP;
const int mqtt_port = 1883;
const char* mqtt_topic = "home/sensor/temperature";
const char* mqtt_topic_sub = "home/sensor/setpoint";

WiFiClient espClient;
PubSubClient client(espClient);

// ===== vairables recibidos =====

int TVOC = -1;
int CO2 = -1;
float temperature = -1;
float humidity = -1;
float height = -1;
float pressure = -1;

bool pmsRead = false;

float p03 = 0;
float p05 = 0;
float p10 = 0;
float p25 = 0;
float p50 = 0;
float p100 = 0;

// ===== Setpoints recibidos =====
float set_temp = 100;
float set_hum  = 65;
float set_pres = 1000;
float set_alt = 1000;
float set_TVOC = 1000;
float set_CO2 = 1000;
float set_particulas03 = 10000;
float set_particulas05 = 10000;
float set_particulas10 = 10000;
float set_particulas25 = 10000;
float set_particulas50 = 10000;
float set_particulas100= 10000;

//*
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
  //#define SDA_PIN D2  // GPIO4 (D2)
  //#define SCL_PIN D1  // GPIO5 (D1)
  
  if (!bme.begin(0x76)) {  // <- direccion para comunicarse con el sensor.
    Serial.println("No se detecta el BME280");  // <- por si no se comunica
    while (1);
  }

  //Activar PMS
  pmsSerial.begin(9600);

  
  //activar CCS
  //*
  //Wire.begin(SDA_PIN, SCL_PIN);
  if (!ccs.begin()) {
    Serial.println("No se encontró CCS811. Revisa conexiones.");
    while (1) { delay(1000); }
  }

    while (!ccs.available()) {
    delay(100);
  }
  Serial.println("CCS811 listo."); //*/
  Serial.println("Connecting to wifi");
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
  client.setCallback(callback);
  reconnect();
  
  pinMode(LED_OUT, OUTPUT);
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

//*
void alertaWarning(char* dataName, int setpoint, int current)
{
  Serial.print("¡Revasó el setpoint de ");
  Serial.print(dataName);
  Serial.print("! \t SETPOINT: ");
  Serial.print(setpoint);
  Serial.print(", VALUE: ");
  Serial.print(current);
  Serial.print("\n");
}

void alertas(){
  bool mainAlarm = false;
  if (temperature > set_temp) {
      alertaWarning("Temperatura", set_temp, temperature);
      mainAlarm = true;
  }
  if (humidity > set_hum) {
      alertaWarning("Humedad", set_hum, humidity);
      mainAlarm = true;
  }
  if (pressure > set_pres) {
      alertaWarning("Presion", set_pres, pressure);
      mainAlarm = true;
  }
  if (height > set_alt) {
      alertaWarning("Altitud", set_alt, height);
      mainAlarm = true;
  }
  if (TVOC > set_TVOC) {
      alertaWarning("TVOC", set_TVOC, TVOC);
      mainAlarm = true;
  }
  if (CO2 > set_CO2) {
      alertaWarning("CO2", set_CO2, CO2);
      mainAlarm = true;
  }
  if (p03 > set_particulas03) {
      alertaWarning("p03", set_particulas03, p03);
      mainAlarm = true;
  }
  if (p05 > set_particulas05) {
      alertaWarning("p05", set_particulas05, p05);
      mainAlarm = true;
  }
  if (p10 > set_particulas10) {
      alertaWarning("p10", set_particulas05, p10);
      mainAlarm = true;
  }

  if (mainAlarm)
  {
    digitalWrite(LED_OUT, HIGH);
  }else
  {
    digitalWrite(LED_OUT, LOW);  
  }
  
}//*/


float readPar03(bool dataRead){
  if (!dataRead) { return p03; }  
  return data.particles_03um;
}

float readPar05(bool dataRead){
  if (!dataRead) return p05; 
    return data.particles_05um;
}

float readPar10(bool dataRead){
  if (!dataRead) return p10; 
  return data.particles_10um;
}

float readPar25(bool dataRead){
  if (!dataRead) return p25; 
  return data.particles_25um;
}

float readPar50(bool dataRead){
  if (!dataRead) return p50; 
  return data.particles_50um;
}

float readPar100(bool dataRead){
  if (!dataRead) return p100; 
  return data.particles_100um;
}

void readBME()
{
  temperature = bme.readTemperature(); 
  humidity = bme.readHumidity();
  height = bme.readAltitude(SEALEVELPRESSURE_HPA);
  pressure = bme.readPressure() / 100.0F;
}

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

void ccs811()
{
  if (ccs.available()) {
    if (!ccs.readData()) {
      TVOC = ccs.getTVOC();
      CO2 =ccs.geteCO2();
      Serial.print("eCO2: ");
      Serial.print(CO2);
      Serial.print(" ppm\tTVOC: ");
      Serial.print(TVOC);
      Serial.println(" ppb");
    } else {
      Serial.println("Error leyendo datos del sensor.");
    }
  }
}  

void sendData()
{
 
  // Leer 
  ccs811();
  readBME();  
  pmsRead = airQuality();
  
  p03 = readPar03(pmsRead);
  p05 = readPar05(pmsRead);
  p10 = readPar10(pmsRead);
  p25 = readPar25(pmsRead);
  p50 = readPar50(pmsRead);
  p100 = readPar100(pmsRead);
  
  if (true 
      //&& !isnan(temperature)
      //&& !isnan(humidity)
      //&& !isnan(height)
      //&& !isnan(pressure)
      //&& pmsRead
      ) {
    // Crear y publicar JSON
    StaticJsonDocument<256> jsonDoc;
    jsonDoc["temperature"] = temperature;

    jsonDoc["pressure"] = pressure;
    jsonDoc["height"] = height;
    jsonDoc["humidity"] = humidity;

    jsonDoc["co2"] = CO2;
    jsonDoc["tvoc"] = TVOC;
    
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
  alertas();
}
