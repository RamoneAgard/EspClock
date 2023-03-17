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
    // Fields //
    PxMATRIX display;
    
    hw_timer_t * timer = NULL;
    void (*interruptFunc)() = NULL;
 
  public:
    // Fields //
    static const uint8_t display_draw_time = 70;

    // Constructor //
    DisplayController();

    // Methods //
    void startDisplayController();
    void setInterruptFunc(void (f)());
    void show(uint8_t draw_time);
    
    void displayTime(char* tm, uint8_t y1, uint8_t y2);
    int stepScroll(int lastStep, unsigned long firstTime, byte scroll_delay, uint8_t ypos,  char* text, uint8_t colorR, uint8_t colorG, uint8_t colorB);
    void scroll_text(uint8_t ypos, unsigned long scroll_delay, String text, uint8_t colorR, uint8_t colorG, uint8_t colorB, String text2 = "", uint8_t ypos2 = 24);
    void printToScreen(uint8_t ypos, char* text);
    void clearScreen();
    void setScreenBrightness(uint8_t brightness);

    // Some standard colors
    uint16_t myRED;
    uint16_t myGREEN;
    uint16_t myBLUE;
    uint16_t myWHITE;
    uint16_t myYELLOW;
    uint16_t myCYAN;
    uint16_t myMAGENTA;
    uint16_t myBLACK;

};

#endif
