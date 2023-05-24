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

using namespace CFMH201::FrameBytes;

byte CardReader::readCardCommand[CARD_READ_CMD_LEN] = {
  CARD_STX, CARD_STATION_ID, CARD_READ_BUFF_LEN, CARD_CMD_READ, // 2,0,10,32
  CARD_REQ, CARD_RD_NUMBER_OF_BLOCK, CARD_STX_ADDRESS_BLOCK, //1,1,4
  CARD_KEY, CARD_KEY, CARD_KEY, CARD_KEY, CARD_KEY, CARD_KEY,
  CARD_READ_COMMAND_BCC, CARD_ETX
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
  serial.begin(CARD_READER_BAUDRATE);
  serial.setTimeout(0);
}


uint8_t CardReader::calcBCC(uint8_t* buffer, uint8_t buffSize) {
  uint8_t BCC = buffer[1];
  for (int i = 2; i < (buffSize - 2); ++i)
    BCC ^= buffer[i];
  return BCC;
}

bool CardReader::checkPacket(uint8_t* buffer, uint8_t buffSize) {
  uint8_t respBCC = buffer[1];
  for (int i = 2; i < buffSize - 2; ++i)
    respBCC ^= buffer[i];
  return buffer[buffSize - 2] == respBCC;
}

/* Read bytes from card reader via serial */
bool CardReader::readCardBytes() {
  if (!serial.available()) return false;

  int it = 0;
  while (serial.available()) {
    incomingBytes[it] = serial.read();
    ++it;
    if (it == CARD_READ_RESP_LENGTH)
      return true;
  }
  // size_t res = serial.readBytes(incomingBytes, CARD_READ_RESP_LENGTH);
  return false;
}

// This Function will be used for pin validation in future updates
/* Write card command prepare and dump via serial
void CardReader::cmdWriteCard() {
  uint8_t buffer[CARD_WRITE_CMD_LEN] = { CARD_STX, CARD_STATION_ID, CARD_WRITE_BUFF_LEN, CARD_CMD_WRITE,
                                        CARD_REQ, CARD_WRT_NUMBER_OF_BLOCK, CARD_STX_ADDRESS_BLOCK,
                                        CARD_KEY, CARD_KEY, CARD_KEY, CARD_KEY, CARD_KEY, CARD_KEY,
    CARD_PIN, CARD_PIN, CARD_PIN, CARD_PIN,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    calcBCC(buffer, CARD_WRITE_CMD_LEN), CARD_ETX };

  serial.write(buffer, sizeof(buffer));
}
*/

/* Needs to be called in your loop function */
void CardReader::process() {
  if ((++loopCnt) % priority) return;
  if (millis() - requestTime > requestFrequency) {
    requestTime = millis();
    serial.write(readCardCommand, CARD_READ_CMD_LEN);
  }

  if (!readCardBytes()) return;
  if (!checkPacket(incomingBytes, CARD_READ_RESP_LENGTH)) return;

  for (int i = 4; i < 4 + CARD_UID_LEN; ++i)
    CardUID[i - 4] = incomingBytes[i];

  if (memcmp(CardUIDold, CardUID, CARD_UID_LEN)) {
    memcpy(CardUIDold, CardUID, CARD_UID_LEN);
    return;
  }
  if (callback != nullptr)
    callback(CardUID);
  return;
}