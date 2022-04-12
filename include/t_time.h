//
// Created by titto on 2022/4/11.
//

#ifndef TIT_COROUTINE_TIME_H
#define TIT_COROUTINE_TIME_H

#pragma once

#include "def.h"
#include "cerrno"

namespace tit {

namespace now {

// monotonic timestamp in milliseconds
int64_t ms();

// monotonic timestamp in microseconds
int64_t us();

} // now

namespace epoch {

// milliseconds since epoch
int64_t ms();

// microseconds since epoch
int64_t us();

} // epoch

namespace ___ {
namespace sleep {

void ms(uint32_t n);

void sec(uint32_t n);

} // sleep
} // ___

using namespace ___;

class Timer {
    public:
    Timer() {
      _start = now::us();
    }

    void restart() {
      _start = now::us();
    }

    int64_t us() const {
      return now::us() - _start;
    }

    int64_t ms() const {
      return this->us() / 1000;
    }

    private:
    int64_t _start;
};

}  // namespace tit

#endif  // TIT_COROUTINE_TIME_H
