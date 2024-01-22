#include <SoftwareSerial.h>
#include <Arduino.h>
#include "CardReader.h"

CardReader cardReader([](const uint8_t* CardUID) {
  for (int i = 0; i < 4; i++) {
    Serial.print(" ");
    Serial.print(CardUID[i], HEX);
  }
  Serial.println();
  });

void setup() {
  Serial.begin(115200);
  Serial2.begin(9000);
  cardReader.setup();
  Serial.println();
}
void loop() {
  cardReader.process();
}