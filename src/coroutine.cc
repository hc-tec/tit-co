//
// Created by titto on 2022/4/12.
//

#include "coroutine.h"

#include "scheduler.h"

namespace tit {

namespace co {

Coroutine::Coroutine()
    : id_(0),
      sched_id_(0),
      stack_id_(0),
      state_(co_state::kInit),
      stack_(),
      ctx_(nullptr),
      cb_(nullptr),
      scheduler_() {}

Coroutine::~Coroutine() {
  stack_.~Stream();
}

}  // namespace co

}  // namespace tit


