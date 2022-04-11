//
// Created by titto on 2022/4/11.
//

#include "atomic.h"
#include "t_time.h"
#include "timer_manager.h"
#include "scheduler.h"

namespace tit {

namespace co {

timer_id_t TimerManager::add_timer(int ms, Coroutine* co) {
  return it_ = timer_.insert(it_, std::make_pair(now::ms() + ms, co));
}

void TimerManager::del_timer(const timer_id_t& it) {
  if (it_ == it) ++it_;
  timer_.erase(it_);
}

int TimerManager::check_timeout(std::vector<Coroutine*>& res) {
  if (timer_.empty()) return (uint32_t)-1;

  int64_t now_ms = now::ms();
  auto it = timer_.begin();
  for (; it != timer_.end(); ++it) {
    if (it->first > now_ms) break;
    Coroutine* co = it->second;
    if (co->it != timer_.end()) co->it = timer_.end();
    if (!co->waitx) {
      if (co->state == st_init || atomic_swap(&co->state, st_init) == st_wait) {
        res.push_back(co);
      }
    } else {
      auto waitx = (co::waitx_t*) co->waitx;
      if (atomic_compare_swap(&waitx->state, st_init, st_timeout) == st_init) {
        res.push_back(co);
      }
    }
  }

  if (it != timer_.begin()) {
    if (it_ != timer_.end() && it_->first <= now_ms) it_ = it;
    timer_.erase(timer_.begin(), it);
  }

  if (timer_.empty()) return (uint32_t)-1;
  return (int) (timer_.begin()->first - now_ms);
}

}  //  namespace co

}  // namespace tit
