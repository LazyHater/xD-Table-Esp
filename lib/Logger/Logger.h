#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>
#include <stdarg.h>

class Logger
{
public:
  enum verbose_level
  {
    DEBUG = 10,
    INFO = 20,
    ERROR = 30,
    NONE = 40,
  };

  void debug(const char *fmt, ...);

  void info(const char *fmt, ...);

  void error(const char *fmt, ...);

  void printf(const char *fmt, ...);

  inline void setLevel(verbose_level level)
  {
    this->level = level;
  }

private:
  verbose_level level = INFO;
};
extern Logger logger;
#endif // LOGGER_H
