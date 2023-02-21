#ifndef displayController_h
#define displayController_h

#include <Arduino.h>
#include <PxMatrix.h>

#ifdef ESP32
#define P_LAT 22
#define P_A 19
#define P_B 23
#define P_C 18
#define P_D 5
#define P_E 15
#define P_OE 16
#endif
#define matrix_width 64
#define matrix_height 32

class DisplayController {
  
  private:
    PxMATRIX display;
    uint8_t display_draw_time;
    hw_timer_t * timer = NULL;
    portMUX_TYPE timerMux;
    void IRAM_ATTR display_updater();

  public:
    DisplayController();
    void displayTime(String tm, uint8_t y1, uint8_t y2);
    int stepScroll(int lastStep, unsigned long firstTime, uint8_t ypos, unsigned long scroll_delay, String text, uint8_t colorR, uint8_t colorG, uint8_t color);
    void scroll_text(uint8_t ypos, unsigned long scroll_delay, String text, uint8_t colorR, uint8_t colorG, uint8_t colorB, String text2 = "", uint8_t ypos2 = 24);
    // Some standard colors
    uint16_t myRED = display.color565(255, 0, 0);
    uint16_t myGREEN = display.color565(0, 255, 0);
    uint16_t myBLUE = display.color565(0, 0, 255);
    uint16_t myWHITE = display.color565(255, 255, 255);
    uint16_t myYELLOW = display.color565(255, 255, 0);
    uint16_t myCYAN = display.color565(0, 255, 255);
    uint16_t myMAGENTA = display.color565(255, 0, 255);
    uint16_t myBLACK = display.color565(0, 0, 0);

};

#endif
