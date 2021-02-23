#include "MyEEprom.h"


int maxNrOfBytes = 512;

void begin_EEProm(int aMaxNrOfBytes) {
    maxNrOfBytes = aMaxNrOfBytes;
    EEPROM.begin(maxNrOfBytes); // Maximum number of bytes available to be used as EEProm, this is 512 for ESP32
}

// read byte from EEProm
byte read_ByteEEProm(int offset) { // offset = 0 - 511
    return byte(EEPROM.read(offset)); // return byte stored in EEProm at offset
}

// read int from EEProm
int read_intEEProm(int offset) { // offset = 0 - 511
    int result = 0;

    for (int i = sizeof(result) - 1; i >= 0; i--) {
        result = (result << 8) + byte(EEPROM.read(offset + i));
    }
    return result;
}

// write byte to EEProm
// after all bytes are written, make a call to save_EEProm()
void write_ByteEEProm(int offset, byte aByteValue) { // offset = 0 - 511
	EEPROM.write(offset, aByteValue); // store byte in EEProm at offset
}

// write int to EEProm
// after all bytes are written, make a call to save_EEProm()
void write_intEEProm(int offset, int aintValue) { // offset = 0 - (511 - 4)
    int result = aintValue;

    for (int i = 0; i < (int)sizeof(result); i++) {
    	EEPROM.write(offset + i, (byte)(result & 0xFF)); // store byte in EEProm at offset + i
        result = result >> 8;
    }
}

// save bytes written to EEProm
void save_EEProm(void) {
    EEPROM.commit();
}

String read_blockEEProm(int offset, int len)
{
  String res = "";
  for (int i = 0; i < len; ++i)
  {
    res += char(EEPROM.read(i + offset));
  }
  return res;
}

void write_blockEEProm(int offset, int len, String value)
{
  for (int i = 0; i < len; ++i)
  {
    if ((unsigned)i < value.length())
    {
      EEPROM.write(i + offset, value[i]);
    }
    else
    {
      EEPROM.write(i + offset, 0);
    }
  }
}

