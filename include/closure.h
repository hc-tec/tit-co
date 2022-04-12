//
// Created by titto on 2022/4/12.
//

#ifndef TIT_COROUTINE_CLOSURE_H
#define TIT_COROUTINE_CLOSURE_H

#include <functional>

namespace tit {

namespace co {

typedef std::function<void()> Closure;

}  // namespace co

}  // namespace tit

#endif  // TIT_COROUTINE_CLOSURE_H
