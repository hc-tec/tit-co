//
// Created by titto on 2022/4/11.
//

#include <deque>

#include "mutex.h"

#include "scheduler.h"
#include "coroutine.h"

namespace tit {

namespace co {

void Mutex::Lock() {
  auto scheduler = TLSScheduler::instance();
  mutex_.Lock();
  if (lock_) {
    Coroutine* co = scheduler->running();
    if (co->scheduler_ != scheduler) co->scheduler_ = scheduler;
    wait_queue_.push(co);
    mutex_.UnLock();
    scheduler->Yield();
  } else {
    lock_ = true;
    mutex_.UnLock();
  }
}

void Mutex::UnLock() {
  mutex_.Lock();
  if (!wait_queue_.empty()) {
    Coroutine* co = wait_queue_.front();
    wait_queue_.pop();
    mutex_.UnLock();
    (static_cast<SchedulerImpl*>(co->scheduler_))->AddReadyTask(co);
  } else {
    lock_ = false;
    mutex_.UnLock();
  }
}

bool Mutex::TryLock() {
  ThreadMutexLockGuardType guard(mutex_);
  return lock_ ? false : (lock_ = true);
}

}  // namespace co
}  // namespace tit

