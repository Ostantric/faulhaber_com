#include <iostream>
#include <sstream>
#include <stdarg.h>
#include <stdio.h>
#include <string>

#include "logging.h"

static void LogWriteVAArgs(LogLevel level, const char * format, va_list args) {
  std::stringstream ss;
  ss << level << " : " << format << std::endl;
  vfprintf(stdout, ss.str().c_str(), args);
}

void LogWrite(LogLevel level, const char * format, ...) {
  va_list args;
  va_start(args, format);
  LogWriteVAArgs(level, format, args);
  va_end(args);
}

void PrintfLog(const char* format, ...) {
  va_list args;
  va_start(args, format);
  LogWriteVAArgs(INFO, format, args);
  va_end(args);
}
