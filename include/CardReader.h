/*
  This is a library for the CFMH201 Card reader chip of Chafon technology.
  It is compatible with ISO1443A/Type1-4th NFC cards.

  Written by HoustonAsh, Adiya21j for SEM Industries.

  MIT License

  Copyright (c) 2023 HoustonAsh, Adiya21j

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.
*/

#ifndef CARDREADER_H
#define CARDREADER_H

#include "Arduino.h"

#ifndef CARD_READER_HARDWARE_SERIAL
#include <SoftwareSerial.h>
#endif

#define CARD_STX                 0x02
#define CARD_STATION_ID          0x00
#define CARD_READ_BUFF_LEN       0x0A
#define CARD_WRITE_BUFF_LEN      0x1A
#define CARD_CMD_READ            0x20
#define CARD_CMD_WRITE           0x21
#define CARD_KEY                 0xFF
#define CARD_REQ                 0x01
#define CARD_RD_NUMBER_OF_BLOCK  0x01
#define CARD_WRT_NUMBER_OF_BLOCK 0x01
#define CARD_STX_ADDRESS_BLOCK   0x04
#define CARD_BCC                 0x00
#define CARD_ETX                 0x03
#define CARD_PIN                 0x21
#define CARD_WRITE_CMD_LEN       0x1F
#define CARD_READ_COMMAND_BCC    0x2E

#define CARD_READ_CMD_LEN        15
#define RESP_LENGTH              26
#define CARD_UID_LEN             4
#define CARD_UID_OFFSET          4
#define CARD_CMD_HEAD            4
#define SERIAL_READ_TICK_TIMEOUT 1

#define CARD_READ_TIME_DELTA 1000

class CardReader {
  static const byte cmdHead[CARD_CMD_HEAD];
  static const byte readCardCommand[CARD_READ_CMD_LEN];

  static uint8_t CardUID[CARD_UID_LEN];
  static uint8_t CardUIDold[CARD_UID_LEN];
  uint8_t incomingBytes[RESP_LENGTH];

  uint8_t it = 0;
  int64_t loopCnt = 0;
  int64_t requestTime = 0;
  int64_t parsedTime = 0;

  typedef void (*readCardCB)(const uint8_t* CardUID);

#ifndef CARD_READER_HARDWARE_SERIAL
  SoftwareSerial serial;
#else
  HardwareSerial serial;
#endif

  readCardCB callback;
  int priority;
  int requestFrequency;

public:

#ifndef CARD_READER_HARDWARE_SERIAL
  CardReader(uint8_t rx, uint8_t tx, readCardCB callback = nullptr, uint16_t priority = 200, uint16_t requestFrequency = 300);
#else
  CardReader(readCardCB callback = nullptr, uint16_t priority = 200, uint16_t requestFrequency = 300);
#endif
  void setup();
  void process();
};

#endif