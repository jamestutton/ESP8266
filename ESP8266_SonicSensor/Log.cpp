#include "Log.h"

Mylog gLog;

Mylog::Mylog() {
  _maxSize = 8192;
  _enableLog = 0;
  _enableSerial = 0;
  _log = (uint8_t *)malloc(_maxSize + 1);
  _size = 0;
  _tail = 0;
}

Mylog::~Mylog() {
  free(_log);
}

size_t Mylog::write(uint8_t c)
{
  if (_enableLog) {
    _log[_tail] = c;
    _tail ++;
    if (_tail >= _maxSize) {
      _tail = 0;
    }
  }
  if (_enableSerial) {
    Serial.write(c);
  }
  return 1;
}

size_t Mylog::write(const uint8_t *buffer, size_t size) {
  if (_enableLog) {
    if (_maxSize - _tail + 1 >= size) {
      memcpy(_log + _tail, buffer, size);
      _tail += size;
    }
    else {
      size_t t = _maxSize - _tail + 1;
      memcpy(_log + _tail, buffer, t);
      memcpy(_log, buffer + t, size - t);
      _tail = size - t;
    }
    if (_tail >= _maxSize) {
      _tail = 0;
    }
  }
  if (_enableSerial) {
    Serial.write(buffer, size);
  }
  return size;
}

void Mylog::enable_log(unsigned int maxLines) {
  if (maxLines) {
    _maxSize = maxLines;
    _log = (uint8_t *)realloc(_log, _maxSize + 1);
    _size = 0;
    _tail = 0;
  }
  _enableLog = true;
}

void Mylog::disable_log(void) {
  _enableLog = false;
}

void Mylog::enable_serial(void) {
  _enableSerial = true;
}

void Mylog::disable_serial(void) {
  _enableSerial = false;
}

void Mylog::export_log(String &str) {
  uint8_t t;

  if (_tail == _size) {
    t = _log[_size];
    _log[_size] = 0;
    str.concat((const char *)_log);
    _log[_size] = t;
  }
  else {
    _log[_maxSize] = 0;
    str.concat((const char *)(_log + _tail));
    t = _log[_tail];
    _log[_tail] = 0;
    str.concat((const char *)_log);
    _log[_tail] = t;
  }
}

