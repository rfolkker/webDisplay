/*
   Copyright (c) 2015, Majenko Technologies
   All rights reserved.

   Redistribution and use in source and binary forms, with or without modification,
   are permitted provided that the following conditions are met:

 * * Redistributions of source code must retain the above copyright notice, this
     list of conditions and the following disclaimer.

 * * Redistributions in binary form must reproduce the above copyright notice, this
     list of conditions and the following disclaimer in the documentation and/or
     other materials provided with the distribution.

 * * Neither the name of Majenko Technologies nor the names of its
     contributors may be used to endorse or promote products derived from
     this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
   ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
   ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "heltec.h" // alias for `#include "SSD1306Wire.h"`
#include <ArduinoJson.h>
// #include "images.h"

#ifndef STASSID
#define STASSID "***"
#define STAPSK  "***"
#endif

const char *ssid = STASSID;
const char *password = STAPSK;
#define DISPLAY_DELAY 10

ESP8266WebServer server(80);

struct TIMAGE{
  int width;
  int height;
  unsigned char imageData[4096];
}TImage;

int oldX,oldY;
String oldText="";

void setTextBuffer(int x, int y, String text)
{
  oldText = text;
  oldX = x;
  oldY = y;
}
void flushTextBuffer(){
  Heltec.display->setColor(BLACK);
  if(oldText.length() > 0)
    Heltec.display->drawString(oldX, oldY, oldText);
  Heltec.display->setColor(WHITE);
  oldX = 0;
  oldY = 0;
  oldText = "";
}
void handleRoot() {
  char temp[400];
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;

  snprintf(temp, 400,

           "<html>\
  <head>\
    <meta http-equiv='refresh' content='5'/>\
    <title>ESP8266 Demo</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
    </style>\
  </head>\
  <body>\
    <h1>Hello from ESP8266!</h1>\
    <p>Uptime: %02d:%02d:%02d</p>\
    <img src=\"/test.svg\" />\
  </body>\
</html>",

           hr, min % 60, sec % 60
          );
  server.send(200, "text/html", temp);
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }

  server.send(404, "text/plain", message);
}

int StrToHex(char* instr)
{
  Serial.print("Original value: ");
  Serial.print(instr);
  Serial.print("/");
  Serial.println(strtol(instr, 0, 16));
  return (int) strtol(instr, 0, 16);
}

void ConvertStringToData(const char* imageData, int imageSize){
  int nSize = strlen(imageData);
  int index = 0;
  int imageOffset = 0;
  char buffer[10]; // Big enough to hold an 0x## value
  for(int n=0;n<10;buffer[n++] = 0x00);
  
  // walk through the image data and convert each hex value to 
  for(int n=0; n<nSize;n++){
    if(imageData[n] == ','){
      // Debug text to clean-up hex to int parsing errors
      // Serial.print("Original value: ");
      // Serial.print(buffer);
      // Serial.print("/");
      // Serial.println(strtol(buffer, 0, 16));
      
      TImage.imageData[imageOffset++] = (int) strtol(buffer, 0, 16);      
      index = 0; // restart the buffer
      // Flush char buffer real quick
      for(int n=0;n<10;buffer[n++] = 0x00);
    }
    else{
      if(!isWhitespace(imageData[n])){
        buffer[index++] = imageData[n];
      }
    }
  }
  // Copy the last bit
  TImage.imageData[imageOffset] = (int) strtol(buffer, 0, 16);
  // That should do it
}

void handleImage(){
  int tempWidth, tempHeight;
  int imageSize = sizeof(int)*2+4096;
  int current, next;
  String message = "";
  const char* value;
  // Needs to be large enough to hold all image data as a string
  DynamicJsonDocument jsonDoc(17000);
  
  Serial.println(server.arg(0));

  deserializeJson(jsonDoc, server.arg(0));

  // verify data
  value = jsonDoc["width"];
  Serial.print("Width: ");
  Serial.println(value);
  value = jsonDoc["height"];
  Serial.print("Height:");
  Serial.println(value);
  value = jsonDoc["data"];
  Serial.print("Data: ");
  Serial.println(value);

  // Please note, the value set is important from above
  TImage.width = jsonDoc["width"];
  TImage.height = jsonDoc["height"];
  ConvertStringToData(value, TImage.width * TImage.height);    
  Heltec.display->drawXbm(0, 0, TImage.width, TImage.height, TImage.imageData);
  server.send(200, "application/json", "{\"status\":\"Successful\",\"message\":\"none\"}");
}

void handleText(){
  int xpos, ypos;
  const char* value;
  // Needs to be large enough to hold a full string buffer
  DynamicJsonDocument jsonDoc(1024);
  
  Serial.println(server.arg(0));

  deserializeJson(jsonDoc, server.arg(0));

  // verify data
  value = jsonDoc["xpos"];
  Serial.print("X Pos: ");
  Serial.println(value);
  value = jsonDoc["ypos"];
  Serial.print("Y Pos:");
  Serial.println(value);
  value = jsonDoc["showPercent"];
  Serial.print("Display: Percent");
  Serial.println(value);
  value = jsonDoc["text"];
  Serial.print("Text: ");
  Serial.println(value);

  // Please note, the value set is important from above
  xpos = jsonDoc["xpos"];
  ypos = jsonDoc["ypos"];
  String txtString = String(value);
  Heltec.display->setTextAlignment(TEXT_ALIGN_LEFT);
  flushTextBuffer();
  if(txtString.length()>0){
    Heltec.display->drawString(xpos, ypos, txtString);
    setTextBuffer(xpos, ypos, txtString);
  }
  server.send(200, "application/json", "{\"status\":\"Successful\",\"message\":\"none\"}");
}

void handleProgress(){
  int txtpos, progress;
  bool showPercent = false;
  const char* value;
  // Needs to be large enough to hold a full string buffer
  DynamicJsonDocument jsonDoc(1024);
  
  Serial.println(server.arg(0));

  deserializeJson(jsonDoc, server.arg(0));

  // verify data
  value = jsonDoc["txtpos"];
  Serial.print("Text Pos: ");
  Serial.println(value);
  value = jsonDoc["progress"];
  Serial.print("Progress:");
  Serial.println(value);
  value = jsonDoc["text"];
  Serial.print("Text: ");
  Serial.println(value);

  // Please note, the value set is important from above
  progress = jsonDoc["progress"];
  txtpos = jsonDoc["txtpos"];
  Heltec.display->setTextAlignment(TEXT_ALIGN_CENTER);
  Heltec.display->drawProgressBar(0, 10, 120, 10, progress);

  showPercent = jsonDoc["showPercent"];
  String txtString = String(value);
  if(txtString.length()<0){
    txtString = "";
  }
  else{
    txtString += String(" ");
  }
  
  flushTextBuffer();
  if(showPercent){
    Heltec.display->drawString(txtpos, 0, txtString+String(progress) + "%");
    setTextBuffer(txtpos, 0, txtString+String(progress) + "%");
  }
  else{
    Heltec.display->drawString(txtpos, 0, txtString);
    setTextBuffer(txtpos, 0, txtString);
  }

  server.send(200, "application/json", "{\"status\":\"Successful\",\"message\":\"none\"}");
}

void handleClear(){
  Heltec.display->clear();
  server.send(200, "application/json", "{\"status\":\"Successful\",\"message\":\"none\"}");

}
void setup(void) {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");
  Heltec.begin(true /*DisplayEnable Enable*/); //, true /*Serial Enable*/);
  Heltec.display->flipScreenVertically();
  Heltec.display->setFont(ArialMT_Plain_10);
 delay(2000);
 
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Heltec.display->drawString(0,10,WiFi.localIP().toString());

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp8266")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);
  server.on("/image", HTTP_POST, handleImage);
  server.on("/text", HTTP_POST, handleText);
  server.on("/progress", HTTP_POST, handleProgress);
  server.on("/clear",handleClear);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started"); 
}

long timeSinceLastModeSwitch = 0;
void loop(void) {
  server.handleClient();
  MDNS.update();
  if (millis() - timeSinceLastModeSwitch > DISPLAY_DELAY) {
    timeSinceLastModeSwitch = millis();
    Heltec.display->display();
  }
}
