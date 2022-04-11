//
// Created by titto on 2022/4/10.
//

#include "scheduler.h"

#include <cstdlib>

#include <iostream>
#include <functional>

#include "base/simple_thread.h"
#include "base/platform_thread.h"

#include "context/context.h"

namespace tit {

namespace co {

__thread SchedulerImpl* gSched = 0;

void Scheduler::go(Func) {

}

SchedulerImpl::SchedulerImpl(int id, int sched_num, int stack_size)
    : wait_ms_((uint32_t)-1),
      id_(id),
      sched_num_(sched_num),
      stack_size_(stack_size),
      running_co_(0),
      co_pool_(),
      stop_(false) {

    stack_ = (Stack*) calloc(8, sizeof(Stack));
    main_co_ = co_pool_.pop(); // coroutine with zero id is reserved for _main_co
}

void SchedulerImpl::main_func(tb_context_from_t from) {
  ((Coroutine*)from.priv)->ctx = from.ctx;
  (*gSched->running()->cb)(); // run the coroutine function
  tb_context_jump(from.ctx, 0); // jump back to the from context
}

void SchedulerImpl::resume(Coroutine* co) {
  tb_context_from_t from;
  Stack* s = &stack_[co->sid];
  running_co_ = co;
  if (s->p == 0) {
    s->p = (char*) malloc(stack_size_);
    s->top = s->p + stack_size_;
    s->co = co;
  }

  if (co->ctx == 0) {
    // resume new coroutine
    if (s->co != co) { this->save_stack(s->co); s->co = co; }
    co->ctx = tb_context_make(s->p, stack_size_, main_func);
    std::cout << "resume new co: " << co << " id: " << co->id << std::endl;
    from = tb_context_jump(co->ctx, main_co_); // jump to main_func(from):  from.priv == _main_co

  } else {

    // resume suspended coroutine
    std::cout << "resume co: " << co << ", id: " <<  co->id << ", stack: " << co->stack.size() << std::endl;
    if (s->co != co) {
      this->save_stack(s->co);
//      CHECK(s->top == (char*)co->ctx + co->stack.size());
      memcpy(co->ctx, co->stack.data(), co->stack.size()); // restore stack data
      s->co = co;
    }
    from = tb_context_jump(co->ctx, main_co_); // jump back to where the user called yiled()
  }

  if (from.priv) {
    // yield() was called in the coroutine, update context for it
    assert(running_co_ == from.priv);
    running_co_->ctx = from.ctx;
    std::cout << "yield co: " << running_co_ << " id: " << running_co_->id << std::endl;
  } else {
    // the coroutine has terminated, recycle it
    this->recycle();
  }
}

void SchedulerImpl::loop() {
  gSched = this;
  std::vector<Scheduler::Func *> new_tasks;
  std::vector<Coroutine*> ready_tasks;

  while (!stop_) {
    base::PlatformThread::Sleep(1000);


    do {
      task_mgr_.get_all_tasks(new_tasks, ready_tasks);

      if (!new_tasks.empty()) {
        std::cout << ">> resume new tasks, num: " << new_tasks.size() << std::endl;
        for (size_t i = 0; i < new_tasks.size(); ++i) {
          this->resume(this->new_coroutine(new_tasks[i]));
        }
        new_tasks.clear();
      }

      if (!ready_tasks.empty()) {
        std::cout << ">> resume ready tasks, num: " << ready_tasks.size() << std::endl;
        for (size_t i = 0; i < ready_tasks.size(); ++i) {
          this->resume(ready_tasks[i]);
        }
        ready_tasks.clear();
      }

    } while (0);

    if (running_co_) running_co_ = 0;

  }

}

void SchedulerImpl::start() {
  std::function<void()> func = [&]{
    this->loop();
  };
  thread_ = new SimpleThread(func, "");
  thread_->Start();
  thread_->Detach();
}

SchedulerManager::SchedulerManager(int sched_num, int stack_size)
    : sched_num_(sched_num),
      index_(0) {
  for (int i = 0; i < sched_num; ++i) {
    SchedulerImpl* s = new SchedulerImpl(i, sched_num, stack_size);
    s->start();
    scheds_.push_back(s);
  }
}

Scheduler* SchedulerManager::next_scheduler() {
  index_ = index_ % sched_num_;
  return scheds_[index_++];
}

SchedulerManager::~SchedulerManager() {
  for (int i = 0; i < sched_num_; ++i) {
    delete (SchedulerImpl*) scheds_[i];
  }
}

}  // namespace co

}  //  namespace tit