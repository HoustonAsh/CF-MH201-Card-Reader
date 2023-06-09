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


byte CardReader::CardUID[CARD_UID_LEN] = { 0, 0, 0, 0 };
byte CardReader::CardUIDold[CARD_UID_LEN] = { 0xFF, 0xFF, 0xFF, 0xFF };

#ifndef CARD_READER_HARDWARE_SERIAL
CardReader::CardReader(
  uint8_t rx,
  uint8_t tx,
  readCardCB callback,
  uint16_t priority,
  uint16_t requestFrequency
) :
  serial(rx, tx),
  callback(callback),
  priority(priority),
  requestFrequency(requestFrequency) {
}
#else
CardReader::CardReader(
  readCardCB callback,
  uint16_t priority,
  uint16_t requestFrequency
) :
  serial(CARD_READER_HARDWARE_SERIAL),
  callback(callback),
  priority(priority),
  requestFrequency(requestFrequency) {
}
#endif


void CardReader::setup() {
  serial.begin(9600);
}


uint8_t CardReader::calcBCC(const byte* buffer, uint8_t buffSize) {
  uint8_t BCC = buffer[1];
  for (int i = 2; i < (buffSize - 2); ++i)
    BCC ^= buffer[i];
  return BCC;
}

/* Read bytes from card reader via serial */
bool CardReader::readCardBytes() {
  if (serial.available() <= 0) return false;
  for (int i = 0; i < RESP_LENGTH; i++) {
    int a = 0;
    while (serial.available() <= 0) {
      delayMicroseconds(1);
      if (++a > SERIAL_READ_TICK_TIMEOUT)
        return false;
    }

    incomingBytes[i] = serial.read();
  }
  return true;
}

/* Needs to be called in your loop function */
void CardReader::process() {
  static int it = 0;
  static int64_t loopCnt = 0;
  if ((++loopCnt) % priority) return;
  if (millis() - requestTime > requestFrequency) {
    serial.write(readCardCommand[it]);
    ++it;
    if (it == CARD_READ_CMD_LEN) {
      requestTime = millis();
      it = 0;
    }
  }

  if (!readCardBytes()) return;
  if (calcBCC(incomingBytes, RESP_LENGTH) != incomingBytes[RESP_LENGTH - 2])
    return;

  memcpy(CardUID, incomingBytes + 4, CARD_UID_LEN);
  if (memcmp(CardUIDold, CardUID, CARD_UID_LEN)) {
    memcpy(CardUIDold, CardUID, CARD_UID_LEN);
    return;
  }
  if (callback != nullptr)
    callback(CardUID);
  return;
}