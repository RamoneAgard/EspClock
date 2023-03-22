#include <Arduino.h>
#include <time.h>
#include "webController.h"
#include "displayController.h"

// Message buffers and Fields // 
const byte timeBuffLen = 25;
char timeStorage[timeBuffLen];

const byte numTickers = 5;
char* apiTickers[numTickers] = {"spy", "qqqm", "msft", "swppx", "pacb"};

const int marketApiBuffLen = 25 * numTickers;
char marketApiStorage[marketApiBuffLen];

const byte serverMessageBuffLen = 85;
char serverMessageStorage[serverMessageBuffLen];

char debugMessage[50];

// Main web controller and setup // 
WebController webControl(serverMessageStorage, serverMessageBuffLen);

// Main display controller and setup portion //
DisplayController dispControl;
// field for display setup
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
// update method for display setup
#ifdef ESP32
void IRAM_ATTR display_updater() {
  // Increment the counter and set the time of ISR
  portENTER_CRITICAL_ISR(&timerMux);
  dispControl.show(DisplayController::display_draw_time);
  portEXIT_CRITICAL_ISR(&timerMux);
}
#endif

// Loop Timing Variables //
unsigned long setupWait = 10000;
unsigned long currentMillis;

// unsigned long lastMarketRequest = 0;
// unsigned long marketRequestDelay = 86400000;
bool gotUpdate = false;
const char updateHourFirstDigit = '4';
const char updateMinSecDigit = '3';
const char updateMinFirstDigit = '0';
const char updateSecSecondDigit = '0';
const char updateSecFirstDigit = '5';
unsigned long marketDisplayTime = 0;
int marketDisplayScroll = 0;

unsigned long serverMessageDisplayTime = 0;
int serverMessageDisplayScroll = 0;

unsigned long lastTimeRequest = 0;
int timeRequestDelay = 999;

// Function to check if timeStorage matches the update time
bool isUpdateTime(byte hourFDex, byte minSDex, byte minFDex, byte secSDex, byte secFDex){
  if(timeStorage[hourFDex] == updateHourFirstDigit){
    if(timeStorage[minSDex] == updateMinSecDigit && timeStorage[minFDex] == updateMinFirstDigit){
      if(timeStorage[secSDex] == updateSecSecondDigit && timeStorage[secFDex] == updateSecFirstDigit){
        return true;
      }
    }
  }
  return false;
}

void setup(){
  Serial.begin(115200);

  //Setup the web controller
  // Serial.println("Attempting internet connection");
  webControl.startServerAndClient();
  webControl.getIpAddress(debugMessage); 

  //Setup the display controller
  dispControl.setInterruptFunc(display_updater);
  dispControl.startDisplayController();
  dispControl.setScreenBrightness(50);

  //Show IP Address  
  dispControl.printToScreen(0, "IP Address:");  
  dispControl.printToScreen(8, debugMessage, true);
    
  // Serial.println(debugMessage);

  //Get inital market data
  // Serial.print("Attempt Market Request ");
  webControl.getMarket(marketApiStorage, apiTickers, numTickers);

  //Get inital time data 
  webControl.getNptTime(timeStorage, timeBuffLen);

  unsigned long curr = millis();
  while((millis()-curr) <= setupWait){}
  // Serial.println("END SetUp");
  dispControl.clearScreen();
}

/*
Colors in the form (g,b,r) for old board
Colors in form (r,g,b) for new board
*/
//d
void loop(){
  // delay(100);
  currentMillis = millis();
  // Time update //
  if((unsigned long)(currentMillis - lastTimeRequest) >= timeRequestDelay){
    lastTimeRequest = currentMillis;
    webControl.getNptTime(timeStorage, timeBuffLen);
    // Serial.println(timeStorage);
    dispControl.displayTime(timeStorage, 0, 24, 0, 255 , 143);
  }
  
  // Market api update //
  if(isUpdateTime(12, 14, 15, 17, 18)){
    webControl.getMarket(marketApiStorage, apiTickers, numTickers);
  }
  
  // if((unsigned long)(currentMillis - lastMarketRequest) >= marketRequestDelay){
  //   lastMarketRequest = currentMillis;
  //   webControl.getMarket(marketApiStorage, apiTickers, numTickers);
  //   // Serial.println(marketApiStorage);
  // }

  // Server message updating //
  webControl.updateServer();

  // Display Loop Functions // 

  // Market Api Display
  // ( int stepScroll(int lastStep, unsigned long firstTime, byte scroll_delay, uint8_t ypos,  char* text, uint8_t colorR, uint8_t colorG, uint8_t colorB); )
  if(marketDisplayScroll < 0){
    marketDisplayScroll = 0;
    marketDisplayTime = currentMillis;
  }
  marketDisplayScroll = dispControl.stepScroll(marketDisplayScroll, marketDisplayTime, 150, 8, marketApiStorage, 215, 0, 79);

  // Server Message display
  if(serverMessageDisplayScroll < 0){
    serverMessageDisplayScroll = 0;
    serverMessageDisplayTime = currentMillis;
  }
  serverMessageDisplayScroll = dispControl.stepScroll(serverMessageDisplayScroll, serverMessageDisplayTime, 75, 16, serverMessageStorage, 137, 0, 255);

}

/* OLD CODE  */

/*
#include <Arduino.h>
#include <PxMatrix.h>
#include <time.h>
#include "webController.h"

//#include <NTPClient.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WiFiClient.h>
//#include <WiFiUdp.h>
#include <HTTPClient.h>
#include "indexLED.h"
#include "info.h"

#define USE_SERIAL Serial

WebServer server(80);
String my_ip = "";

#ifdef ESP32

#define P_LAT 22
#define P_A 19
#define P_B 23
#define P_C 18
#define P_D 5
#define P_E 15
#define P_OE 16
hw_timer_t * timer = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;

#endif
//WiFiUDP ntpUDP;
//NTPClient timeClient(ntpUDP, "north-america.pool.ntp.org", -21600);
#define matrix_width 64
#define matrix_height 32
String wifiMessage = "";

const char* tickers[] = {"spy", "qqqm", "msft", "swppx", "pacb"};
const byte numTicks = 5;
bool needMarketUpdate = false;

uint8_t display_draw_time = 70;

//PxMATRIX display(32,16,P_LAT, P_OE,P_A,P_B,P_C);
PxMATRIX display(64, 32, P_LAT, P_OE, P_A, P_B, P_C, P_D);
//PxMATRIX display(64,64,P_LAT, P_OE,P_A,P_B,P_C,P_D,P_E);

// Some standard colors
uint16_t myRED = display.color565(255, 0, 0);
uint16_t myGREEN = display.color565(0, 255, 0);
uint16_t myBLUE = display.color565(0, 0, 255);
uint16_t myWHITE = display.color565(255, 255, 255);
uint16_t myYELLOW = display.color565(255, 255, 0);
uint16_t myCYAN = display.color565(0, 255, 255);
uint16_t myMAGENTA = display.color565(255, 0, 255);
uint16_t myBLACK = display.color565(0, 0, 0);

//Setup display updating
#ifdef ESP32
void IRAM_ATTR display_updater() {
  // Increment the counter and set the time of ISR
  portENTER_CRITICAL_ISR(&timerMux);
  display.display(display_draw_time);
  portEXIT_CRITICAL_ISR(&timerMux);
}
#endif

String getTime() {
  time_t now;
  time(&now);
  struct tm tStruct =  *localtime(&now);
  if(tStruct.tm_hour == 16 && tStruct.tm_min == 30){
    if(tStruct.tm_sec <= 2){
      needMarketUpdate = true;
    }
  }
  char t_out[30];
  strftime(t_out, 30, "%a %b %d\n%r", &tStruct); // Mon Apr 13 12:30:45 pm
  return t_out;
}

void displayTime(String tm, uint8_t y1, uint8_t y2) {
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

String getMarket(String ticker) {
  String ticker_p;
  if ((WiFi.status() == WL_CONNECTED)) {

    HTTPClient http;

    String urlStart = "https://www.alphavantage.co/query?function=GLOBAL_QUOTE&symbol=";
    String target = urlStart + ticker + apiEnd;

    USE_SERIAL.print("[HTTP] begin...\n");
    // configure traged server and url
    //http.begin("https://www.howsmyssl.com/a/check", ca); //HTTPS
    http.begin(target); //HTTP

    USE_SERIAL.print("[HTTP] GET...\n");
    // start connection and send HTTP header
    int httpCode = http.GET();

    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      USE_SERIAL.printf("[HTTP] GET... code: %d\n", httpCode);

      // file found at server
      if (httpCode == HTTP_CODE_OK) {
        //String payload = http.getString();
        //Stream& input = http.getString();
        StaticJsonDocument<512> doc;
        DeserializationError error = deserializeJson(doc, http.getString() );

        if (error) {
          USE_SERIAL.print(F("deserializeJson() failed: "));
          USE_SERIAL.println(error.f_str());
          ticker_p = "error";
          return ticker_p;
        }

        JsonObject Global_Quote = doc["Global Quote"];
        const char* symbol = Global_Quote["01. symbol"]; // "QQQM"
        const char* _open = Global_Quote["02. open"]; // "130.3600"
        const char* high = Global_Quote["03. high"]; // "132.7600"
        const char* low = Global_Quote["04. low"]; // "127.8850"
        const char* price = Global_Quote["05. price"]; // "132.1600"
        const char* volume = Global_Quote["06. volume"]; // "209936"
        const char* latest_trading_day = Global_Quote["07. latest trading day"]; // "2021-02-23"
        const char* previous_close = Global_Quote["08. previous close"]; // "132.5500"
        const char* change = Global_Quote["09. change"]; // "-0.3900"
        const char* change_percent = Global_Quote["10. change percent"]; // "-0.2942%"
//        USE_SERIAL.println(symbol);
//        USE_SERIAL.print("Price: ");
//        USE_SERIAL.println(price);
        String price2 = String(price);
        String change2 = String(change);
        ticker_p = String(symbol) + ":" + (price2.substring(0, price2.length() - 2)) + " " + (change2.substring(0, change2.length() - 2));
      }
    } else {
      USE_SERIAL.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      ticker_p = "error";
    }

    http.end();
  }
  else {
    USE_SERIAL.println("Wifi not connected");
    ticker_p = "error";
  }
  return ticker_p;
}


//Server Functions
void handleRoot() {
  server.send(200, "text/html", INDEX_HTML);
}

void handleMessage() {
  String response = "<html> \n <body> \n <h1>NO Message Received</h1>\n <br>\n <h4>send me a message with " + my_ip + "/message?m=ENTER_HERE</h4> \n </body> </html>";
  if (server.args() >= 1 && server.argName(0) == "m") {
    char inputArgBuff[32];
    server.arg("m").toCharArray(inputArgBuff, 31);
    wifiMessage = String(inputArgBuff);
    if (wifiMessage == "" || wifiMessage == " ") {
      wifiMessage = "Send a message to " + my_ip;
    }
    //response = "<html> \n <body> \n <h1>Message Received</h1>\n <br>\n <h4>send me a message with " + my_ip+ "/message?m=ENTER_HERE</h4> \n </body> </html>";
    server.send(200, "text/html", INDEX_HTML);
  } else {
    server.send(200, "text/html", response);
  }

}

void handleNotFound() {
  server.send(404, "text/plain", "Request Not Found");
}


//Full Screen Text Scroll -- From PxMatrix Library examples
void scroll_text(uint8_t ypos, unsigned long scroll_delay, String text, uint8_t colorR, uint8_t colorG, uint8_t colorB, String text2 = "", uint8_t ypos2 = 24)
{
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

//Single line Step Scrolling that operates inside a loop
// must also input the start time of the scroll (firstTime) and
// the last step (lastStep) in the scroll function returned by the last call
// (zero for the first call to the function)
// return is -1 if scrolling is done
int stepScroll(uint8_t ypos, unsigned long scroll_delay, String text, uint8_t colorR, uint8_t colorG, uint8_t colorB, unsigned long firstTime, int lastStep = 0) {
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


//timing variables
// last trade request and request interval 
unsigned long lastTR = 0;
unsigned long inTR = 999;
// last stock request and request interval 
unsigned long lastSR = 0;
unsigned long inSR = 3600000;

String stocks = "";

///Setup
void setup() {
  USE_SERIAL.begin(115200);

  WebController webControl()

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    USE_SERIAL.print(".");
  }
  my_ip = WiFi.localIP().toString();
  wifiMessage = "Send a message to " + my_ip;

  server.on("/", handleRoot);
  server.on("/message", handleMessage);
  server.onNotFound(handleNotFound);
  server.begin();

  display.begin(16);
  display.flushDisplay();
  USE_SERIAL.println("before");

  //pixel Setup
#ifdef ESP32
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &display_updater, true);
  timerAlarmWrite(timer, 2000, true);
  timerAlarmEnable(timer);
#endif


  delay(1000);

  USE_SERIAL.println("after");
  USE_SERIAL.println();
  USE_SERIAL.println();

  //  for (uint8_t t = 4; t > 0; t--) {
  //    USE_SERIAL.printf("[SETUP] WAIT %d...\n", t);
  //    USE_SERIAL.flush();
  //    delay(1000);
  //  }


  USE_SERIAL.println("");
  USE_SERIAL.println("WiFi connected");
  USE_SERIAL.println("IP address: ");
  USE_SERIAL.println(WiFi.localIP());
  configTime(0, 0, "north-america.pool.ntp.org");
  setenv("TZ", "CST6CDT,M3.2.0,M11.1.0", 1);
  //timeClient.begin()

  display.setTextSize(1);
  display.setCursor(0, 0);
  display.setTextWrap(true);
  display.setTextColor(myGREEN);
  display.println("Connected!");
  display.println("IP: ");
  display.println(my_ip);
  //Starting stock data
  for(int i = 0; i < numTicks; i++){
     stocks = stocks + getMarket(tickers[i]) + ", ";
  }
  delay(1000);
  display.setFastUpdate(true);
  display.clearDisplay();

}

//color variables
uint8_t r = 53;
uint8_t g = 19;

uint8_t b = 121;
uint8_t cAdd = 255;
char cc = 'r';


//wifi message scroll variables
int sStep = -1;
unsigned long scrollTime = 0;

//stock request vars
int tickStep = 0;
uint8_t tickDelay = 500;
int marketScroll = -1;
unsigned long markScrollTime = 0;

void loop() {
  //update display text color
  display.setTextColor(display.color565(r, g, b));

  //start time of the loop
  unsigned long t_now = millis();

//  if (WiFi.status() != WL_CONNECTED) {
//
//  }


  //Check for time update
  if (t_now - lastTR >= inTR) {
    String currTime = getTime();
    //USE_SERIAL.println(currTime);
    displayTime(currTime, 0, 24);
    lastTR = t_now;

    //change color -- cycle
//        switch (cc){
//          case 'r':
//            if (cAdd == 0){
//              cc = 'g';
//            } else {
//              r += 11;
//              cAdd -= 15;
//            }
//            break;
//          case 'g':
//            if (cAdd == 255){
//              cc = 'b';
//            } else {
//              g += 11;
//              cAdd += 15;
//            }
//            break;
//          case 'b':
//            if (cAdd == 0){
//              cc = 'r';
//              cAdd = 255;
//            } else {
//              b += 11;
//              cAdd -= 15;
//            }
//            break;
//        }
  }

  //Check if wifi message scrolling is done -- delay and restart
  if (sStep < 0) {
    sStep = 0;
    scrollTime = t_now + 100;
  }
  sStep = stepScroll(16, 75, wifiMessage, 150, 10, 0, scrollTime, sStep);


  // check if we need to update stock data;
  if (needMarketUpdate) {
    String newStocks = "";
    for (int i = 0; i < numTicks; i++) {
      newStocks = newStocks + getMarket(tickers[i]) + "   ";
    }
    stocks = newStocks;
    needMarketUpdate = false;
  }
  else {
    if (marketScroll < 0) {
      marketScroll = 0;
      markScrollTime = t_now;
    }
    marketScroll = stepScroll(8, 150, stocks, 27, 100, 20, markScrollTime, marketScroll);
  }

  //Check for stock update time
  //  if(t_now - lastSR >= inSR){
  //
  //    if(tickStep == 0){
  //      stocks = "";
  //      marketScroll = stepScroll(8, 150, " ", 255, 255, 0, t_now, 0);
  //    }
  //
  //    if(tickStep == numTicks -1){
  //      lastSR = t_now
  //    }
  //    else if(t_now - lastSR >= inSR + (tickStep * tickDelay)){
  //      stocks = stocks + getMarket(tickers[tickStep])+ ", ";
  //      tickStep += 1;
  //    }
  //    USE_SERIAL.println(stocks);
  //
  //
  //  } // else scroll display the current update
  //  else{
  //    if(marketScroll < 0){
  //      marketScroll = 0;
  //      markScrollTime = t_now;
  //    }
  //    marketScroll = stepScroll(8, 150, stocks, 255, 255, 0, markScrollTime, marketScroll);
  //
  //  }

  //Listen for incoming messages
  server.handleClient();


  //delay(5000);
}

*/






