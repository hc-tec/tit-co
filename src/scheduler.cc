//
// Created by titto on 2022/4/12.
//

#include "scheduler.h"

#include "atomic.h"
#include "coroutine.h"

namespace tit {

namespace co {

SchedulerManager::SchedulerManager(uint sched_num,
                                   uint stack_size)
    : index_(0),
      sched_num_(sched_num) {
  for (int i = 0; i < sched_num; ++i) {
    SchedulerImpl::Ptr scheduler = SchedulerImpl::Create(
        i, sched_num, stack_size);
    scheduler->Start();
    scheds_.push_back(std::move(scheduler));
  }
}

Scheduler::Ptr SchedulerManager::NextScheduler() {
  atomic_inc(&index_);
  return scheds_[index_ % sched_num_];
}

SchedulerImpl::SchedulerImpl(uint sched_id,
                             uint sched_num,
                             uint stack_size)
    : id_(sched_id),
      sched_num_(sched_num),
      stack_size_(stack_size),
      stop_(false) {

}

SchedulerImpl::~SchedulerImpl() {
  if (!stop_) Stop();
}

void SchedulerImpl::Start() {
  assert(!stop_);
  std::string name(&"Scheduler-" [id_]);
  thread_ = new SimpleThread([&]() {
    Loop();
  }, name);
  thread_->Detach();
}

void SchedulerImpl::Stop() {
  assert(!stop_);
  delete thread_;
  stop_ = true;
}

}  // namespace co

}  // namespace tit
