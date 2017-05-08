#ifndef _HELP_H_

#define _HELP_H_

#include <WString.h>
#include "Log.h"

#define COUNT_OF(x) (sizeof(x) / sizeof((x)[0]))

#define SERIAL_PRINTF(fmt, value) { \
    sprintf(output, (fmt), (value)); \
    gLog.print(output); \
  }

#define GET_VALUE(parameter, fmt, min, max) { \
    int temp = valuestr.toInt();  \
    if ((temp >= min) && (temp <= max)) {  \
      (parameter) = temp; \
      SERIAL_PRINTF((fmt), (parameter));  \
    } \
  }

void print_float(char *output, float a, int decimals);

//generate unique name from MAC addr
String macToStr(const uint8_t* mac);
String macToName(const uint8_t* mac);

uint32_t calculateCRC32(const uint8_t *data, size_t length);

void printMemory(uint8_t *data, int size);

#endif

