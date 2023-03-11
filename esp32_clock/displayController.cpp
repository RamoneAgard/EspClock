#include "displayController.h"

DisplayController::DisplayController()
  : display(matrix_width, matrix_height, P_LAT, P_OE, P_A, P_B, P_C, P_D) {
  myRED = display.color565(255, 0, 0);
  myGREEN = display.color565(0, 255, 0);
  myBLUE = display.color565(0, 0, 255);
  myWHITE = display.color565(255, 255, 255);
  myYELLOW = display.color565(255, 255, 0);
  myCYAN = display.color565(0, 255, 255);
  myMAGENTA = display.color565(255, 0, 255);
  myBLACK = display.color565(0, 0, 0);
  //pixel Setup
}

void DisplayController::startDisplayController(){
  display.begin(16);
  display.flushDisplay();
  #ifdef ESP32
    timer = timerBegin(0, 80, true);
    timerAttachInterrupt(timer, interruptFunc, true);
    timerAlarmWrite(timer, 2000, true);
    timerAlarmEnable(timer);
  #endif
}

void DisplayController::displayTime(char tm[], uint8_t y1, uint8_t y2){
  int split = tm.indexOf('\n');
  String top = tm.substring(0, split);
  String bottom = tm.substring(split + 1, 19) + tm.substring(20);
  display.setTextSize(1);
  display.setTextWrap(true);
  display.setCursor(2, y1);
  //display.setTextColor(myGREEN);
  //display.clearDisplay();
  display.fillRect(0, y1, 64, 8, display.color565(0, 0, 0));
  display.print(top);
  display.setCursor(2, y2);
  display.fillRect(0, y2, 64, 8, display.color565(0, 0, 0));
  display.println(bottom);
  return;
}

// Single line Step Scrolling that operates inside a loop
// must also input the start time of the scroll (firstTime) and
// the last step (lastStep) in the scroll function returned by the last call
// (zero for the first call to the function)
// return is -1 if scrolling is done
int DisplayController::stepScroll(int lastStep, unsigned long firstTime, uint8_t ypos, unsigned long scroll_delay, String text, uint8_t colorR, uint8_t colorG, uint8_t colorB){
  if (millis() >= firstTime + (scroll_delay * lastStep)) {
    uint16_t text_length = text.length();
    int startStep = matrix_width;
    int endStep = -(5 + (text_length * 6));
    display.setTextWrap(false);  // we don't wrap text so it scrolls nicely
    display.setTextSize(1);
    display.setRotation(0);
    display.fillRect(0, ypos, 64, 8, display.color565(0, 0, 0));
    display.setTextColor(display.color565(colorR, colorG, colorB));
    int xpos = startStep - lastStep;
    if (xpos <= endStep) {
      display.setCursor(0, ypos);
      return -1;
    }
    display.setCursor(xpos, ypos);
    display.println(text);
    return (lastStep + 1);
  }
  return lastStep;
}

void DisplayController::printToScreen(uint8_t ypos, char* text){
  display.fillRect(0, ypos, 64, 8, display.color565(0, 0, 0));
  display.setCursor(0, ypos);
  display.println(text);
}


//Full Screen Text Scroll -- From PxMatrix Library examples //
void DisplayController::scroll_text(uint8_t ypos, unsigned long scroll_delay, String text, uint8_t colorR, uint8_t colorG, uint8_t colorB, String text2, uint8_t ypos2){
  uint16_t text_length = text.length();
  bool two = (text2.length() > 0 ) ? true : false;
  if (text2.length() > text_length) {
    text_length = text2.length();
  }
  display.setTextWrap(false);  // we don't wrap text so it scrolls nicely
  display.setTextSize(1);
  display.setRotation(0);
  display.setTextColor(display.color565(colorR, colorG, colorB));

  // Asuming 5 pixel average character width
  for (int xpos = matrix_width; xpos > -(5 + (text_length * 6)); xpos--)
  {
    display.setTextColor(display.color565(colorR, colorG, colorB));
    display.clearDisplay();
    display.setCursor(xpos, ypos);
    display.println(text);
    // delay(scroll_delay/2);
    if (two) {
      display.setCursor(xpos, ypos2);
      display.println(text2);
    }
    delay(scroll_delay);
    yield();

    // This might smooth the transition a bit if we go slow
    // display.setTextColor(display.color565(colorR/4,colorG/4,colorB/4));
    // display.setCursor(xpos-1,ypos);
    // display.println(text);

    delay(scroll_delay / 5);
    yield();

  }
}

void DisplayController::setInterruptFunc(void (f)()){
  interruptFunc = f;
}

void DisplayController::show(uint8_t draw_time){
  display.display(draw_time);  
}
