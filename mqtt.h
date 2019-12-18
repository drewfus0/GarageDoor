#include <WiFiClient.h>
#include <PubSubClient.h>

#define MQTT_USERNAME   "drew"
#define MQTT_KEY        "drew33"
IPAddress mqttIP(10, 1, 1, 10);
#define Garage "garage/door/state"
#define Controller "garage/door/controller"

void mqttMsg(char* topic, byte* payload, unsigned int length){
  String msg = (char*)payload;
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}
WiFiClient wifiClient;
PubSubClient client(mqttIP, 1883, mqttMsg, wifiClient);
long lastReconnectAttempt = 0;

boolean reconnect() {
  if (client.connect("GarageEsp8266", MQTT_USERNAME, MQTT_KEY, Controller, 0, true, "Disconnected")) {
    // Once connected, publish an announcement...
    client.publish(Controller,"Connected");
    // ... and resubscribe
    client.subscribe(Garage);
  }
  return client.connected();
}

void mqttSetup()
{

}

void mqttLoop()
{
  if (!client.connected()) {
    long now = millis();
    if (now - lastReconnectAttempt > 5000) {
      lastReconnectAttempt = now;
      // Attempt to reconnect
      Serial.print("Attempting MQTT connection...");
      if (reconnect()) {
        lastReconnectAttempt = 0;
        Serial.println("Connected");
      }else
      {
          Serial.println("Failed");
      }
      
    }
  } else {
    // Client connected
    
    client.loop();
  }
}