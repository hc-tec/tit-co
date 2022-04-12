//
// Created by titto on 2022/4/12.
//

#include "scheduler.h"

#include "atomic.h"
#include "coroutine.h"


namespace tit {

namespace co {

struct Stack {
  char* p;       // stack pointer
  char* top;     // stack top
  Coroutine* co; // coroutine owns this stack
};

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

  stacks_ = static_cast<Stack*>(calloc(kShareStackSize, stack_size));
  main_co_ = co_pool_.Pop();
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

Coroutine* SchedulerImpl::NewCoroutine(Closure* func) {
  Coroutine* co = co_pool_.Pop();
  co->func_ = func;
  return co;
}

void SchedulerImpl::Resume(Coroutine* co) {
  tb_context_from_t from;
  Stack* stack = &stacks_[co->stack_id_];
  if (stack->p == 0) {
    stack->p = static_cast<char*>(malloc(stack_size_));
    stack->top = stack->p + stack_size_;
    stack->co = co;
  }

  // coroutine context don't make before
  // represent it's a new coroutine, and a new func(haven't yield)
  if (co->ctx_ == nullptr) {
    // cause it's share stack
    // so maybe there is other coroutine's info
    // we need move old stack into old coroutine's private stack area
    // and now stack is belong to new coroutine
    if (stack->co != co) {
      this->SaveStack(stack->co);
      stack->co = co;
    }
    co->ctx_ = tb_context_make(stack->p, stack_size_, MainFunc);
    LOG(TRACE) << "resume new co: " << co << " id: " << co->id_;
    // coroutine stack info ready
    // it turns to run MainFunc after jump
    // and in MainFunc, we will execute func of coroutine
  } else {
    LOG(TRACE) << "resume co: " << co << ", id: " <<  co->id_ << ", stack: " << co->stack_.size();
    if (stack->co != co) {
      this->SaveStack(stack->co);
      // there is some stack info in yield coroutine
      // we need move into coroutine context
      memcpy(co->ctx_, co->stack_.data(), co->stack_.size());
      stack->co = co;
    }
  }
  from = tb_context_jump(co->ctx_, main_co_);
  // after finish executing func in coroutine
  // from.priv will be set to nullptr. see MainFunc
  // but if coroutine had yield, from.priv will still keeped
  // so we can judge whether it had yield or not
  if (from.priv) {
    assert(running_co_ == from.priv);
    running_co_->ctx_ =  from.ctx;
    LOG(TRACE) << "yield co: " << running_co_ << " id: " << running_co_->id_;
  } else {
    Recycle();
  }
}

void SchedulerImpl::SaveStack(Coroutine* co) {
  if (co) {
    Stack* stack = &stacks_[co->stack_id_];
    co->stack_.reset();
    char* bottom = static_cast<char*>(co->ctx_);
    co->stack_.append(co->ctx_, stack->top - bottom);
  }
}

void SchedulerImpl::Recycle() {
  stacks_[running_co_->stack_id_].co = nullptr;
  co_pool_.Push(running_co_);
}

void SchedulerImpl::MainFunc(tb_context_from_t from) {
  ((Coroutine*)from.priv)->ctx_ = from.ctx;
  auto scheduler = ThreadLocalSingleton<SchedulerImpl*>::instance();
  scheduler->running()->func_->operator()(); // run the coroutine function
  tb_context_jump(from.ctx, 0); // jump back to the from context
}

void SchedulerImpl::Loop() {
  ThreadLocalSingleton<SchedulerImpl*>::instance() = this;

  TaskManager::NewTaskList newTaskList;
  TaskManager::ReadyTaskList readyTaskList;

  while (!stop_) {
    if (stop_) break;

    do {
      task_mgr_.GetAllTasks(newTaskList, readyTaskList);
      if (!newTaskList.empty()) {
        LOG(TRACE) << ">> resume new tasks, num: " << newTaskList.size();
        for (Closure* func : newTaskList) {
          Resume(NewCoroutine(func));
        }
        newTaskList.clear();
      }
      if (!readyTaskList.empty()) {
        LOG(TRACE) << ">> resume ready tasks, num: " << readyTaskList.size();
        for (Coroutine* co : readyTaskList) {
          Resume(co);
        }
        readyTaskList.clear();
      }

    } while(0);

    running_co_ = nullptr;

  }

}

}  // namespace co

}  // namespace tit
