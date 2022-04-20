//
// Created by titto on 2022/4/20.
//

#ifndef TIT_COROUTINE_TIMER_MANAGER_H
#define TIT_COROUTINE_TIMER_MANAGER_H

#include <map>

#include "def.h"
#include "t_time.h"
#include "task_manager.h"

namespace tit {

namespace co {

struct Coroutine;
typedef std::multimap<int64, Coroutine*>::iterator timer_id_t;

class TimerManager {

 public:

  TimerManager()
      : timer_(),
        it_(timer_.end()) {}

  ~TimerManager() = default;

  timer_id_t AddTimer(uint32 ms, Coroutine* co) {
    return it_ = timer_.insert(it_, std::make_pair(now::ms() + ms, co));
  }

  void DelTimer(const timer_id_t& it) {
    if (it_ == it) ++it_;
    timer_.erase(it);
  }

  bool is_end(const timer_id_t& it) {
    return it == timer_.end();
  }

  uint32 CollectTimeoutTasks(TaskManager::ReadyTaskList& tasks);

 private:

  std::multimap<int64, Coroutine*> timer_;
  std::multimap<int64, Coroutine*>::iterator it_;

};

}  // namespace co

}  // namespace tit


#endif  // TIT_COROUTINE_TIMER_MANAGER_H
