#include "webController.h"
#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <time.h>
#include "info.h"
#include "indexLED.h"

WebController::WebController(char * wifiBuff, byte wifiBuffLen)
  : server(80),
  wifiMessage(wifiBuff),
  wifiMessageBuffLen(wifiBuffLen)
{}

void WebController::startServerAndClient() {
  // start web libraries and connect to internet
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("Connected to wifi");
  ((WiFi.localIP()).toString()).toCharArray(ipAddress, 20);
  
  // set server mappings and pram variables
  strcpy(wifiMessage,"Send a Message to ");
  strcat(wifiMessage, ipAddress);

  server.on("/", std::bind(&WebController::handleRoot, this));
  server.on("/message", std::bind(&WebController::handleMessage, this));
  server.onNotFound(std::bind(&WebController::handleNotFound, this));
  server.begin();

  //configure time settings
  configTime(0, 0, "north-america.pool.ntp.org");
  setenv("TZ", "CST6CDT,M3.2.0,M11.1.0", 1);
  tzset();

}

void WebController::getNptTime(char timeBuff[], byte buffLen) {
  struct tm timeInfo;
  if(!getLocalTime(&timeInfo)){
    strcpy(timeBuff, "Failed to get time");
    return;
  }
  // timeInfo.tm_sec += 1;
  // if(timeInfo.tm_sec == 60){
  //   mktime(&timeInfo);
  // }
  strftime(timeBuff, buffLen, "%a %b %d\n%I:%M:%S%p", &timeInfo); // Mon Apr 13 12:30:45pm
}

void WebController::getMarket(char marketBuff[], char *searchTickers[], byte numTickers) {
  char apiCallBuffer[25];
  for (byte i = 0; i < numTickers; i++) {
    stock_api_request(searchTickers[i], apiCallBuffer);
    if(i == 0){
      strcpy(marketBuff, apiCallBuffer);
    }
    else {
      strcat(marketBuff, ", ");
      strcat(marketBuff, apiCallBuffer);
    }
  }
}

void WebController::stock_api_request(char* ticker, char apiBuff[]) {
  if ((WiFi.status() == WL_CONNECTED)) {
    WiFiClientSecure client;
    client.setInsecure();
    HTTPClient http;

    char target[100];
    strcpy(target, apiStart);
    strcat(target, ticker);
    strcat(target, apiEnd);

    Serial.print("url target: ");    
    Serial.println(target);
    http.begin(client, target); //HTTP
    http.setFollowRedirects(HTTPC_STRICT_FOLLOW_REDIRECTS);

    //    USE_SERIAL.print("[HTTP] GET...\n");
    // start connection and send HTTP header
    int httpCode = http.GET();
    Serial.println(httpCode);

    //Serial.println(http.getString());
    // httpCode will be negative on error
    if (httpCode == HTTP_CODE_OK) {
      // HTTP header has been send and Server response header has been handled
      // Serial.printf("[HTTP] GET... code: %d\n", httpCode);
      StaticJsonDocument<512> doc;
      DeserializationError error = deserializeJson(doc, http.getStream());
      // file found at server
      if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
        strcpy(apiBuff, "deserialization error");
        return;
      }

      JsonObject Global_Quote = doc["Global Quote"];
      strcpy(apiBuff, Global_Quote["01. symbol"]);
      strcat(apiBuff, ":");
      strncat(apiBuff, Global_Quote["05. price"], strlen(Global_Quote["05. price"])-2);
      strcat(apiBuff, " ");
      strncat(apiBuff, Global_Quote["09. change"], strlen(Global_Quote["09. change"])-2);
      // const char* symbol = Global_Quote["01. symbol"]; // "QQQM"
      // const char* price = Global_Quote["05. price"]; // "132.1600
      // const char* change = Global_Quote["09. change"]; // "-0.3900"
      // String price2 = String(price);
      // String change2 = String(change);
      // ticker_p = String(symbol) + ":" + (price2.substring(0, price2.length() - 2)) + " " + (change2.substring(0, change2.length() - 2));

    } else {
      // Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      strcpy(apiBuff, "HTTP GET error");
    }
    Serial.println("Http end");
    http.end();
  }
  else {
    // Serial.println("Wifi not connected");
    strcpy(apiBuff, "WIFI error");
  }
}

bool WebController::updateServer() {
  server.handleClient();
  if(serverDidUpdate){
    serverDidUpdate = false;
    return true;
  }
  return false;
}

void WebController::handleRoot() {
  server.send(200, "text/html", INDEX_HTML);
}

void WebController::handleMessage() {
  if (server.args() >= 1 && server.argName(0) == "m") {
    server.arg("m").toCharArray(wifiMessage, wifiMessageBuffLen);
    strcat(wifiMessage, " ->(");
    strcat(wifiMessage, ipAddress);
    strcat(wifiMessage, ")");
    serverDidUpdate = true;
    if (wifiMessage[0] == 0 || wifiMessage[1] == 0){
      strcpy(wifiMessage, "Send a message to ");
      strcat(wifiMessage, ipAddress);
    }
    server.send(200, "text/html", INDEX_HTML);
  } else {
    char response[170] = "<html> \n <body> \n <h1>NO Message Received</h1>\n <br>\n <h4>send me a message with ";
    strcat(response, ipAddress);
    strcat(response, "/message?m=ENTER_HERE</h4> \n </body> </html>");
    server.send(200, "text/html", response);
  }
}

void WebController::handleNotFound() {
  server.send(404, "text/plain", "Request Not Found");
}

void WebController::getIpAddress(char ipBuff[]){
  strcpy(ipBuff, ipAddress);
}
