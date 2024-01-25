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

#include "CardReader.h"

const byte CardReader::readCardCommand[CARD_READ_CMD_LEN] = {
  CARD_STX,
  CARD_STATION_ID,
  CARD_READ_BUFF_LEN,
  CARD_CMD_READ,
  CARD_REQ,
  CARD_RD_NUMBER_OF_BLOCK,
  CARD_STX_ADDRESS_BLOCK,
  CARD_KEY, CARD_KEY,
  CARD_KEY, CARD_KEY,
  CARD_KEY, CARD_KEY,
  CARD_READ_COMMAND_BCC,
  CARD_ETX,
};

const byte CardReader::cmdHead[CARD_CMD_HEAD] = { CARD_STX, CARD_STATION_ID, 0x15, 0x00 };

byte CardReader::CardUID[CARD_UID_LEN] = { 0, 0, 0, 0 };
byte CardReader::CardUIDold[CARD_UID_LEN] = { 0xFF, 0xFF, 0xFF, 0xFF };

CardReader::CardReader(
#ifndef CARD_READER_HARDWARE_SERIAL
  uint8_t rx,
  uint8_t tx,
#endif
  readCardCB callback,
  uint16_t priority,
  uint16_t requestFrequency
) :
#ifndef CARD_READER_HARDWARE_SERIAL
  serial(rx, tx),
#else
  serial(CARD_READER_HARDWARE_SERIAL),
#endif
  callback(callback),
  priority(priority),
  requestFrequency(requestFrequency) {
}


void CardReader::setup() {
  serial.begin(9600);
  serial.setTimeout(SERIAL_READ_TICK_TIMEOUT);
}


void CardReader::sendRequest() {
  if (millis() - requestTime > requestFrequency) {
    serial.write(readCardCommand[it]);
    if (++it == CARD_READ_CMD_LEN) {
      requestTime = millis();
      it = 0;
    }
  }
}


void CardReader::readResponse() {
  static uint8_t buf[RESP_LENGTH];
  static int bufIndex = 0;

  int read = serial.read();
  if (read == -1) return;

  buf[bufIndex] = read;
  ++bufIndex;

  if (bufIndex <= CARD_CMD_HEAD) {
    if (read != int(cmdHead[bufIndex - 1])) {
      bufIndex = 0;
      return;
    }
    return;
  }

  if (bufIndex == RESP_LENGTH) {
    bufIndex = 0;

    uint8_t BCC = 0;
    for (int i = 1; i < RESP_LENGTH - 2; ++i) BCC ^= buf[i];
    if (BCC != buf[RESP_LENGTH - 2]) return;

    memcpy(CardUID, buf + CARD_UID_OFFSET, CARD_UID_LEN);
    if (memcmp(CardUIDold, CardUID, CARD_UID_LEN) || millis() - parsedTime > CARD_READ_TIME_DELTA) {
      memcpy(CardUIDold, CardUID, CARD_UID_LEN);
      parsedTime = millis();
      return;
    }

    memset(CardUIDold, 0, CARD_UID_LEN);
    if (callback != nullptr) callback(CardUID);
  }
}

/* Needs to be called in your loop function */
void CardReader::process() {
  if ((++loopCnt) % priority) return;
  sendRequest();
  readResponse();
}