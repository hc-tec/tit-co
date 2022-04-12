//
// Created by titto on 2022/4/12.
//

#ifndef TIT_COROUTINE_SCHEDULER_H
#define TIT_COROUTINE_SCHEDULER_H

#include <vector>
#include <memory>

#include "co_poll.h"
#include "def.h"
#include "task_manager.h"
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

  void Sleep();

  // schedule tasks and io events of coroutines
  void Loop();

  /* Task Manager Begin */

  // when task isn't yield, it's a new task
  void AddNewTask(Closure func) {
    task_mgr_.AddNewTasks(func);
  }

  // when task is yield, and ready to resume, it's a ready task
  void AddReadyTask(Coroutine* co) {
    task_mgr_.AddReadyTasks(co);
  }

  /* Task Manager End */

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

  Thread* thread_;  // use pointer to save Thread, prevent thread destruction

  uint id_;  // scheduler id
  uint sched_num_;  // num of schedulers
  uint stack_size_;

  Coroutine* main_co_;
  Coroutine* running_co_;

  Stack* stacks_;  // share stack
  Copoll co_pool_;

  TaskManager task_mgr_;

  bool stop_;
};



}  // namespace co

}  // namespace tit



#endif  // TIT_COROUTINE_SCHEDULER_H
