#include "webController.h"

WebController::WebController(char[][] myTickers, short tickersLen, unsigned int wifiMessageBuffer){
  tickers = myTickers;
  wifiMessageBuffLen = wifiMessageBuffer;
  wifiMessage = new char[wifiMessageBuffLen];
  
}

void WebController::startServerAndClient(){
  // start web libraries and connect to internet
  WebServer server(80);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    USE_SERIAL.print(".");
  }
  ip_address = WiFi.localIP().toString();

  // set server mappings and pram variables
  wifiMessage = "Send a message to " + ip_address;
  server.on("/", handleRoot);
  server.on("/message", handleMessage);
  server.onNotFound(handleNotFound);
  server.begin();

  //configure time settings
  configTime(0, 0, "north-america.pool.ntp.org");
  setenv("TZ", "CST6CDT,M3.2.0,M11.1.0", 1);
  
}

String WebController::getNptTime(){
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

String WebController::getMarket(){
  String stockInfo = "";
  for (short i = 0; i < numTickers; i++) {
      newStocks = newStocks + stock_api_request(tickers[i]) + "   ";
    }
  
}

String WebController::stock_api_request(char[] ticker){
  String ticker_p;
  if ((WiFi.status() == WL_CONNECTED)) {

    HTTPClient http;
    
    String target = apiStart + ticker + apiEnd;

//    USE_SERIAL.print("[HTTP] begin...\n");
    http.begin(target); //HTTP

//    USE_SERIAL.print("[HTTP] GET...\n");
    // start connection and send HTTP header
    int httpCode = http.GET();

    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      USE_SERIAL.printf("[HTTP] GET... code: %d\n", httpCode);

      // file found at server
      if (httpCode == HTTP_CODE_OK) {
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
        const char* price = Global_Quote["05. price"]; // "132.1600
        const char* change = Global_Quote["09. change"]; // "-0.3900"
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

void WebController::updateServer(){
  server.handleClient();
  
}

void WebController::handleRoot(){
  server.send(200, "text/html", INDEX_HTML);
}

void WebController::handleMessage(){
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

void WebController::handleNotFound(){
  server.send(404, "text/plain", "Request Not Found");
}
