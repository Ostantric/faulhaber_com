#ifndef _LOGGING_
#define _LOGGING_

enum LogLevel {
  TRACE = 0,
  INFO = 1,
  ERROR = 2
};

void LogWrite(LogLevel level, const char * format, ...);

// NOTE[imo]: Temporary stub for video compatibility.  This is deprecated so you should
//   not use it!
__attribute__((deprecated)) void PrintfLog(const char* fmt, ...);

#endif
