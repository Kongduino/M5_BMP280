#include <ESP32-Chimera-Core.h>
#include "Free_Fonts.h"
#include "Seeed_BMP280.h"
#include <Wire.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;
BMP280 bmp280;
double t0, t1;
float seaLevel = 1013.3;
float readAltitude(float SL, float pressure) {
  float atmospheric = pressure / 100.0F;
  return 44330.0 * (1.0 - pow(atmospheric / SL, 0.1903));
}

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" // UART service UUID
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"
// TODO
// Add notif characteristic
// Add menu

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };
    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();
      double fValue;
      if (rxValue.length() > 0) {
        Serial.println("*********");
        Serial.print("Received Value: ");
        for (int i = 0; i < rxValue.length(); i++)
          Serial.print(rxValue[i]);
        Serial.println();
        Serial.println("*********");
        fValue = ::atof(rxValue.c_str());
        if (fValue > 0.0) {
          seaLevel = fValue;
          Serial.println("Changed seaLevel to " + String(seaLevel));
          displayBMP();
        }
      }
    }
};

void buttons_test() {
  if (M5.BtnA.isPressed()) {
    Serial.printf("-0.10 HPa");
    seaLevel -= 0.1;
    displayBMP();
    delay(200);
  }
  if (M5.BtnB.isPressed()) {
    Serial.printf("B");
  }
  if (M5.BtnC.isPressed()) {
    Serial.printf("+0.10 HPa");
    seaLevel += 0.1;
    displayBMP();
    delay(200);
  }
}
void setup() {
  Serial.begin(115200);
  delay(1000);
  M5.begin();
  M5.Lcd.setTextColor(TFT_BLACK, TFT_WHITE);
  M5.Lcd.fillScreen(TFT_WHITE);
  M5.Lcd.setFreeFont(FSSB12);
  // Select the font: FreeMono18pt7b – see Free_Fonts.h
  M5.Lcd.drawString(F("BMP280 Test"), 85, 18, GFXFF);
  M5.Lcd.setFreeFont(FSS9); // FreeSans9pt7b
  if (!bmp280.init()) {
    Serial.println("Device error!");
    M5.Lcd.drawString(F("Starting BMP280 failed!"), 24, 57, GFXFF);
    M5.Lcd.drawString(F("Cannot do anything!"), 24, 82, GFXFF);
    while (1);
  }
  Serial.println(F("BMP280 init succeeded."));
  M5.Lcd.drawString(F("BMP280 init succeeded."), 24, 57, GFXFF);
  // Create the BLE Device
  BLEDevice::init("BMP Sea Level Service");
  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);
  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_TX, BLECharacteristic::PROPERTY_NOTIFY);
  pCharacteristic->addDescriptor(new BLE2902());
  BLECharacteristic *pCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_RX, BLECharacteristic::PROPERTY_WRITE);
  pCharacteristic->setCallbacks(new MyCallbacks());
  // Start the service
  pService->start();
  // Start advertising
  pServer->getAdvertising()->start();
  Serial.println("Waiting a client connection to notify...");
  displayBMP();
}

void displayBMP() {
  float pressure, temp, alt;
  char txString[23];
  String ret;
  M5.Lcd.fillRect(0, 100, 320, 137, TFT_WHITE);
  //get and print temperatures
  Serial.print("Temp: ");
  temp = bmp280.getTemperature();
  Serial.print(temp);
  Serial.println("C");

  //get and print atmospheric pressure data
  Serial.print("Pressure: ");
  pressure = bmp280.getPressure();
  Serial.print(pressure / 100.0);
  Serial.println("HPa");

  //get and print altitude data
  Serial.print("Altitude: ");
  alt = readAltitude(seaLevel, pressure);
  Serial.print(alt);
  Serial.println("m");
  Serial.println("\n");
  ret = String(temp, 2) + "C @ " + String(alt, 2) + "m";
  ret.toCharArray(txString, ret.length() + 1);
  pCharacteristic->setValue(txString);
  pCharacteristic->notify(); // Send the value to the app!
  Serial.print("*** Sent Value: ");
  Serial.print(txString);
  Serial.println(" ***");

  uint8_t linePos = 110;
  M5.Lcd.setFreeFont(FSSB9);
  M5.Lcd.drawString(F("* Temp: "), 5, linePos, GFXFF);
  linePos += 20;
  M5.Lcd.drawString(F("* Pressure: "), 5, linePos, GFXFF);
  linePos += 20;
  M5.Lcd.drawString(F("* Sea level: "), 5, linePos, GFXFF);
  linePos += 20;
  M5.Lcd.drawString(F("* Alt: "), 5, linePos, GFXFF);
  M5.Lcd.setFreeFont(FSS9);
  linePos -= 60;
  M5.Lcd.drawString(String(temp) + " C", 110, linePos, GFXFF);
  linePos += 20;
  M5.Lcd.drawString(String(pressure / 100) + " HPa", 110, linePos, GFXFF);
  linePos += 20;
  M5.Lcd.drawString(String(seaLevel) + " HPa", 110, linePos, GFXFF);
  linePos += 20;
  M5.Lcd.drawString(String(alt) + " m", 110, linePos, GFXFF);
  t0 = millis();
}

void loop() {
  t1 = millis() - t0;
  if (t1 > 9999) {
    displayBMP();
    t0 = millis();
  }
  buttons_test();
  M5.update(); // 好importantですね！
  // If not the buttons status is not updated lo.
}
