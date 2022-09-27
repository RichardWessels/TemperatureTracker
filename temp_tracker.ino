#include <Arduino.h>
#include <TM1637Display.h>
#include <Adafruit_AHTX0.h>
#include <SPI.h>
#include <SD.h>

#define CLK 2
#define DIO 3

#define LOOP_LENGTH 1000      // Loop length in ms
#define DATA_WRITE_LOOP 60    // How many update loops until data is written to SD

TM1637Display display(CLK, DIO);
Adafruit_AHTX0 aht;

uint8_t data[] = { 0xff, 0xff, 0xff, 0xff };
uint8_t blank[] = { 0x00, 0x00, 0x00, 0x00 };
const int chipSelect = 4;
int loopCount = DATA_WRITE_LOOP;

void setup() {
  Serial.begin(115200);
  Serial.println("Temperature Monitor Running...");

  if (!aht.begin()) {  // Redundant
    while (!aht.begin()) {
      Serial.println("Could not find AHT20.");
      delay(500);
    }
  }
  Serial.println("AHT20 found");

  if (!SD.begin(chipSelect)) {  // Redundant
    while (!SD.begin(chipSelect)) {
      Serial.println("SD card failed");
      delay(1000);
    }
  }
  Serial.println("SD card initialized.");
}

void loop() {

  display.setBrightness(0x01);

  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);

  int negative = temp.temperature < 0 ? 1 : 0;
  int sigDigits = negative ? -int(temp.temperature) : int(temp.temperature);
  int fracDigits = int((temp.temperature - sigDigits) * 100);
  fracDigits = fracDigits < 0 ? -fracDigits : fracDigits; // Make positive
  String sigDigitsString = String(sigDigits); // Significant digits of temperature
  String fracDigitsString = String(fracDigits); // Fractional digits (after dot)

  Serial.print("Temperature: "); Serial.print(temp.temperature); Serial.println(" degrees C");
  Serial.print("Humidity: "); Serial.print(humidity.relative_humidity); Serial.println("% rH");

  data[0] = negative ? 0b01000000 : 0;
  data[1] = sigDigits < 10 ? display.encodeDigit(0) : display.encodeDigit(sigDigitsString[0]);
  data[2] = sigDigits < 10 ? display.encodeDigit(sigDigitsString[0]) : display.encodeDigit(sigDigitsString[1]);
  data[3] = display.encodeDigit(fracDigitsString[0]);
  
  display.setSegments(data);


  if (loopCount == DATA_WRITE_LOOP) {

    String dataString = "";
    dataString += String(temp.temperature) + ", ";
    dataString += String(humidity.relative_humidity);
  
    File dataFile = SD.open("data.csv", FILE_WRITE);
  
    if (dataFile) {
      dataFile.println(dataString);
      dataFile.close();
      Serial.println("Following data string saved:");
      Serial.println(dataString);
    }
    else {
      Serial.println("Error opening file");
    }
    loopCount = 0;
  }
  
  else {
    loopCount += 1;
  }
  
  Serial.print("Loop count: ");
  Serial.println(loopCount);

  delay(LOOP_LENGTH);
}
