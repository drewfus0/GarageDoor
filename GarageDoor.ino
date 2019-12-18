#include <ESP8266mDNS.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>

#include "Field.h"

//Wifi Stuff
#ifndef STASSID
#define STASSID "ImWifiRick"
#define STAPSK  "1234567890"
#endif


int relay_pin = D1;
int led = D4;
int sensor = D7;
int unk = 0;

const char *ssid = STASSID;
const char *password = STAPSK;

ESP8266WebServer webServer(80);
#include "FSBrowser.h"
#include "mqtt.h"

void returnPage(String str) {
  digitalWrite(led, LOW);
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;

  String temp = DoorStatus();
  webServer.send(200, "text/html",  DoorStatus());
  digitalWrite(led, HIGH);
}

char* DoorStatus2(){
  if (!digitalRead(sensor))
  {
    return "Closed";
  }
  return "Open";
}

String DoorStatus(){
  return String(DoorStatus2());
}

void handleNotFound() {
  digitalWrite(led, LOW);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += webServer.uri();
  message += "\nMethod: ";
  message += (webServer.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += webServer.args();
  message += "\n";

  for (uint8_t i = 0; i < webServer.args(); i++) {
    message += " " + webServer.argName(i) + ": " + webServer.arg(i) + "\n";
  }

  webServer.send(404, "text/plain", message);
  digitalWrite(led, HIGH);
}

void setup(void) {
  pinMode(led, OUTPUT);
  pinMode(relay_pin, OUTPUT);
  digitalWrite(led, HIGH);
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266garage")) {
    Serial.println("MDNS responder started");
  }

  webServer.on("/garage", promptDoor);
  webServer.on("/doorstatus", []() {
    webServer.send(200, "text/html",  DoorStatus());
  });
  SPIFFS.begin();
  {
    Serial.println("SPIFFS contents:");

    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {
      String fileName = dir.fileName();
      size_t fileSize = dir.fileSize();
      Serial.printf("FS File: %s, size: %s\n", fileName.c_str(), String(fileSize).c_str());
    }
    Serial.printf("\n");
    
    // Done when init
    //client.setServer(SERVER, SERVERPORT);
    //client.setCallback(callback);
  }
  //list directory
  webServer.on("/list", HTTP_GET, handleFileList);
  //load editor
  webServer.on("/edit", HTTP_GET, []() {
    if (!handleFileRead("/edit.htm")) webServer.send(404, "text/plain", "FileNotFound");
  });
  //create file
  webServer.on("/edit", HTTP_PUT, handleFileCreate);
  //delete file
  webServer.on("/edit", HTTP_DELETE, handleFileDelete);
  //first callback is called after the request has ended with all parsed arguments
  //second callback handles file uploads at that location
  webServer.on("/edit", HTTP_POST, []() {
    webServer.send(200, "text/plain", "");
  }, handleFileUpload);

  webServer.serveStatic("/", SPIFFS, "/", "max-age=86400");

  webServer.onNotFound(handleNotFound);
  webServer.begin();
  Serial.println("HTTP server started");


  
}
int LastknownSensor;
void CheckDoor() {
  if (digitalRead(sensor) != LastknownSensor)
  {
    /* publish mqtt */
    LastknownSensor = digitalRead(sensor);
    client.publish(Garage, DoorStatus2());
  }
}

void loop(void) {
  webServer.handleClient();
  MDNS.update();
  mqttLoop();
  CheckDoor();
}

void promptDoor() {
  digitalWrite(led, LOW);
  digitalWrite(relay_pin, HIGH);
  delay(100);
  digitalWrite(relay_pin, LOW);
  delay(1000);
  webServer.sendHeader("Location", "/",true);
  webServer.send(302,"text/plain","");
  digitalWrite(led, HIGH);
}