#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include "Display_TFT.h"
#include <MFRC522.h>
#include <Arduino.h>
#include <WiFi.h>
#include <SPI.h>
#include <AESLib.h> // Librería AES compatible con tu versión

#define SS_PIN 5
#define RST_PIN 0

MFRC522 rfid(SS_PIN, RST_PIN);
WebSocketsClient webSocket;
AESLib aesLib;

const char *ssid = "your_SSID";
const char *password = "your_PASSWORD";
const char *websocket_server = "your_server_ip";

// Function declarations
String uidToDecString(byte *buffer, byte bufferSize);
void webSocketEvent(WStype_t type, uint8_t *payload, size_t length);

// Clave AES-128 de 16 bytes
byte key1[16] = {clave de 16 digitos};

// IV de 16 bytes para modo CBC
byte iv[16] = {clave 2 de 16 digitos};

void setup()
{
    Serial.begin(115200);
    SPI.begin();
    rfid.PCD_Init();

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }

    webSocket.begin(websocket_server, 8765, "/");
    webSocket.onEvent(webSocketEvent);
    Serial.println("Connected to WiFi");

    Display_TFT_init();
}

void loop()
{
    webSocket.loop();

    if (!rfid.PICC_IsNewCardPresent())
        return;

    // Verify if the UID has been readed
    if (!rfid.PICC_ReadCardSerial())
        return;

    String uid = uidToDecString(rfid.uid.uidByte, rfid.uid.size);
    Serial.println("UID: " + uid);
    Display_TFT_loop(); // Call the display function

    Serial.println("Sending data to server...");

    // Create a JSON object
    JsonDocument doc;
    doc["type"] = "New scan";
    doc["UID"] = uid;
    doc["Thermal Feeling"] = thermalComfort;
    doc["In Class"] = (inClass) ? 1 : 0;

    // Serialize JSON to string
    String json;
    serializeJson(doc, json);

    // Convertir a char* con padding
    int jsonLen = json.length() + 1;
    char plainText[jsonLen];
    json.toCharArray(plainText, jsonLen);

    // Redondear al múltiplo de 16
    int paddedLen = ((jsonLen + 15) / 16) * 16;
    for (int i = jsonLen; i < paddedLen; i++)
        plainText[i] = '\0';

    // Encriptar con AES-128 CBC
    byte encrypted[paddedLen];
    aesLib.encrypt((const byte *)plainText, paddedLen, encrypted, key1, 128, iv);

    // Enviar datos cifrados al servidor por WebSocket
    webSocket.sendBIN(encrypted, paddedLen);
}

String uidToDecString(byte *buffer, byte bufferSize)
{
    String uid;
    for (byte i = 0; i < bufferSize; i++)
    {
        uid += String(buffer[i], DEC);
        if (i < bufferSize - 1)
        {
            uid += " ";
        }
    }
    return uid;
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