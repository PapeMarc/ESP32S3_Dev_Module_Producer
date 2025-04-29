#include <string>

// OLED-Display
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
// temperature sensor
#include "DHT.h"
// WiFi-Manager
#include <WiFiManager.h>
// MQTT Client
#include <PubSubClient.h>

// connecting sensor
#define DHTTYPE DHT22
#define DHT22_Pin 21
DHT dht(DHT22_Pin, DHTTYPE);

// configuring OLED
#define I2C_SDA 14 //4  // GPIO 14
#define I2C_SCL 13 //5  // GPIO 13
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1 // Reset-Pin (-1 = not used)
#define I2C_ADDRESS 0x3C // Standard Address for SH1106
Adafruit_SH1106G display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);


// Variables
WiFiManager wm;
WiFiClient espClient;
String ssid;

PubSubClient client(espClient);
const char* mqtt_server = "YOUR_SERVER_ADDRESS";
const int mqtt_port = 1883;
const char* mqtt_user = "MQTT_USERNAME";
const char* mqtt_password = "MQTT_PASSWORD";

float humidity, temperature;

void setup()
{
  WiFi.mode(WIFI_STA);
  Serial.begin(115200);
  //wm.resetSettings(); //Reset settings in setup if you wish to connect to another Network
  
  bool res;
  res = wm.autoConnect("ESP32-Setup", "WEBSERVER_PASSWORD");

  if(!res){
    Serial.println("Failed to connect.");
    
  }
  else{
    ssid = wm.getWiFiSSID();
    Serial.print("Connected successfully to ");
    Serial.println(ssid);
  }

  // Setting MQTT Server
  client.setServer(mqtt_server, mqtt_port);

  // Initializing the OLED via I2C
  Wire.begin(I2C_SDA, I2C_SCL); 
  if (!display.begin(I2C_ADDRESS)) {
    Serial.println(F("Display could not be initialized!"));
  }
  display.display();
  display.clearDisplay();

  // Initializing temperature Sensor
  dht.begin();
}

void reconnect() {
  // Loop until connected
  while (!client.connected()) {
    if (client.connect("ESP32Client", mqtt_user, mqtt_password)) {
      Serial.println("Connected successfully to MQTT-Broker.");
    } else {
      delay(5000); // try every five secounds
    }
  }
}

void loop(){

  if(!ssid){
    exit(0);
  }

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  //Read temperature and humidity
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();

  char buffer[10];
  snprintf(buffer, sizeof(buffer), "%.2f", temperature);
  client.publish("ESP32-S3/temperature", buffer);

  snprintf(buffer, sizeof(buffer), "%.2f", humidity);
  client.publish("ESP32-S3/humidity", buffer);

  // Console Output
  Serial.print("Sent ");
  Serial.print(humidity);
  Serial.println("% Humidity and ");
  Serial.print(temperature);
  Serial.print((char)247);
  Serial.print("C to ");
  Serial.print(mqtt_server);
  Serial.print(":");
  Serial.print(mqtt_port);

  // Display Output
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0,0);
  
  display.print("Connected to ");
  display.println(ssid);
  display.print("Broker: ");
  display.print(mqtt_server);
  display.print(":");
  display.println(mqtt_port);

  display.display();
  delay(5000);  
}