#ifndef webController_h
#define webController_h
#include <WebServer.h>


class WebController {
  
  private:
    // Fields // 
    const byte wifiMessageBuffLen;
    char * wifiMessage;
    char ipAddress[20];
    bool serverDidUpdate = false;
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
    bool updateServer();
    void getIpAddress(char ipBuff[]);
 
};

#endif
