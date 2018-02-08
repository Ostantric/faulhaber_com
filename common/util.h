#ifndef __CLASS_UTILS_H
#define __CLASS_UTILS_H

#include <chrono>
#include <cmath>
#include <string>
#include <time.h>

namespace util {

struct Error {
  enum Enum { UNKNOWN, OK, NULL_PTR, NOT_INITIALIZED, INVALID, TIMEOUT, MEMORY, FAIL }; //FAIL added
};

typedef Error::Enum ErrorT;

// A base class that can be used to make sure your class object's are not
// copied (in eg, cases where resources like file descriptors are held in the class)
class NoCopy {
public:
  NoCopy() = default;
  ~NoCopy() = default;

private:
  NoCopy(const NoCopy&) = delete;
  NoCopy& operator=(const NoCopy&) = delete;
};

// Return the current time from the monotonic system clock in ms.  This time is
// references to an arbitrary point, so you shouldn't use it in an absolute
// sense.  Use this for eg: calculating time elapsed in a function.
//
// This will return NAN upon error.
inline double TimeMonotonicMS(void) {
  struct timespec t;

  if (clock_gettime(CLOCK_MONOTONIC, &t) < 0) {
    return NAN;
  } else {
    return (t.tv_sec*1000.0 + t.tv_nsec / 1e6);
  }
}

util::ErrorT GetFileContents(const char *filename, std::string &out_string);

} // namespace utils
#endif // __CLASS_UTILS_H
