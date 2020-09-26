#include <SPI.h>
#include <SD.h>
#include <FS.h>
#include <JPEGDecoder.h>
#include <TFT_eSPI.h>

#include "Free_Fonts.h"

#define LOAD_SD_LIBRARY;

TFT_eSPI tft = TFT_eSPI();  // Invoke custom library
File root;

String gloablImageList[255];
int gloablImageListCount = 0;
int imageRotationCount = 0;

void printC(int d) {
  Serial.println();
  Serial.println(d);
  Serial.print("gloablImageListCount: ");
  Serial.print(gloablImageListCount);
  Serial.println();
  Serial.println();
}

void setup() {

  Serial.begin(115200);

  initSDCardReader();
  initTFT();
  initWiFi();

  // onscreenError("ERROR", "Your SD card is missing, please insert an SD card and restart the device.");

  root = SD.open("/");

  delay(10000);

  generateImageList();

  printC(0);
  
  delay(2000);

  for (int i=0; i < gloablImageListCount; i++) {
    Serial.print(i);
    Serial.print(" : ");
    Serial.println(gloablImageList[i]);
  }

  printC(1);

}


void laodImages() {
  Serial.println("Trying to load: 20200607_190316");
  drawJpeg("/20200607_190316.jpg", 0, 0);
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

  tft.setRotation(0);  // landscape
  tft.fillScreen(TFT_BLACK);
  
}

void initWiFi() {
  
}

void onscreenError(String title, String text) {

  int r = tft.getRotation();

  int padx, pady = 0;

  if (r == 0 || r == 2) { //PORTRAIT
    padx = 5; pady = 20;
  } else {                //LANDSCAPE
    padx = 20; pady = 5;
  }
  
  tft.setTextColor(TFT_RED);
  tft.setFreeFont(FSSB24);
  tft.setCursor(padx, 40 + pady);
  tft.print(title); // Print the font name onto the TFT screen
  tft.fillRect(5 + padx, 50 + pady, tft.width() - pady, 2, TFT_DARKGREY);
  tft.setTextColor(TFT_WHITE);
  tft.setFreeFont(FSS9);
  tft.setCursor(padx + 1, pady + 70);
  tft.setTextPadding(padx + 1);
  tft.setTextWrap(true, true);
  tft.println(text);


}

void POST(String text) {
  
}

bool SDReady = false;
int readError = 0;


void loop() {
  printC(2);
  if (readError == 0) {
    if (SDReady) {
      // load next photo;

      if (imageRotationCount >= gloablImageListCount) {
        imageRotationCount = 0;
      }

      Serial.print(imageRotationCount);
      Serial.print(" / ");
      Serial.println(gloablImageListCount);
      printC(5);
      char __dataFileName[gloablImageList[imageRotationCount].length() + 1];
      gloablImageList[imageRotationCount++].toCharArray(__dataFileName, sizeof(__dataFileName));
      readError = drawJpeg(__dataFileName, 0, 0);
      printC(4);
      delay(5000);
    }
    else {
      SDReady = true;
      readError = 0;
      // recreate photo list and load first photo
      Serial.println("Generating Image List...");
      generateImageList();
      printC(3);
    }
  } else {
    SDReady = false;
    onscreenError("Error SDCard", "In order to use this BuddyBox, you need to insert an micro SD card as permanant storage device.");
    delay(3000); //Wait a bit to retry SD
  }
  
  
}

void generateImageList() {
  gloablImageListCount = 0;
  root.rewindDirectory();
  
  String h = "";
  
  while (true) {

    File entry =  root.openNextFile();
    
    if (!entry) {
      // no more files
      Serial.println("No more files");
      break;
    }

    h = entry.name();

    Serial.print("Adding ");
    Serial.print(h);
    Serial.print(" into ");
    Serial.println(gloablImageListCount);
    
    if (h.endsWith(".jpg") && !h.startsWith("/.")) {
      gloablImageList[gloablImageListCount++] = String(entry.name());
    }
  
//    if (entry.isDirectory()) {
//      Serial.println("/");
//      printDirectory(entry, numTabs + 1);
//    } else {
//      // files have sizes, directories do not
//      Serial.print("\t\t");
//      Serial.println(entry.size(), DEC);
//    }

    entry.close();
  }
    
}
