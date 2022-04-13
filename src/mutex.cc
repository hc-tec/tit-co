//
// Created by titto on 2022/4/11.
//

#include <deque>

#include "mutex.h"

#include "scheduler.h"
#include "coroutine.h"

namespace tit {

namespace co {

//extern __thread SchedulerImpl* gSched;

class MutexImpl {
 public:
  MutexImpl() : _lock(false) {}
  ~MutexImpl() = default;

  void Lock();

  void Unlock();

  bool TryLock();

 private:
  Mutex _mtx;
  std::deque<Coroutine*> _co_wait;
  bool _lock;
};

inline bool MutexImpl::TryLock() {
  MutexGuard g(_mtx);
  return _lock ? false : (_lock = true);
}

inline void MutexImpl::Lock() {
  auto s = SchedulerTLS::instance();
  _mtx.Lock();
  if (!_lock) {
    _lock = true;
    _mtx.Unlock();
  } else {
    Coroutine* co = s->running();
    if (co->scheduler_ != s) co->scheduler_ = s;
    _co_wait.push_back(co);
    _mtx.Unlock();
    s->Yield();
  }
}

inline void MutexImpl::Unlock() {
  _mtx.Lock();
  if (_co_wait.empty()) {
    _lock = false;
    _mtx.Unlock();
  } else {
    Coroutine* co = _co_wait.front();
    _co_wait.pop_front();
    _mtx.Unlock();
    ((SchedulerImpl*)co->scheduler_)->AddReadyTask(co);
  }
}

// memory: |4(refn)|4|MutexImpl|
Mutex::Mutex() {
  _p = (uint32_t *) malloc(sizeof(MutexImpl) + 8);
  _p[0] = 1; // refn
  new (_p + 2) MutexImpl;
}

Mutex::~Mutex() {
  if (_p && atomic_dec(_p) == 0) {
    ((MutexImpl*)(_p + 2))->~MutexImpl();
    ::free(_p);
  }
}

void Mutex::Lock() const {
  ((MutexImpl*)(_p + 2))->Lock();
}

void Mutex::Unlock() const {
  ((MutexImpl*)(_p + 2))->Unlock();
}

bool Mutex::TryLock() const {
  return ((MutexImpl*)(_p + 2))->TryLock();
}


}  // namespace co
}  // namespace tit

