#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

// Update these with values suitable for your network.
//const char* ssid = "3bits_203-2.4Ghz";
//const char* password = "Apto!203@)#";
const char* ssid = "Network_Mustard";
const char* password = "458@33ns!";
const char* mqtt_server = "mqtt.tago.io";
#define mqtt_port 1883
#define MQTT_USER "esp32"
#define MQTT_PASSWORD "e8e008b2-edba-4954-bd58-6111f26c8d41"
#define MQTT_SERIAL_PUBLISH_CH "Traffic_light/light_color"
#define MQTT_SERIAL_RECEIVER_CH "Traffic_light/sensors/presence"

//D0, D1, D2
int red_light=16, yellow_light=5, green_light=4; //pin definition for the traffic light
int p_red_light=14, p_yellow_light=12, p_green_light=13; //pin definition for the pedestrian traffic light
//D5, D6, D7

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

void yellow_red() {
  digitalWrite(red_light,LOW);
  digitalWrite(yellow_light,HIGH);
  digitalWrite(green_light,LOW);
  publishSerialData("Yellow");
  Serial.print("Yellow");
  Serial.println();
  delay(2000);
  
  digitalWrite(red_light,HIGH);
  digitalWrite(p_red_light,LOW);
  digitalWrite(yellow_light,LOW);
  digitalWrite(green_light,LOW);  
  digitalWrite(p_green_light,HIGH);
  publishSerialData("Red");
  Serial.print("Red");
  Serial.println();
  Serial.print("Pedestrian crossing is available!");
  Serial.println(); 
  delay(10000);
  Serial.print("Pedestrian crossing time out!");
  Serial.println();
  for(int cont = 0; cont < 4; cont++){
    p_red_blink();
  }  
}

void p_red_blink(){
  delay(250);
  digitalWrite(p_red_light,HIGH);
  digitalWrite(p_green_light,LOW);
  delay(250);
}

void green() {
  digitalWrite(red_light,LOW);
  digitalWrite(yellow_light,LOW);
  digitalWrite(green_light,HIGH);
  Serial.print("Green");
  Serial.println(); 
  delay(4000);
  publishSerialData("Green");
  Serial.print("Publish: Green");
  Serial.println(); 
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
  if((char)payload[0] == '1'){
    yellow_red();
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
  
  //Set Pins as outputs
  pinMode(red_light,OUTPUT);
  pinMode(yellow_light,OUTPUT);
  pinMode(green_light,OUTPUT);

  //Start the program with all light off
  digitalWrite(red_light,LOW);
  digitalWrite(yellow_light,LOW);
  digitalWrite(green_light,LOW);
}

void publishSerialData(char *serialData){
  if (!client.connected()) {
    reconnect();
  }
  client.publish(MQTT_SERIAL_PUBLISH_CH, serialData);
}

void loop() {
   client.loop();
   green();
}
