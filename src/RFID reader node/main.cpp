#include <SPI.h>
#include <MFRC522.h>
#include <Arduino.h>

#define SS_PIN 10
#define RST_PIN 9

MFRC522 rfid(SS_PIN, RST_PIN);

// Function declarations
String uidToDecString(byte *buffer, byte bufferSize);

void setup()
{
    Serial.begin(115200);
    SPI.begin();     // Init SPI bus
    rfid.PCD_Init(); // Init MFRC522
}

void loop()
{
    if (!rfid.PICC_IsNewCardPresent())
        return;

    // Verify if the NUID has been readed
    if (!rfid.PICC_ReadCardSerial())
        return;

    String uid = uidToDecString(rfid.uid.uidByte, rfid.uid.size);
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