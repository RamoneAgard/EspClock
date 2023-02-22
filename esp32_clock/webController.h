#ifndef webController_h
#define webController_h

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WiFiClient.h>
#include <time.h>
#include "info.h"
#include "indexLED.h"

class WebController {
  
  private:
    String ip_address;
    unsigned int wifiMessageBuffLen;
    WebServer server;
    const char[][] tickers;
    const short numTickers;
    String stock_api_request(char[] ticker);
    void handleRoot();
    void handleMessage();
    void handleNotFound();
    
  public:
    WebController(char[][] myTickers, short tickersLen, unsigned int wifiMessageBuffer);
    void startServerAndClient();
    String wifi_message;
    String getNptTime();
    String getMarket();
    void updateServer();
 
};

#endif
