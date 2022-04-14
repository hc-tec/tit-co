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
      func_(nullptr) {}

Coroutine::~Coroutine() {
  stack_.~Stream();
}

void Coroutine::Init() {
  state_ = co_state::kInit;
  ctx_ = nullptr;
  stack_.reset();
}

}  // namespace co

}  // namespace tit


