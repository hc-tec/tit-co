//
// Created by titto on 2022/4/12.
//

#include "co_poll.h"
#include "coroutine.h"

namespace tit {

namespace co {

Copoll::Copoll()
    : id_(0),
      idle_co_ids_(kPoolSize),
      pool_(kPoolSize) {}

Coroutine* Copoll::Pop() {
  // if there is no more coroutine
  // it's time to consider capacity expansion
  if (idle_co_ids_.empty()) {
    // fixme: resize method wrong
//    pool_.resize(pool_.size() * 1.5);
    Coroutine& co = pool_[id_];
    co.id_ = id_;
    ++id_;
    co.stack_id_ = static_cast<uint8>(co.id_ & 7);
    return &co;
  } else {
    uint32 last = idle_co_ids_.back();
    idle_co_ids_.pop_back();
    Coroutine& co = pool_[last];
    co.Init();
    return &co;
  }

}

void Copoll::Push(Coroutine* co) {
  idle_co_ids_.push_back(co->id_);
}

}  // namespace co

}  // namespace tit