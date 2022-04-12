//
// Created by titto on 2022/4/11.
//

#include <deque>

#include "mutex.h"

namespace tit {

namespace co {

extern __thread SchedulerImpl* gSched;

class MutexImpl {
 public:
  MutexImpl() : _lock(false) {}
  ~MutexImpl() = default;

  void lock();

  void unlock();

  bool try_lock();

 private:
  Mutex _mtx;
  std::deque<Coroutine*> _co_wait;
  bool _lock;
};

inline bool MutexImpl::try_lock() {
  MutexGuard g(_mtx);
  return _lock ? false : (_lock = true);
}

inline void MutexImpl::lock() {
  auto s = gSched;
  _mtx.lock();
  if (!_lock) {
    _lock = true;
    _mtx.unlock();
  } else {
    Coroutine* co = s->running();
    if (co->s != s) co->s = s;
    _co_wait.push_back(co);
    _mtx.unlock();
    s->yield();
  }
}

inline void MutexImpl::unlock() {
  _mtx.lock();
  if (_co_wait.empty()) {
    _lock = false;
    _mtx.unlock();
  } else {
    Coroutine* co = _co_wait.front();
    _co_wait.pop_front();
    _mtx.unlock();
    ((SchedulerImpl*)co->s)->add_ready_task(co);
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

void Mutex::lock() const {
  ((MutexImpl*)(_p + 2))->lock();
}

void Mutex::unlock() const {
  ((MutexImpl*)(_p + 2))->unlock();
}

bool Mutex::try_lock() const {
  return ((MutexImpl*)(_p + 2))->try_lock();
}


}  // namespace co
}  // namespace tit

