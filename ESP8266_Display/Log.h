#ifndef _LOG_H_
#define _LOG_H_

#include "Arduino.h"
#include "WString.h"
#include "Print.h"

extern int a;

class Mylog: public Print {
private:
  int _maxSize;
  bool _enableLog;
  bool _enableSerial;
  uint8_t *_log;
  int _tail, _size;
public:
  Mylog();
  ~Mylog();
  size_t write(uint8_t c);
  size_t write(const uint8_t *buffer, size_t size);
  using Print::write; // pull in write(str) and write(buf, size) from Print
  void enable_log(unsigned int maxSize = 0);
  void disable_log(void);
  void enable_serial(void);
  void disable_serial(void);
  void export_log(String &str);
};

extern Mylog gLog;

#endif
