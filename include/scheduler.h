//
// Created by titto on 2022/4/10.
//

#ifndef TIT_COROUTINE_SCHEDULER_H
#define TIT_COROUTINE_SCHEDULER_H

#include <cstring>

#include <functional>
#include <vector>

#include "base/mutex.h"
#include "base/thread.h"

//#include "epoll.h"
#include "table.h"
#include "stream.h"
#include "context/context.h"

namespace tit {

namespace co {

using Mutex = base::MutexLock;
using MutexGuard = base::MutexLockGuard;

struct Coroutine;

class Scheduler {
 public:
  typedef std::function<void()> Func;

  void go(Func);

 protected:
  Scheduler() = default;
  ~Scheduler() = default;
};

class TaskManager {
 public:
  TaskManager() = default;
  ~TaskManager() = default;

  void add_new_task(Scheduler::Func* cb) {
    MutexGuard guard(mutex_);
    new_tasks_.push_back(cb);
  }

  void add_ready_task(Coroutine* co) {
    MutexGuard guard(mutex_);
    ready_tasks_.push_back(co);
  }

  void get_all_tasks(
      std::vector<Scheduler::Func*>& new_tasks,
      std::vector<Coroutine*>& ready_tasks
  ) {
    MutexGuard guard(mutex_);
    if (!new_tasks_.empty()) new_tasks_.swap(new_tasks);
    if (!ready_tasks_.empty()) ready_tasks_.swap(ready_tasks);
  }

 private:
    Mutex mutex_;
    std::vector<Scheduler::Func*> new_tasks_;
    std::vector<Coroutine*> ready_tasks_;
};

struct Stack {
  char* p;       // stack pointer
  char* top;     // stack top
  Coroutine* co; // coroutine owns this stack
};

enum co_state_t : uint8_t {
  st_init = 0,     // initial state
  st_wait = 1,     // wait for an event
  st_ready = 2,    // ready to resume
  st_timeout = 4,  // timeout
};

struct Coroutine {
  Coroutine() = default;
  ~Coroutine() {};

  uint32_t id;
  uint8_t state;
  uint8_t sid;

  tb_context_t ctx;

  // for saving stack data of this coroutine
  union { Stream stack; char _dummy1[sizeof(Stream)]; };

  // Once the coroutine starts, we no longer need the cb, and it can
  // be used to store the Scheduler pointer.
  union {
    Scheduler::Func* cb;   // coroutine function
    Scheduler* s;  // scheduler this coroutine runs in
  };

};

class Copool {
 public:
  // _tb(14, 14) can hold 2^28=256M coroutines.
  Copool() : table_(14, 14), id_(0) {
    ids_.reserve(1u << 14);
  }

  ~Copool() {
    for (int i = 0; i < id_; ++i) table_[i].~Coroutine();
  }

  Coroutine* pop() {
    if (!ids_.empty()) {
      int last = ids_.back();
      ids_.pop_back();
      auto& co = table_[last];
      co.state = st_init;
      co.ctx = 0;
      co.stack.bzero();
      return &co;
    } else {
      auto& co = table_[id_];
      co.id = id_++;
      co.sid = (uint8_t)(co.id & 7);
      return &co;
    }
  }

  void push(Coroutine* co) {
    ids_.push_back(co->id);
    if (ids_.size() >= 1024) co->stack.bzero();
  }

  Coroutine* operator[](size_t i) {
    return &table_[i];
  }

 private:
  co::table<Coroutine> table_;
  std::vector<int> ids_;
  int id_;
};


class SchedulerImpl : public Scheduler {
 public:
  SchedulerImpl(int id, int sched_num, int stack_size);
  ~SchedulerImpl() {
      delete thread_;
  };

  Coroutine* running() { return running_co_; }

  void resume(Coroutine* co);

  void yield();

  // add a new task will run in a coroutine later (thread-safe)
  void add_new_task(Scheduler::Func* cb) {
    task_mgr_.add_new_task(cb);
//    _epoll->signal();
  }

  // add a coroutine ready to resume (thread-safe)
  void add_ready_task(Coroutine* co) {
    task_mgr_.add_ready_task(co);
//    _epoll->signal();
  }

  void start();

 private:

  static void main_func(tb_context_from_t from);

  void recycle() {
    stack_[running_co_->sid].co = 0;
    co_pool_.push(running_co_);
  }

  // save stack for the coroutine
  void save_stack(Coroutine* co) {
    if (co) {
      co->stack.reset();
      co->stack.append(co->ctx, stack_[co->sid].top - (char*)co->ctx);
    }
  }

  // pop a Coroutine from the pool
  Coroutine* new_coroutine(Scheduler::Func* cb) {
    Coroutine* co = co_pool_.pop();
    co->cb = cb;
    return co;
  }

  void loop();

  int wait_ms_;
  int id_;
  int sched_num_;
  int stack_size_;
  Stack* stack_;

  base::Thread* thread_;

  Coroutine* running_co_;
  Coroutine* main_co_;

  Copool co_pool_;
  TaskManager task_mgr_;

  bool stop_;
};

class SchedulerManager  {
 public:
  SchedulerManager(int sched_num, int stack_size);
  ~SchedulerManager();

  Scheduler* next_scheduler();

 private:
  int sched_num_;
  std::vector<Scheduler*> scheds_;
  int index_;
};



}  // namespace co

}  //  namespace tit



#endif //TIT_COROUTINE_SCHEDULER_H
