#include <WebSocketsClient.h>
#include <ArduinoJson.h>
#include <MFRC522.h>
#include <Arduino.h>
#include <WiFi.h>
#include <SPI.h>

#define SS_PIN 10
#define RST_PIN 9

MFRC522 rfid(SS_PIN, RST_PIN);
WebSocketsClient webSocket;

const char *ssid = "your_SSID";
const char *password = "your_PASSWORD";
const char *websocket_server = "your_server_ip";

// Function declarations
String uidToDecString(byte *buffer, byte bufferSize);

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
    int tempThermalFeeling = 0;
    int tempInClass = 1;

    // Create a JSON object
    StaticJsonDocument<200> doc;
    doc["type"] = "New scan";
    doc["UID"] = uid;
    doc["Thermal Feeling"] = tempThermalFeeling;
    doc["In Class"] = tempInClass;

    // Serialize JSON to string
    String json;
    serializeJson(doc, json);

    // Send JSON string
    webSocket.sendTXT(json);
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
    // Handle WebSocket events here
}