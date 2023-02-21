#ifndef webController_h
#define webController_h

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WiFiClient.h>
#include "info.h"
#include "indexLED.h"

class WebController {
  
  private:
    String ip_address;
    WebServer server;
    const char[][] tickers;
    String stock_api_request(char[] ticker);
    void handleRoot();
    void handleMessage();
    void handleNotFound();
    
  public:
    WebController(char[][] myTickers);
    void startServerAndClient();
    String wifi_message;
    String get_time();
    String get_market();
    void updateServer();
 
};

#endif
