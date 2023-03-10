#ifndef webController_h
#define webController_h
#include <WebServer.h>


class WebController {
  
  private:
    // Fields // 
    const byte wifiMessageBuffLen;
    char * wifiMessage;
    //const byte numTickers;
    //const char** tickers;
    char ipAddress[20];
    WebServer server;

    // Methods //
    void stock_api_request(char* ticker, char apiBuff[]);
    void handleRoot();
    void handleMessage();
    void handleNotFound();
    
  public:
    // Methods //
    WebController(char * wifiBuff, byte wifiBuffLen);
    void startServerAndClient();
    void getNptTime(char timeBuff[], byte buffLen);
    void getMarket(char marketBuff[], char *searchTickers[], byte numTickers);
    void updateServer();
 
};

#endif
