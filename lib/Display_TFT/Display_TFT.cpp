#include <FS.h>
#include "Display_TFT.h"

TFT_eSPI tft = TFT_eSPI();
char keyLabel[9][10] = {"Too hot", "Hot", "Warm", "Cool", "Cold", "Too cold", "Neutral", "In class", "Send"};
int keyValue[8] = {3, 2, 1, -1, -2, -3, 0, 1};
uint16_t keyColor[9] = {TFT_RED, TFT_ORANGE, TFT_YELLOW, TFT_CYAN, TFT_BLUE, TFT_NAVY, TFT_GREEN, TFT_PURPLE, TFT_WHITE};
TFT_eSPI_Button key[9];

int thermalComfort = 0;
bool inClass = false;
bool sendData = false;

void Display_TFT_init()
{
    tft.init();
    tft.setRotation(1);
    touch_calibrate();
    tft.fillScreen(TFT_DARKGREY);

    tft.fillRect(0, 0, 320, 240, TFT_DARKGREY);
    tft.setTextColor(TFT_WHITE, TFT_DARKGREY);
    tft.setTextSize(2);
    tft.drawString("Select your thermal", 50, 10);
    tft.drawString("comfort level", 90, 30);
    drawKeypad();
}

void Display_TFT_loop()
{
    while (sendData)
    {

        uint16_t t_x = 0, t_y = 0;
        bool pressed = tft.getTouch(&t_x, &t_y);

        for (uint8_t i = 0; i < 9; i++)
        {
            if (pressed && key[i].contains(t_x, t_y))
            {
                key[i].press(true);
            }
            else
            {
                key[i].press(false);
            }
        }

        for (uint8_t i = 0; i < 9; i++)
        {
            if (key[i].justReleased())
                key[i].drawButton();

            if (key[i].justPressed())
            {
                key[i].drawButton(true);

                if (i < 7)
                {
                    thermalComfort = keyValue[i];
                    Serial.println(thermalComfort);
                }
                else if (i == 7)
                {
                    inClass = !inClass;
                    String inClassStr = String("In class: ") + (inClass ? "Yes" : " No");
                    tft.setTextColor(TFT_WHITE, TFT_DARKGREY);
                    tft.setTextSize(0.5);
                    tft.drawString(inClassStr, 90, 220);
                }
                else if (i == 8)
                {
                    sendData = true;
                    tft.setTextColor(TFT_WHITE, TFT_DARKGREY);
                    tft.setTextSize(0.5);
                    tft.drawString("Sending data..", 90, 220);
                }

                delay(10); // Debounce
            }
        }
    }
}

void drawKeypad()
{
    for (uint8_t i = 0; i < 9; i++)
    {
        tft.setFreeFont(LABEL1_FONT);

        uint8_t row = i / 3;
        uint8_t col = i % 3;
        int x_pos = KEY_X + col * (KEY_W + KEY_SPACING_X);
        int y_pos = KEY_Y + row * (KEY_H + KEY_SPACING_Y);

        key[i].initButton(
            &tft,
            x_pos, y_pos,
            KEY_W, KEY_H,
            TFT_BLACK, keyColor[i], TFT_BLACK,
            keyLabel[i], KEY_TEXTSIZE);
        key[i].drawButton();
    }
}

void touch_calibrate()
{
    uint16_t calData[5];
    uint8_t calDataOK = 0;

    if (!SPIFFS.begin())
    {
        Serial.println("Formatting file system");
        SPIFFS.format();
        SPIFFS.begin();
    }

    if (SPIFFS.exists(CALIBRATION_FILE))
    {
        if (REPEAT_CAL)
        {
            SPIFFS.remove(CALIBRATION_FILE);
        }
        else
        {
            File f = SPIFFS.open(CALIBRATION_FILE, "r");
            if (f)
            {
                if (f.readBytes((char *)calData, 14) == 14)
                    calDataOK = 1;
                f.close();
            }
        }
    }

    if (calDataOK && !REPEAT_CAL)
    {
        tft.setTouch(calData);
    }
    else
    {
        tft.fillScreen(TFT_BLACK);
        tft.setCursor(20, 0);
        tft.setTextFont(2);
        tft.setTextSize(1);
        tft.setTextColor(TFT_WHITE, TFT_BLACK);

        tft.println("Touch corners as indicated");

        tft.setTextFont(1);
        tft.println();

        if (REPEAT_CAL)
        {
            tft.setTextColor(TFT_RED, TFT_BLACK);
            tft.println("Set REPEAT_CAL to false to stop this running again!");
        }

        tft.calibrateTouch(calData, TFT_MAGENTA, TFT_BLACK, 15);

        tft.setTextColor(TFT_GREEN, TFT_BLACK);
        tft.println("Calibration complete!");

        File f = SPIFFS.open(CALIBRATION_FILE, "w");
        if (f)
        {
            f.write((const unsigned char *)calData, 14);
            f.close();
        }
    }
}