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

/* Needs to be called in your loop function */
void CardReader::process() {
  if ((++loopCnt) % priority) return;
  if (millis() - requestTime > requestFrequency) {
    serial.write(readCardCommand[it]);
    if (++it == CARD_READ_CMD_LEN) {
      requestTime = millis();
      it = 0;
    }
  }

  if (serial.available() < 26) return;
  if (serial.readBytes(incomingBytes, RESP_LENGTH) != RESP_LENGTH) return;
  serial.flush();

  uint8_t BCC = 0;
  for (int i = 1; i < RESP_LENGTH - 2; ++i) BCC ^= incomingBytes[i];
  if (BCC != incomingBytes[RESP_LENGTH - 2]) return;

  memcpy(CardUID, incomingBytes + CARD_UID_OFFSET, CARD_UID_LEN);
  if (memcmp(CardUIDold, CardUID, CARD_UID_LEN)) {
    memcpy(CardUIDold, CardUID, CARD_UID_LEN);
    return;
  }
  memset(CardUIDold, 0, CARD_UID_LEN);
  if (callback != nullptr) callback(CardUID);
}