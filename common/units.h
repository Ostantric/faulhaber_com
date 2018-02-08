#ifndef __COMMON_UNITS_H__
#define __COMMON_UNITS_H__

#include <stdint.h>

// At Neuralink, lengths are represented as [nm] stored in int64_t.  With this we can
// represent the range:
//   -2,147,483,648 [nm] .. 2,147,483,647 [nm]
// or about:
//    -2.15 [m] to 2.15 [m]
// Whenever you have the need to represent a length or position at Neuralink, you should
// use this typedef for your variables and store the values as [nm].
//
// Why? : If all of Neuralink sticks with 1 standard unit of length, then interfaces with
//   length arguments are always known to be in [nm].  Also as we develop tools to do
//   math, visualization, etc on lengths we have a better chance of being able to share
//   and re-use them untouched.  Finally since we all know that all lengths are [nm],
//   we don't need to worry about careful conversion between different length
//   scales - we just always use [nm] stored in int64_t (aka the Length type here).
//
// NOTE: In protobuf messages, Length are encoded as bare int64_t (we can't use
//   the Length typedef there).
namespace units {

typedef int64_t Length;

constexpr Length DOUBLE_NM_TO_NM(double nm) { return nm < 0.0 ? (Length)(nm - 0.5) : (Length)(nm + 0.5); }

constexpr Length MM_TO_NM(double mm) { return DOUBLE_NM_TO_NM(mm * 1e6); }
constexpr double NM_TO_MM(Length nm) { return ((double)(nm) / 1e6); }

// At Neuralink, velocities are represented as [nm/s] stored in int64_t.  The logic
// for this matches with that of the Length in [nm] logic, so please read the comment
// there for context.
typedef int64_t Velocity;

typedef int64_t Accel;
}
#endif // __COMMON_UNITS_H__
