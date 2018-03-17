#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DallasTemperature.h>

#define SLEEP_DELAY_IN_SECONDS  1800
#define ONE_WIRE_BUS            D4      // DS18B20 pin

const char* ssid = "";
const char* password = "";

const char* mqtt_server = "192.168.1.131";
const char* mqtt_username = "";
const char* mqtt_password = "";
const char* mqtt_topic = "tempsud";
const char* mqtt_topic2 = "battery";

IPAddress subnet(255, 255, 255, 0); // set subnet mask to match your network
IPAddress ip(192, 168, 1, 212); // controllare che non sia doppio
IPAddress gateAway(192, 168, 1, 254);
IPAddress DNS(8, 8, 8, 8);

WiFiClient espClient;
PubSubClient client(espClient);

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);

char temperatureString[6];

void setup() {
  DS18B20.begin();
}

void setup_wifi() {
  delay(10);
  WiFi.config(ip, gateAway, subnet);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    // Attempt to connect
    if (client.connect("ESP8266Client", mqtt_username, mqtt_password)) {
    } else {
      delay(5000);
    }
  }
}

float getTemperature() {
  //Serial << "Requesting DS18B20 temperature..." << endl;
  float temp;
  do {
    DS18B20.requestTemperatures(); 
    temp = DS18B20.getTempCByIndex(0);
    delay(100);
  } while (temp == 85.0 || temp == (-127.0));
  return temp;
}

void loop() {
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  float temperature = getTemperature();
  // convert temperature to a string with two digits before the comma and 2 digits for precision
  dtostrf(temperature, 2, 2, temperatureString);
  // send temperature to the MQTT topic
  client.publish(mqtt_topic, temperatureString);
  client.publish(mqtt_topic2, analogRead(A0));
  client.disconnect();
  WiFi.disconnect();
  delay(100);
  ESP.deepSleep(SLEEP_DELAY_IN_SECONDS * 1000000); //, WAKE_RF_DEFAULT);
  delay(500);   // wait for deep sleep to happen */
}
