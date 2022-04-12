//
// Created by titto on 2022/4/12.
//

#ifndef TIT_COROUTINE_SCHEDULER_H
#define TIT_COROUTINE_SCHEDULER_H

#include <vector>
#include <memory>

#include "def.h"
#include "base/platform_thread.h"
#include "base/singleton.h"
#include "base/simple_thread.h"
#include "base/thread_local_singleton.h"
#include "log/logging.h"

namespace tit {

namespace co {

struct Coroutine;

class Scheduler {
 public:
  // smart pointer of Scheduler
  typedef std::shared_ptr<Scheduler> Ptr;

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
  typedef std::shared_ptr<SchedulerImpl> Ptr;

  SchedulerImpl(uint sched_id, uint sched_num, uint stack_size);
  ~SchedulerImpl();

  // factory method
  // return smart pointer of Scheduler
  static Ptr Create(uint sched_id, uint sched_num, uint stack_size) {
    return std::make_shared<SchedulerImpl>(
        sched_id, sched_num, stack_size);
  }

  // start a new thread to run Loop
  // for schedule tasks and io events of coroutines
  void Start();

  // stop the scheduler, free some pointer memory
  void Stop();

  void Sleep();

  // schedule tasks and io events of coroutines
  void Loop();

 private:

  Thread* thread_;  // use pointer to save Thread, prevent thread destruction

  uint id_;  // scheduler id
  uint sched_num_;  // num of schedulers
  uint stack_size_;

  bool stop_;
};

}  // namespace co

}  // namespace tit



#endif  // TIT_COROUTINE_SCHEDULER_H
