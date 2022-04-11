//
// Created by titto on 2022/4/11.
//

#include "t_time.h"

#include <time.h>
#include <sys/time.h>

namespace tit {

namespace now {
namespace _Mono {

#ifdef CLOCK_MONOTONIC
inline int64_t ms() {
  struct timespec t;
  clock_gettime(CLOCK_MONOTONIC, &t);
  return static_cast<int64_t>(t.tv_sec) * 1000 + t.tv_nsec / 1000000;
}

inline int64_t us() {
  struct timespec t;
  clock_gettime(CLOCK_MONOTONIC, &t);
  return static_cast<int64_t>(t.tv_sec) * 1000000 + t.tv_nsec / 1000;
}

#else
inline int64_t ms() {
    return epoch::ms();
}

inline int64_t us() {
    return epoch::us();
}
#endif

} // _Mono

int64_t ms() {
  return _Mono::ms();
}

int64_t us() {
  return _Mono::us();
}

} // now

namespace epoch {

int64_t ms() {
  struct timeval t;
  gettimeofday(&t, 0);
  return static_cast<int64_t>(t.tv_sec) * 1000 + t.tv_usec / 1000;
}

int64_t us() {
  struct timeval t;
  gettimeofday(&t, 0);
  return static_cast<int64_t>(t.tv_sec) * 1000000 + t.tv_usec;
}

} // epoch

namespace ___ {
namespace sleep {

void ms(uint32_t n) {
  struct timespec ts;
  ts.tv_sec = n / 1000;
  ts.tv_nsec = n % 1000 * 1000000;
  while (nanosleep(&ts, &ts) == -1 && errno == EINTR);
}

void sec(uint32_t n) {
  struct timespec ts;
  ts.tv_sec = n;
  ts.tv_nsec = 0;
  while (nanosleep(&ts, &ts) == -1 && errno == EINTR);
}

} // sleep
} // ___

}  // namespace tit