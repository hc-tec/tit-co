//
// Created by titto on 2022/4/21.
//

#ifndef TIT_COROUTINE_MICROS_H
#define TIT_COROUTINE_MICROS_H

#include "log/logging.h"

#define CHECK(cond) \
    if (!(cond)) LOG(FATAL) << "check failed: " #cond "! "

#endif  // TIT_COROUTINE_MICROS_H
