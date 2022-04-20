//
// Created by titto on 2022/4/20.
//

#include "atomic.h"
#include "coroutine.h"
#include "timer_manager.h"

namespace tit {

namespace co {

uint32 TimerManager::CollectTimeoutTasks(TaskManager::ReadyTaskList& tasks) {

  auto it = timer_.begin();
  for (;it != timer_.end(); ++it) {
    if (it->first > now::ms()) break;
    Coroutine* co = it->second;
    if (co->wait_ctx_) {
      auto& wait_ctx = co->wait_ctx_;
      if (atomic_compare_swap(&wait_ctx->state_, co_state::kInit, co_state::kTimeout) == co_state::kInit) {
        tasks.push_back(co);
      } else {
        if (co->state_ == co_state::kInit || atomic_swap(&co->state_, co_state::kInit) == co_state::kWait) {
          tasks.push_back(co);
        }
      }
    }
  }

  // means there is(are) timeout task(s)
  if (it != timer_.begin()) {
    if (it != timer_.end() && it->first <= now::ms()) it_ = it;
    timer_.erase(timer_.begin(), it);
  }

  // no more timeout task
  if (timer_.empty()) return -1;

  // next timeout time
  return static_cast<int>(timer_.begin()->first - now::ms());
}
}  // namespace co

}  // namespace tit
