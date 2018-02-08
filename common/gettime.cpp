#include <chrono>
#include "gettime.h"
#include <sys/time.h>                   // for CLOCK_MONOTONIC_RAW
#include <time.h>                       // for timespec, clock_gettime


long double GetTime()  /* in [s] */
{
	timespec pt ;
	clock_gettime(CLOCK_MONOTONIC_RAW, &pt);
	long double ret = (long double)(pt.tv_sec) ;
	ret += (long double)(pt.tv_nsec) / 1e9 ;
	return ret;
}

long GetUTime()  /* in [us] */
{
	return std::chrono::duration_cast<std::chrono::microseconds>(
		std::chrono::system_clock::now().time_since_epoch()
	).count();
}
