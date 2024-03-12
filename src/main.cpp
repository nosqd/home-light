#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266mDNS.h>
#include <Adafruit_NeoPixel.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.hpp>
#include "config.h"
#ifdef HEXAGON
#include "hexagon.h"
#endif

using namespace ArduinoJson;

WiFiClient espClient;
Adafruit_NeoPixel strip(STRIP_LENGTH, STRIP_PIN, NEO_GRB + NEO_KHZ800);

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len);
void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len);
void setup_wifi()
{
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  server.begin();

}

void setup()
{
  Serial.begin(9600);
  setup_wifi();

  String nodeMDNS = "led-";
  nodeMDNS += NODE_ID;

  if (MDNS.begin(nodeMDNS.c_str()))
  {
    Serial.println("MDNS responder started");
  }

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "application/json", "{\"message\": \"Yea it works... Kinda.\", \"status\": true}");
  });
  server.addHandler(&ws);
  ws.onEvent(onEvent);

  strip.begin();
  strip.show();
}

void loop()
{
  ws.cleanupClients();
}

void handleWebSocketMessage(AsyncWebSocketClient *client, void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    data[len] = 0;
    char* szData = (char*)data;
    JsonDocument response;
    JsonDocument request;

    deserializeJson(request, szData);
    
    String method = request["method"];

    #ifdef HEXAGON
    if (method == "hexagon_clear") {
      int hex = request["hexagon"];
      uint8_t r = request["r"];
      uint8_t g = request["g"];
      uint8_t b = request["b"];
      uint32_t color = strip.Color(r,g,b);
      hexagon_clear(strip, hex, color);
      Serial.printf("Clearing hexagon %d with color %d\n", hex, color);
      response["ok"] = true;
    }
    else if (method == "hexagon_set") {
      int hex = request["hexagon"];
      int pix = request["pixel"];
      uint8_t r = request["r"];
      uint8_t g = request["g"];
      uint8_t b = request["b"];
      uint32_t color = strip.Color(r,g,b);
      Serial.printf("Setting hexagon %d pixel %d with color %d\n", hex, pix, color);

      hexagon_set(strip, hex, pix, color);
      response["ok"] = true;
    }
    #endif
    #ifdef STRIPS
    if (method == "strip_clear") {
      uint8_t r = request["r"];
      uint8_t g = request["g"];
      uint8_t b = request["b"];
      uint32_t color = strip.Color(r,g,b);
      Serial.printf("Clearing strip with color %d\n", color);
      for (int i = 0; i < STRIP_LENGTH; i++) strip.setPixelColor(i, color);
      response["ok"] = true;
    }
    else if (method == "strip_set") {
      uint8_t r = request["r"];
      uint8_t g = request["g"];
      uint8_t b = request["b"];
      uint32_t color = strip.Color(r,g,b);
      int pix = request["pixel"];
      Serial.printf("Setting strip pixel %d with color %d\n", pix, color);
      for (int i = 0; i < STRIP_LENGTH; i++) strip.setPixelColor(i, color);
      response["ok"] = true;
    }
    #endif
    else {
      response["ok"] = false;
    }
    strip.show();
    String buf;
    serializeJson(response, buf);

    Serial.printf("< %s\n", szData);
    Serial.printf("> %s\n", buf.c_str());
    client->text(buf);
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
    switch (type) {
      case WS_EVT_CONNECT:
        Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
        break;
      case WS_EVT_DISCONNECT:
        Serial.printf("WebSocket client #%u disconnected\n", client->id());
        break;
      case WS_EVT_DATA:
        handleWebSocketMessage(client, arg, data, len);
        break;
      case WS_EVT_PONG:
      case WS_EVT_ERROR:
        break;
  }
}