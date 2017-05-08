#include "Helper.h"

void print_float(char *output, float a, int decimals) {
  int i, b;
  char *p;

  p = output;
  b = a;
  p += sprintf(p, "%d.", b);
  for (i = 0; i < decimals; i ++)
  {
    a *= 10.0;
    if (i == decimals - 1)
    {
      a += 0.5;
    }
    b = a;
    *p++ = '0' + (b % 10);
  }
  *p++ = '\0';
}

String macToStr(const uint8_t* mac) {
  String result;

  for (int i = 0; i < 6; ++i) {
    result += String(mac[i], 16);

    if (i < 5) {
      result += ':';
    }
  }

  return result;
}

String macToName(const uint8_t* mac) {
  String result;

  for (int i = 0; i < 6; ++i) {
    result += String(mac[i] >> 4, 16);
    result += String(mac[i] & 0xf, 16);
  }

  return result;
}

uint32_t calculateCRC32(const uint8_t *data, size_t length) {
  uint32_t crc = 0xffffffff;
  while (length--) {
    uint8_t c = *data++;
    for (uint32_t i = 0x80; i > 0; i >>= 1) {
      bool bit = crc & 0x80000000;
      if (c & i) {
        bit = !bit;
      }
      crc <<= 1;
      if (bit) {
        crc ^= 0x04c11db7;
      }
    }
  }
  return crc;
}

void printMemory(uint8_t *data, int size) {
  for (int i = 0; i < size; i++) {
    gLog.printf("%02X", data[i]);
    if ((i + 1) % 32 == 0) {
      gLog.println();
    }
    else {
      gLog.print(" ");
    }
  }
  gLog.println();
}

