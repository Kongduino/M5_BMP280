#include <M5Stack.h>
#include "Free_Fonts.h"
#include "Seeed_BMP280.h"
#include <Wire.h>

BMP280 bmp280;

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
    M5.Lcd.drawJpgFile(SD, "/XMark20.jpg", 2, 55);
    while (1);
  }
  Serial.println(F("BMP280 init succeeded."));
  M5.Lcd.drawString(F("BMP280 init succeeded."), 24, 57, GFXFF);
  M5.Lcd.drawJpgFile(SD, "/Check20.jpg", 2, 55);
}

float seaLevel = 1017.8;
float readAltitude(float SL, float pressure) {
  float atmospheric = pressure / 100.0F;
  return 44330.0 * (1.0 - pow(atmospheric / SL, 0.1903));
}

void loop() {
  float pressure, temp, alt;
  M5.Lcd.fillRect(0, 100, 320, 137, TFT_WHITE);
  //get and print temperatures
  Serial.print("Temp: ");
  temp = bmp280.getTemperature();
  Serial.print(temp);
  Serial.println("C");

  //get and print atmospheric pressure data
  Serial.print("Pressure: ");
  Serial.print(pressure = bmp280.getPressure());
  Serial.println("Pa");

  //get and print altitude data
  Serial.print("Altitude: ");
  alt = readAltitude(seaLevel, pressure);
  Serial.print(alt);
  Serial.println("m");
  Serial.println("\n");

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
  M5.Lcd.drawString(String(temp)+" C", 110, linePos, GFXFF);
  linePos += 20;
  M5.Lcd.drawString(String(pressure / 100)+" HPa", 110, linePos, GFXFF);
  linePos += 20;
  M5.Lcd.drawString(String(seaLevel)+" HPa", 110, linePos, GFXFF);
  linePos += 20;
  M5.Lcd.drawString(String(alt)+" m", 110, linePos, GFXFF);
  delay(5000);
}
