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
    Serial.println("Connected to WiFi");
}

void loop()
{
    webSocket.loop();

    static unsigned long lastSendTime = 0;
    if (millis() - lastSendTime > 5000)
    {
        lastSendTime = millis();

        float h = dht.readHumidity();
        float t = dht.readTemperature();

        if (isnan(h) || isnan(t))
        {
            Serial.println(F("Failed to read from DHT sensor!"));
            return;
        }

        Serial.print("Humidity: ");
        Serial.print(h);
        Serial.print("%  Temperature: ");
        Serial.println(t);

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
        Serial.println("Data sent to server.");
    }
}

void webSocketEvent(WStype_t type, uint8_t *payload, size_t length)
{
    switch (type)
    {
    case WStype_DISCONNECTED:
        Serial.println("WebSocket Disconnected. Reconnecting...");
        webSocket.begin(websocket_server, 8765, "/");
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
                digitalWrite(relay, HIGH);
            }
            else if (action == "turn_off_cooler")
            {
                Serial.println("Action: Turn off cooler");
                digitalWrite(relay, LOW);
            }
        }
        break;
    case WStype_BIN:
        Serial.println("Received binary data");
        break;
    case WStype_PING:
        Serial.println("Received ping");
        break;
    case WStype_PONG:
        Serial.println("Received pong");
        break;
    default:
        break;
    }
}