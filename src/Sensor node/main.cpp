#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include <Arduino.h>
#include <WiFi.h>
#include "DHT.h"

#define DHTTYPE DHT11
#define DHTPIN 4

const int relay = 26;

DHT dht(DHTPIN, DHTTYPE);
WebSocketsClient webSocket;

const char *ssid = "your_SSID";
const char *password = "your_PASSWORD";
const char *websocket_server = "your_server_ip";

void webSocketEvent(WStype_t type, uint8_t *payload, size_t length);

void setup()
{
    Serial.begin(115200);
    pinMode(relay, OUTPUT);
    dht.begin();

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }

    webSocket.begin(websocket_server, 8765, "/");
    webSocket.onEvent(webSocketEvent);
}

void loop()
{
    webSocket.loop();

    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if (isnan(h) || isnan(t))
    {
        Serial.println(F("Failed to read from DHT sensor!"));
        return;
    }

    // Create a JSON object
    JsonDocument doc;
    doc["type"] = "New environment data";
    doc["temperature"] = t;
    doc["humidity"] = h;

    // Serialize JSON to string
    String json;
    serializeJson(doc, json);

    // Send JSON string
    webSocket.sendTXT(json);

    delay(2000); // Send data every 2 seconds
}

void webSocketEvent(WStype_t type, uint8_t *payload, size_t length)
{
    switch (type)
    {
    case WStype_DISCONNECTED:
        Serial.println("WebSocket Disconnected");
        break;
    case WStype_CONNECTED:
        Serial.println("WebSocket Connected");
        break;
    case WStype_TEXT:
        Serial.printf("Received message: %s\n", payload);
        // Handle the received message here
        {
            String action = String((char *)payload);
            if (action == "turn_on_cooler")
            {
                Serial.println("Action: Turn on cooler");
                digitalWrite(relay, LOW);
            }
            else if (action == "turn_off_cooler")
            {
                Serial.println("Action: Turn off cooler");
                digitalWrite(relay, HIGH);
            }
        }
        break;
    case WStype_BIN:
        Serial.println("Received binary data");
        break;
    }
}