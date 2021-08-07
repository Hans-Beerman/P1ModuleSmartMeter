#pragma once

#include <Arduino.h>
#include <EEPROM.h>

void begin_EEProm(int aMaxNrOfBytes);

byte read_ByteEEProm(int offset);

int read_intEEProm(int offset);

void write_ByteEEProm(int offset, byte aByteValue);

void write_intEEProm(int offset, int aintValue);

void save_EEProm(void);

String read_blockEEProm(int offset, int len);

void write_blockEEProm(int offset, int len, String value);
