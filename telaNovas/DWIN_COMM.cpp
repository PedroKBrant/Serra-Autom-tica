// DWIN_Comm.h - Library for communication between Arduino and DWIN-DGUS LCDs.
// Created by Thiago Lugli Gonçales, January 27, 2015.
// Revisited by Lucas Tamborrino, 2018
// Edited by André Cecchi, June 25th, 2018

#include "DWIN_COMM.h"

DWIN::DWIN(uint32_t baud_rate, HardwareSerial *serialPort, bool enableCRC,
           uint8_t enablePin)
    : DWINPort_(serialPort), enablePin_(enablePin), enableCRC_(enableCRC) {
  DWINPort_->begin(baud_rate);
  if (enablePin > 0) {
    pinMode(enablePin, OUTPUT);
  }
}

void DWIN::enableRead() {
  if (enablePin_ > 0) {
    digitalWrite(enablePin_, 0);
  }
}

void DWIN::enableWrite() {
  if (enablePin_ > 0) {
    digitalWrite(enablePin_, 1);
  }
}

dwin_t DWIN::page(uint16_t *val) {
  uint8_t frame[] = {0x5A, 0xA5, 0x05, 0x81, 0x03, 0x02};
  uint8_t bufferIn[10];
  uint16_t crc = calculateCRC(frame, 6);
  frame[6] = (uint8_t)(crc >> 8);
  frame[7] = (uint8_t)(crc & 0xFF);
  int i = 0;
  uint32_t oldMillis = millis();

  flushInput();

  enableWrite();
  DWINPort_->write(frame, 8);
  DWINPort_->flush();
  enableRead();
  while (i < 10) {
    while (!DWINPort_->available()) {
      if ((millis() - oldMillis) > TIMEOUT) {
#ifdef _DEBUG_IHM_
        Serial.println(F("DWIN Error - ERROR_READ_TIMEOUT"));
#endif

        return ERROR_READ_TIMEOUT;
      }
    }

    bufferIn[i++] = DWINPort_->read();
  }
  if (checkCRC(bufferIn)) {
    val[0] = (uint16_t)(bufferIn[7] | bufferIn[6] << 8);
    return DWIN_OK;

  } else {
    return ERROR_CRC;
  }
}

dwin_t DWIN::setPage(uint16_t page_id) {
  uint8_t frame[9] = {0x5A, 0xA5, 0x06, 0x80, 0x03};
  frame[5] = page_id >> 8;
  frame[6] = page_id & 0xFF;
  uint16_t crc = calculateCRC(frame, 7);
  frame[7] = (uint8_t)(crc >> 8);
  frame[8] = (uint8_t)(crc & 0xFF);

  enableWrite();
  DWINPort_->write(frame, 9);
  DWINPort_->flush();
  enableRead();
  return DWIN_OK;
}

bool DWIN::flushInput() {
  while (DWINPort_->available()) {
    DWINPort_->read();
  }
  return 1;
}

dwin_t DWIN::read(uint16_t VP, int16_t *var) {
  int16_t values;

  flushInput();

  frameRead(VP, 1);
  if (receiveValues(1, &values) == DWIN_OK) {
    var[0] = values;
    return DWIN_OK;
  } else {
#ifdef _DEBUG_IHM_
    Serial.println(F("READ - BAD CRC"));
#endif
    return ERROR_CRC;
  }
}

dwin_t DWIN::read(uint16_t VP, uint8_t *var) {
  int16_t values;

  flushInput();

  frameRead(VP, 1);
  if (receiveValues(1, &values) == DWIN_OK) {
    var[0] = (uint8_t)values;
    return DWIN_OK;
  } else {
#ifdef _DEBUG_IHM_
    Serial.println(F("READ - BAD CRC"));
#endif
    return ERROR_CRC;
  }
}

dwin_t DWIN::readString(uint16_t VP, uint16_t text_size, char *text) {
  if (text_size % 2 != 0) {
    text_size--;
  }

  flushInput();

  frameRead(VP, text_size / 2);

  return receiveString(text_size, text);
}

dwin_t DWIN::write(uint16_t VP, int16_t value) {
  flushInput();

  frameWrite(VP, 1, &value);

  return DWIN_OK;
}

dwin_t DWIN::writeLong(uint16_t VP, uint32_t value) {
  int longs[2];
  flushInput();

  longs[0] = (int)(value >> 16);
  longs[1] = (int)value;

  write(VP, longs[0]);
  write(VP + 1, longs[1]);

  return DWIN_OK;
}

dwin_t DWIN::writeString(uint16_t VP, uint16_t area_size, int16_t text_size,
                         char *text) {
  flushInput();

  return frameString(VP, area_size, text_size, text);
}

dwin_t DWIN::buzzer(uint8_t buzzTime) {
  uint8_t bufferOut[8];

  bufferOut[0] = 0x5A;
  bufferOut[1] = 0xA5;
  bufferOut[2] = 0x05;
  bufferOut[3] = 0x80;
  bufferOut[4] = 0x02;
  bufferOut[5] = buzzTime;

  uint16_t crc = calculateCRC(bufferOut, 6);
  bufferOut[6] = (uint8_t)(crc >> 8);
  bufferOut[7] = (uint8_t)(crc & 0xFF);

  flushInput();
  enableWrite();
  DWINPort_->write(bufferOut, 8);
  DWINPort_->flush();
  enableRead();

  return DWIN_OK;
}

dwin_t DWIN::PlotSingleDotTrendCurve(uint8_t channel, int16_t valor) {
  uint8_t bufferOut[9];

  bufferOut[0] = 0x5A;
  bufferOut[1] = 0xA5;
  bufferOut[2] = 0x06;
  bufferOut[3] = 0x84;
  bufferOut[4] = channel;
  bufferOut[5] = valor >> 8;
  bufferOut[6] = valor & 0xFF;

  uint16_t crc = calculateCRC(bufferOut, 7);
  bufferOut[7] = (uint8_t)(crc >> 8);
  bufferOut[8] = (uint8_t)(crc & 0xFF);

  flushInput();

  enableWrite();
  DWINPort_->write(bufferOut, 9);
  DWINPort_->flush();
  enableRead();
  return DWIN_OK;
}

dwin_t DWIN::ClearTrendCurve(uint8_t channel) {
  uint8_t bufferOut[8];

  bufferOut[0] = 0x5A;
  bufferOut[1] = 0xA5;
  bufferOut[2] = 0x05;
  bufferOut[3] = 0x80;
  bufferOut[4] = 0xEB;
  bufferOut[5] = channel + 0x55;

  uint16_t crc = calculateCRC(bufferOut, 6);
  bufferOut[6] = (uint8_t)(crc >> 8);
  bufferOut[7] = (uint8_t)(crc & 0xFF);

  enableWrite();
  DWINPort_->write(bufferOut, 8);
  DWINPort_->flush();
  enableRead();

  return DWIN_OK;
}

dwin_t DWIN::frameWrite(uint16_t VP, uint8_t LEN, int16_t *values) {
  uint8_t bufferOut[2 * LEN + 8];
  int frame_size = 5 + 2 * LEN;

  bufferOut[0] = 0x5A;
  bufferOut[1] = 0xA5;
  bufferOut[2] = frame_size & 0xFF;
  bufferOut[3] = 0x82;
  bufferOut[4] = VP >> 8;
  bufferOut[5] = VP & 0xFF;

  for (int i = 0; i < LEN; i++) {
    bufferOut[6 + 2 * i] = values[i] >> 8;
    bufferOut[7 + 2 * i] = values[i] & 0xFF;
  }

  uint16_t crc = calculateCRC(bufferOut, (frame_size + 1) & 0xFF);
  bufferOut[6 + 2 * (LEN)] = (uint8_t)(crc >> 8);
  bufferOut[7 + 2 * (LEN)] = (uint8_t)(crc & 0xFF);

  enableWrite();
  DWINPort_->write(bufferOut, frame_size + 3);
  DWINPort_->flush();
  enableRead();

  return DWIN_OK;
}

dwin_t DWIN::frameRead(uint16_t VP, uint8_t LEN) {
  uint8_t buffer[9];

  if (LEN > 50) {
    LEN = 50;
  }

  buffer[0] = 0x5A;
  buffer[1] = 0xA5;
  buffer[2] = 0x06;
  buffer[3] = 0x83;
  buffer[4] = VP >> 8;
  buffer[5] = VP & 0xFF;
  buffer[6] = LEN & 0xFF;

  uint16_t crc = calculateCRC(buffer, 7);
  buffer[7] = (uint8_t)(crc >> 8);
  buffer[8] = (uint8_t)(crc & 0xFF);

  enableWrite();
  DWINPort_->write(buffer, 9);
  DWINPort_->flush();
  enableRead();
  return DWIN_OK;
}

dwin_t DWIN::frameString(uint16_t VP, uint16_t area_size, uint16_t text_size,
                         char *text) {
  uint8_t frame[8 + area_size];
  int frame_size = 3 + area_size + 2;
  frame[0] = 0x5A;
  frame[1] = 0xA5;
  frame[2] = frame_size & 0xFF;
  frame[3] = 0x82;
  frame[4] = VP >> 8;
  frame[5] = VP & 0xFF;
  for (uint8_t i = 0; i < text_size; i++) {
    frame[6 + i] = text[i];
  }
  for (uint8_t i = text_size; i < area_size; i++) {
    frame[6 + i] = '\0';
  }

  uint16_t crc = calculateCRC(frame, (frame_size + 1) & 0xFF);
  frame[6 + area_size] = (uint8_t)(crc >> 8);
  frame[7 + area_size] = (uint8_t)(crc & 0xFF);

  enableWrite();

  DWINPort_->write(frame, frame_size + 3);
  DWINPort_->flush();
  enableRead();
  return DWIN_OK;
}

uint16_t DWIN::calculateCRC(uint8_t *frame, uint8_t LEN) {
  unsigned char index, crch, crcl;
  crch = 0xff;
  crcl = 0xff;
  uint8_t aux1;

  for (uint8_t i = 3; i < LEN; i++) {
    index = crch ^ frame[i];
    aux1 = pgm_read_byte_near(CRCTABH + index);
    crch = crcl ^ aux1;

    crcl = pgm_read_byte_near(CRCTABL + index);
  }

  return (uint16_t)((crch & 0xFF) * 0x0100 + (crcl & 0xFF));
}

bool DWIN::checkCRC(uint8_t *frame_read) {
  unsigned char index, crch, crcl;
  crch = 0xff;
  crcl = 0xff;
  uint8_t aux1;
  uint8_t LEN = frame_read[2];

  for (uint8_t i = 3; i < LEN + 1; i++) {
    index = crch ^ frame_read[i];
    aux1 = pgm_read_byte_near(CRCTABH + index);
    crch = crcl ^ aux1;

    crcl = pgm_read_byte_near(CRCTABL + index);
  }

  if (crch != frame_read[LEN + 1] || crcl != frame_read[LEN + 2]) {
    return 0;
  }
  return 1;
}

dwin_t DWIN::receiveValues(uint16_t nvalues, int16_t *values) {
  uint32_t oldMillis = millis();
  uint16_t i = 0;

  uint8_t bufferIn[16];

  while (i < (9 + 2 * nvalues)) {
    while (!DWINPort_->available()) {
      if ((millis() - oldMillis) > TIMEOUT) {
#ifdef _DEBUG_IHM_
        Serial.println(F("DWIN receiveValues - ERROR_READ_TIMEOUT"));
#endif

        return ERROR_READ_TIMEOUT;
      }
    }

    bufferIn[i++] = DWINPort_->read();
  }
  if (checkCRC(bufferIn)) {
    for (i = 0; i < nvalues; i++) {
      values[i] = bufferIn[8 + 2 * i] | bufferIn[7 + 2 * i] << 8;
    }

    return DWIN_OK;
  } else {
    return ERROR_CRC;
  }
}

dwin_t DWIN::receiveString(uint16_t text_size, char *text) {
  unsigned long oldMillis = millis();
  uint16_t i = 0;
  uint8_t frame[9 + text_size];
  while (i < (9 + text_size)) {
    while (!DWINPort_->available()) {
      if ((millis() - oldMillis) > TIMEOUT) {
#ifdef _DEBUG_IHM_
        Serial.println(F("receiveString - Receive Timeout"));
#endif
        return ERROR_READ_TIMEOUT;
      }
    }
    frame[i++] = DWINPort_->read();
  }

  if (checkCRC(frame)) {
    i = 0;
    for (i = 0; i < text_size; i++) {
      if (frame[7 + i] > 126) {
        for (i = i; i < text_size; i++) {
          text[i] = '\0';
        }
        break;
      }
      text[i] = frame[7 + i];
    }
    if (i == (text_size)) {
      text[--i] = '\0';
    } else {
      text[i] = '\0';
    }
    return DWIN_OK;
  } else {
#ifdef _DEBUG_IHM_
    Serial.println(F("receiveString - BadCRC"));
#endif
    return ERROR_CRC;
  }
}
