#ifndef DISPLAY_TFT_H
#define DISPLAY_TFT_H

#include <TFT_eSPI.h>

// Definiciones de constantes
#define CALIBRATION_FILE "/TouchCalData1"
#define REPEAT_CAL false

#define KEY_X 55
#define KEY_Y 80
#define KEY_W 95
#define KEY_H 40
#define KEY_SPACING_X 10
#define KEY_SPACING_Y 15
#define KEY_TEXTSIZE 1
#define SCREEN_WIDTH 320

#define LABEL1_FONT &FreeMonoBold9pt7b

extern int thermalComfort;
extern bool inClass;

// Declaración de funciones
void Display_TFT_init();
void Display_TFT_loop();
void touch_calibrate();
void drawKeypad();

#endif // DISPLAY_TFT_H