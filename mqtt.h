#include <WiFiClient.h>
#include <PubSubClient.h>

#define MQTT_USERNAME   "drew"
#define MQTT_KEY        "drew33"
IPAddress mqttIP(10, 1, 1, 10);
#define MQTT_NAME       "GarageTest"
#define Garage "garage/door2/state"
#define Controller "garage/door2/controller"
#define Trigger "garage/door2/Trigger"

void (*DoorToggle)() = 0;

void MqttSetFuctionDoor(void (*fp)()) {
  DoorToggle = fp;
}

void DoorT(){
  if(0!=DoorToggle){
    (*DoorToggle)();
  } else
  {
    Serial.print("Error DoorToggle Not found");
  }
  
}
void mqttMsg(char* topic, byte* payload, unsigned int length);

WiFiClient wifiClient;
PubSubClient client(mqttIP, 1883, mqttMsg, wifiClient);
long lastReconnectAttempt = 0;

void mqttMsg(char* topic, byte* payload, unsigned int length){
  String msg;
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
    msg += (char)payload[i];
  }
  Serial.println();
  if ( strcmp(topic,Trigger)==0 )
  {
      Serial.print("Trigger :");
      Serial.print(msg);
      Serial.println();
      if (msg.equals("now"))
      {
          Serial.println("OpenDoor;");
          client.publish(Trigger,"Done");
          DoorT();
      }
  }
}

boolean reconnect() {
  if (client.connect(MQTT_NAME, MQTT_USERNAME, MQTT_KEY, Controller, 0, true, "Disconnected")) {
    // Once connected, publish an announcement...
    client.publish(Controller,"Connected", true);
    // ... and resubscribe
    client.subscribe(Garage);
    client.subscribe(Trigger);
  }
  return client.connected();
}

void mqttSetup()
{
  
}
long temper = 0;
void mqttLoop()
{
  if (!client.connected()) {
    long now = millis();
    if (now - lastReconnectAttempt > 5000) {
      lastReconnectAttempt = now;
      // Attempt to reconnect
      Serial.print("Attempting MQTT connection...");
      if (reconnect()) {
        //lastReconnectAttempt = 0;
        //Serial.print(now);
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