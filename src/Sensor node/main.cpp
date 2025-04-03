#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include <Arduino.h>
#include <WiFi.h>
#include "DHT.h"
#include <AESLib.h>  // Incluir librería AES

#define DHTTYPE DHT11
#define DHTPIN 4

const int relay = 26;

DHT dht(DHTPIN, DHTTYPE);
WebSocketsClient webSocket;
AESLib aesLib;

const char *ssid = "your_SSID";
const char *password = "your_PASSWORD";
const char *websocket_server = "your_server_ip";

// Clave en formato hexadecimal: 01 23 45 67 89 ab cd ef 01 23 45 67 89 ab cd ef
byte key1[16] = { 
    0x01, 0x23, 0x45, 0x67,
    0x89, 0xab, 0xcd, 0xef, 
    0x01, 0x23, 0x45, 0x67,
    0x89, 0xab, 0xcd, 0xef };  // Clave en formato de byte
byte iv[16] = {
    0x02, 0x03, 0x05, 0x07,
    0x09, 0x0b, 0x0d, 0xaf, 
    0x10, 0x32, 0x54, 0x76, 
    0x98, 0xab, 0xcd, 0xef};  // Vector de inicialización (IV), se puede personalizar


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

    // Convertir a char* con padding
    int jsonLen = json.length() + 1;
    char plainText[jsonLen];
    json.toCharArray(plainText, jsonLen);

    // Redondear al múltiplo de 16
    int paddedLen = ((jsonLen + 15) / 16) * 16;
    for (int i = jsonLen; i < paddedLen; i++) plainText[i] = '\0';

    // Encriptar con AES-128 CBC
    byte encrypted[paddedLen];
    aesLib.encrypt((const byte*)plainText, paddedLen, encrypted, key1, 128, iv);

    // Enviar datos cifrados al servidor por WebSocket
    webSocket.sendBIN(encrypted, paddedLen);

    delay(2000); // Enviar datos cada 2 segundos
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
