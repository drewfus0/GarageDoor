
#include <ESP8266mDNS.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>


#ifndef STASSID
#define STASSID "ImWifiRick"
#define STAPSK  "1234567890"
#endif

int relay_pin = D1;

const char *ssid = STASSID;
const char *password = STAPSK;

ESP8266WebServer server(80);

const int led = D4;

void handleRoot(){
  returnPage("...");
}

void returnPage(String str) {
  digitalWrite(led, 1);
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;

  String tmp1 = "<!DOCTYPE html>\
  <html>\
  <head>\
    <title>ESP8266 Swtich.</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>Hello from ESP8266</h1>\
    <p>Uptime: " + String(hr) + ":" + String(min % 60) + ":" + String(sec % 60) + "</p>\
    ";
    String tmp2;
    tmp2 = "Door Is Open";
    if (!digitalRead(D7))
    {
      tmp2 = "Door Is Closed";
    }
    String tmp3 = "<p><button type='button' onclick=\"window.location.href = '\\smoke';\">Smoke the Coffin!</button></p>\
    <p>" + str + "</p>\
    <img src='/test.svg' />\
  </body>\
</html>";
  String temp = tmp1 + tmp2 + tmp3;
  server.send(200, "text/html", temp);
  digitalWrite(led, 0);
}

void handleNotFound() {
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }

  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}

void setup(void) {
  pinMode(led, OUTPUT);
  pinMode(relay_pin, OUTPUT);
  digitalWrite(led, 0);
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

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);
  server.on("/test.svg", drawGraph);
  server.on("/inline", []() {
    server.send(200, "text/plain", "this works as well");
  });
  server.on("/smoke", promptDoor);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
}

void loop(void) {
  server.handleClient();
  MDNS.update();
}

void promptDoor() {
  digitalWrite(led, 1);
  digitalWrite(relay_pin, HIGH);
  delay(100);
  digitalWrite(relay_pin, LOW);
  digitalWrite(led, 0);
  delay(1000);
  server.sendHeader("Location", "/",true);
  server.send(302,"text/plain","");
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

  server.send(200, "image/svg+xml", out);
}
