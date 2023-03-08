#ifndef webController_h
#define webController_h

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <WebServer.h>
#include <HTTPClient.h>
#include <WiFiClient.h>
#include <time.h>
#include "info.h"
#include "indexLED.h"

class WebController {
  
  private:
    // Fields // 
    const byte wifiMessageBuffLen;
    char wifi_message[];
    const byte numTickers;
    const char* tickers[];
    const char[] ip_address;
    WebServer server;

    // Methods //
    String stock_api_request(char[] ticker);
    void handleRoot();
    void handleMessage();
    void handleNotFound();
    
  public:
    // Methods //
    WebController(char[][] myTickers, short tickersLen, short wifiMessageBuffer);
    void startServerAndClient();
    String getNptTime();
    String getMarket();
    char* getWifiMessage();
    void updateServer();
 
};

#endif
