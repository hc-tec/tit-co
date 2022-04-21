//
// Created by titto on 2022/4/12.
//

#ifndef TIT_COROUTINE_SCHEDULER_H
#define TIT_COROUTINE_SCHEDULER_H

#include <vector>
#include <memory>

#include "co_poll.h"
#include "def.h"
#include "epoll.h"
#include "io_event.h"
#include "task_manager.h"
#include "timer_manager.h"
#include "base/platform_thread.h"
#include "base/singleton.h"
#include "base/simple_thread.h"
#include "base/thread_local_singleton.h"
#include "log/logging.h"
#include "context/context.h"

namespace tit {

namespace co {

struct Coroutine;
struct Stack;

class Scheduler {
 public:
  // smart pointer of Scheduler
//  typedef std::shared_ptr<Scheduler> Ptr;
  typedef Scheduler* Ptr;

};

class SchedulerManager {
 public:
  SchedulerManager(uint sched_num, uint stack_size);
  ~SchedulerManager() = default;

  // use Round-Robin to choose Scheduler
  Scheduler::Ptr NextScheduler();

 private:
  uint index_;  // current index of scheduler
  uint sched_num_;  // num of scheduler
  std::vector<Scheduler::Ptr> scheds_;  // list of schedulers
};


class SchedulerImpl : public Scheduler {
 public:
  // smart pointer of SchedulerImpl
  // if Ptr is changed, the factory method [Create] need to be adjust also.
//  typedef std::shared_ptr<SchedulerImpl> Ptr;
  typedef SchedulerImpl* Ptr;

  SchedulerImpl(uint sched_id, uint sched_num, uint stack_size);
  ~SchedulerImpl();

  // factory method
  // return smart pointer of Scheduler
  static Ptr Create(uint sched_id, uint sched_num, uint stack_size) {
//    return std::make_shared<SchedulerImpl>(
//        sched_id, sched_num, stack_size);
      return new SchedulerImpl(sched_id, sched_num, stack_size);
  }

  uint id() const { return id_; }

  Coroutine* running() { return running_co_; }

  // start a new thread to run Loop
  // for schedule tasks and io events of coroutines
  void Start();

  // stop the scheduler, free some pointer memory
  void Stop();

  // schedule tasks and io events of coroutines
  void Loop();

  /* Task Manager Begin */

  // when task isn't yield, it's a new task
  void AddNewTask(Closure func) {
    task_mgr_.AddNewTasks(func);
    epoll_->Wakeup();
  }

  // when task is yield, and ready to resume, it's a ready task
  void AddReadyTask(Coroutine* co) {
    task_mgr_.AddReadyTasks(co);
    epoll_->Wakeup();
  }

  /* Task Manager End */

  /* Timer Manager Begin */

  // add timeout task in running coroutine
  void AddTimer(uint32 ms);

  // is running coroutine timeout
  bool is_timeout() const { return timeout_; }

  void Sleep(uint32 ms);

  /* Timer Manager End */

  /* Epoll Begin */

  // add io event to running coroutine
  bool AddIoEvent(int fd, io_event_t event);

  // delete io event to running coroutine
  void DelIoEvent(int fd, io_event_t event);

  // delete all io event to running coroutine
  void DelIoEvent(int fd);

  /* Epoll End */

  /* Stack Begin */
  // return true if p is on running coroutine' stack
  bool on_stack(void* p) const;

  /* Stack End */

  void Yield();

 private:

  static void MainFunc(tb_context_from_t from);

  void Resume(Coroutine* co);

  // save coroutine stack from share stack to private stack
  void SaveStack(Coroutine* co);

  void Recycle();

  /* Copool Begin */

  Coroutine* NewCoroutine(Closure func);

  /* Copool End */

  const uint8 kShareStackSize = 8;

  uint id_;  // scheduler id
  uint sched_num_;  // num of schedulers
  uint stack_size_;
  uint wait_ms_;  // epoll wait ms

  Thread* thread_;  // use pointer to save Thread, prevent thread destruction
  Epoll* epoll_;

  Coroutine* main_co_;
  Coroutine* running_co_;

  Stack* stacks_;  // share stack
  Copoll co_pool_;

  TaskManager task_mgr_;
  TimerManager timer_mgr;

  bool timeout_;  // is running coroutine timeout
  bool stop_;
};

inline co::SchedulerManager& schedulerManager() {
  static co::SchedulerManager schedulerManager(1, 1024*1024);
  return schedulerManager;
}

typedef ThreadLocalSingleton<SchedulerImpl*> TLSScheduler;

bool timeout();

void go(Closure func);

}  // namespace co

}  // namespace tit




#endif  // TIT_COROUTINE_SCHEDULER_H
