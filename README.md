# CF-MH201 Card Reader

## Usage

### With SoftwareSerial

```cpp
#include <Arduino.h>
#include "CardReader.h"

void readenCard(const uint8_t* cardUID) {
  Serial.print("CARD IS READEN: ");
  Serial.print(cardUID[0], HEX); Serial.print(" ");
  Serial.print(cardUID[1], HEX); Serial.print(" ");
  Serial.print(cardUID[2], HEX); Serial.print(" ");
  Serial.print(cardUID[3], HEX); Serial.print("\n");
}

CardReader cardReader(52, 53, readenCard);

void setup() {
  Serial.begin(115200);
  Serial.println("READY!");
  cardReader.setup();
}

void loop() {
  cardReader.process();
}
```

Note: Manually specify package according your platform in your platformio.ini: 
`featherfly/SoftwareSerial@1.0` or `plerup/EspSoftwareSerial@^8.0.1`

### With HardwareSerial

```cpp
#define CARD_READER_HARDWARE_SERIAL Serial2

#include <Arduino.h>
#include "CardReader.h"

void readenCard(const uint8_t* cardUID) {
  Serial.print("CARD IS READEN: ");
  Serial.print(cardUID[0], HEX); Serial.print(" ");
  Serial.print(cardUID[1], HEX); Serial.print(" ");
  Serial.print(cardUID[2], HEX); Serial.print(" ");
  Serial.print(cardUID[3], HEX); Serial.print("\n");
}

CardReader cardReader(readenCard);

void setup() {
  Serial.begin(115200);
  Serial.println("READY!");
  cardReader.setup();
}

void loop() {
  cardReader.process();
}
```


// NOT WORKING
Note: You can also define your serial in platformio.ini:
```
build_flags =
    -D CARD_READER_HARDWARE_SERIAL=Serial3
```

