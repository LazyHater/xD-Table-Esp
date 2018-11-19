#include "Logger.h"

void Logger::debug(const char *fmt, ...)
{
  if (this->level > DEBUG)
    return;

  char buf[128]; // resulting string limited to 128 chars
  strcpy(buf, "DEBUG ");
  va_list args;
  va_start(args, fmt);
  vsnprintf(buf + 6, 128 - 6, fmt, args);
  va_end(args);
  Serial.print(buf);
}

void Logger::info(const char *fmt, ...)
{
  if (this->level > INFO)
    return;

  char buf[128]; // resulting string limited to 128 chars
  strcpy(buf, "INFO ");
  va_list args;
  va_start(args, fmt);
  vsnprintf(buf + 5, 128 - 5, fmt, args);
  va_end(args);
  Serial.print(buf);
}

void Logger::error(const char *fmt, ...)
{
  if (this->level > ERROR)
    return;

  char buf[128]; // resulting string limited to 128 chars
  strcpy(buf, "ERROR ");
  va_list args;
  va_start(args, fmt);
  vsnprintf(buf + 6, 128 - 6, fmt, args);
  va_end(args);
  Serial.print(buf);
}

void Logger::printf(const char *fmt, ...)
{
  char buf[128]; // resulting string limited to 128 chars
  va_list args;
  va_start(args, fmt);
  vsnprintf(buf, 128, fmt, args);
  va_end(args);
  Serial.print(buf);
}

Logger logger;