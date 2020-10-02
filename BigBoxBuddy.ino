#include <SPI.h>
#include <SD.h>
#include <FS.h>
#include <JPEGDecoder.h>
#include <TFT_eSPI.h>
#include <WiFi.h>
#include <ArduinoOTA.h>

#define PROJECT_NAME "BOX BUDDY"
#define PROJECT_VERSION "2.0"

// SCREEN SIZE: 480 x 320
#define SCREEN_W TFT_HEIGHT
#define SCREEN_H TFT_WIDTH


#include "Free_Fonts.h"

#define LOAD_SD_LIBRARY;

const char* ssid     = "Burton";
const char* password = "Takeachance01";

class Screen {
  public:
    TFT_eSPI* x;
    uint32_t HightLightColor;
    
    Screen(TFT_eSPI* tftObject) {
      x = tftObject;
      x->setTextWrap(true, true);
      x->setCursor(0, 15);
      x->setFreeFont(FSS9);
      HightLightColor = ILI9486_GREENYELLOW;
    }


    void consoleconsoleconsoleWrite(String text, bool ln = false, bool forceNoPadding = false) {
      if (x->getTextPadding() && !forceNoPadding) {
        x->setCursor(x->getCursorX() + x->getTextPadding(), x->getCursorY());
      }
      if (ln)
        x->println(text);
      else
        x->print(text);
    
    }

    void highlight(String text) {
      uint32_t tmpCol = x->textcolor;
      x->setTextColor(HightLightColor);
      consoleWrite(text, false, true);
      x->setTextColor(tmpCol);
    }

};

class POSTScreen: public Screen {

  public:
    POSTScreen(TFT_eSPI* tftObject):Screen(tftObject) {
      x->setTextPadding(5);
      x->fillScreen(ILI9486_BLACK);
      
      x->fillRect(0, 22, SCREEN_W, 50, ILI9486_DARKCYAN);
      
      x->setTextColor(ILI9486_WHITE);
      x->setFreeFont(FSS18);
      x->setTextDatum(TL_DATUM);
      consoleWrite("", true);
      consoleWrite(String(PROJECT_NAME " v" PROJECT_VERSION), true);

      x->drawFastHLine(0, 20, SCREEN_W, ILI9486_CYAN);
      x->drawFastHLine(0, 21, SCREEN_W, ILI9486_LIGHTGREY);
      x->drawFastHLine(0, 71, SCREEN_W, ILI9486_CYAN);
      x->drawFastHLine(0, 72, SCREEN_W, ILI9486_LIGHTGREY);

      x->setTextColor(ILI9486_DARKCYAN);
      x->setFreeFont(FM9);
      
      consoleWrite("", true);
      consoleWrite("Starting up the system ...", true);
      delay(1500);
    }

  
};

TFT_eSPI tft = TFT_eSPI();  // Invoke custom library

void setup() {
  
  initTFT();
  POSTScreen post = POSTScreen(&tft);
  post.consoleWrite("Opening serial debug ");
  post.highlight("115200 bauds");
  post.consoleWrite(" UART", true, true);
  
  Serial.begin(115200);

  post.consoleWrite("Initiating WiFi connectivity", true);
  initWiFi(post);

  post.consoleWrite("Looking at SPIFFS health", true);
  if (!SPIFFS.begin()) {
    post.consoleWrite("Formating filesystem ...", true);
    SPIFFS.format();
    SPIFFS.begin();
  }
  
  File root = SPIFFS.open("/");
  File file = root.openNextFile();

  while(file){
 
      Serial.print("FILE: ");
      Serial.println(file.name());
 
      file = root.openNextFile();
  }

  post.consoleWrite("Initiating SD card reader module", true);
  //initSDCardReader();
  
  post.consoleWrite("Initiating OTA", true);
  initOTA();

  post.consoleWrite("POST is done!", true);
  post.consoleWrite("", true);
  post.consoleWrite(PROJECT_NAME " is a personal devices made to extend any device display. Display alarms, reminders, photos, states etc, using the RESTApi interface or the Serial COM.");
}

void initSDCardReader() {
    if(!SD.begin(5)){
        Serial.println("Card Mount Failed");
        return;
    }
    
    uint8_t cardType = SD.cardType();

    if(cardType == CARD_NONE){
        Serial.println("No SD card attached");
        return;
    }
    
}

void initTFT() {
  tft.begin();
  tft.setRotation(1);  // landscape
}

void initWiFi(POSTScreen p) {
   
    p.consoleWrite(String("Connecting to WIFI " + String(ssid)), true);
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    p.consoleWrite(String("WiFi Connected on " + String(ssid)));
    p.consoleWrite(" (", false, true);
    p.highlight(WiFi.localIP().toString());
    p.consoleWrite(")", true, true);

}

void initOTA() {
  ArduinoOTA.setHostname("bigboxbuddy");
  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      tft.fillScreen(TFT_BLACK);
      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      tft.println("Start updating " + type);
    })
    .onEnd([]() {
      tft.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      tft.setTextDatum(CC_DATUM);
      tft.fillRect(200, 150, 200, 30, ILI9486_BLACK);
      tft.setTextColor(ILI9486_RED);
      tft.drawNumber((progress * 100) / total, 240, 160, 2);
    })
    .onError([](ota_error_t error) {
      tft.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

  ArduinoOTA.begin();
}

void loop() {
  ArduinoOTA.handle();

}
