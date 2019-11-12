
#include <ESP8266mDNS.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <FS.h>


#ifndef STASSID
#define STASSID "ImWifiRick"
#define STAPSK  "1234567890"
#endif

int relay_pin = D1;

const char *ssid = STASSID;
const char *password = STAPSK;

ESP8266WebServer webServer(80);

#include "FSBrowser.h"

const int led = D4;

void handleRoot(){
  returnPage("...");
}

void returnPage(String str) {
  digitalWrite(led, LOW);
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;
  String tmp2;
  tmp2 = "Door Is Open";
  if (!digitalRead(D7))
  {
    tmp2 = "Door Is Closed";
  }

  String temp = tmp1 + tmp2 + tmp3;
  webServer.send(200, "text/html", temp);
  digitalWrite(led, HIGH);
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

  webServer.on("/", handleRoot);
  webServer.on("/test.svg", drawGraph);
  webServer.on("/inline", []() {
    webServer.send(200, "text/plain", "this works as well");
  });
  webServer.on("/garage", promptDoor);

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

void loop(void) {
  webServer.handleClient();
  MDNS.update();
}

void promptDoor() {
  digitalWrite(led, LOW);
  digitalWrite(relay_pin, LOW);
  delay(100);
  digitalWrite(relay_pin, HIGH);
  delay(1000);
  webServer.sendHeader("Location", "/",true);
  webServer.send(302,"text/plain","");
  digitalWrite(led, HIGH);
}

void drawGraph() {
  String out = "";
  char temp[100];
  out += "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" width=\"400\" height=\"150\">\n";
  out += "<rect width=\"400\" height=\"150\" fill=\"rgb(250, 230, 210)\" stroke-width=\"1\" stroke=\"rgb(0, 0, 0)\" />\n";
  out += "<g stroke=\"black\">\n";
  int y = rand() % 130;
  for (int x = 10; x < 390; x += 10) {
    int y2 = rand() % 130;
    sprintf(temp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" stroke-width=\"1\" />\n", x, 140 - y, x + 10, 140 - y2);
    out += temp;
    y = y2;
  }
  out += "</g>\n</svg>\n";

  webServer.send(200, "image/svg+xml", out);
}
