#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ESPmDNS.h>

// Update these with values suitable for your network.
//const char* ssid = "3bits_203-2.4Ghz";
//const char* password = "Apto!203@)#";
const char* ssid = "Network_Mustard";
const char* password = "458@33ns!";
const char* mqtt_server = "mqtt.tago.io";
#define mqtt_port 1883
#define MQTT_USER "esp32"
#define MQTT_PASSWORD "e8e008b2-edba-4954-bd58-6111f26c8d41"
#define MQTT_SERIAL_PUBLISH_CH "Traffic_light/sensors/presence"
#define MQTT_SERIAL_RECEIVER_CH "Traffic_light/light_color"

//
int presence=23; //pin definition for the sensor_obstacle_ir

WiFiClient wifiClient;

PubSubClient client(wifiClient);

void setup_wifi() {
    delay(10);
    // We start by connecting to a WiFi network
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    randomSeed(micros());
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(),MQTT_USER,MQTT_PASSWORD)) {
      Serial.println("connected");
      //Once connected, publish an announcement...
      client.publish("Traffic_light/sensors/presence", "hello world");
      // ... and resubscribe
      client.subscribe(MQTT_SERIAL_RECEIVER_CH);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void callback(char* topic, byte *payload, unsigned int length) {
    Serial.println();
    Serial.println("-------new message from broker-----");
    Serial.print("channel:");
    Serial.println(topic);
    Serial.print("data:");  
    Serial.write(payload, length);
    Serial.println();
    Serial.println();
    if(digitalRead(presence) == HIGH and (char)payload[0] == 'G') {
      Serial.print("1");
      publishSerialData("1");
      Serial.println();
      delay(2000);    
    }
    if(((digitalRead(presence) == HIGH) or (digitalRead(presence) == LOW)) and (char)payload[0] == 'R') {
      Serial.print("Pedestrian crossing is available");
      Serial.println();
      delay(1000); 
    }
}


//---------------SETUP---------------
void setup() {
  Serial.begin(115200);
  Serial.setTimeout(500);// Set time out for 
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  reconnect();
  
  //Set sensor pin input
  pinMode(presence,INPUT);
}

void publishSerialData(char *serialData){
  if (!client.connected()) {
    reconnect();
  }
  client.publish(MQTT_SERIAL_PUBLISH_CH, serialData);
}

void loop() {
   client.loop();
}
