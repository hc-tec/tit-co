//
// Created by titto on 2022/4/12.
//

#include "scheduler.h"

#include "atomic.h"
#include "coroutine.h"
#include "sock_ctx.h"
#include "base/platform_thread.h"


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
    LOG(INFO) << "scheduler: " << i << " init";
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
      wait_ms_(-1),
      thread_(nullptr),
      epoll_(new Epoll(sched_id)),
      running_co_(nullptr),
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
    this->Loop();
  });
  thread_->Start();
  thread_->Detach();
}

void SchedulerImpl::Stop() {
  assert(!stop_);
  delete thread_;
  stop_ = true;
}

Coroutine* SchedulerImpl::NewCoroutine(Closure func) {
  Coroutine* co = co_pool_.Pop();
  co->func_ = func;
  return co;
}

void SchedulerImpl::Resume(Coroutine* co) {
  tb_context_from_t from;
  Stack* stack = &stacks_[co->stack_id_];
  running_co_ = co;
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
  auto scheduler = SchedulerTLS::instance();
  LOG(TRACE) << "func execute in scheduler" << scheduler;
  (scheduler->running()->func_)(); // run the coroutine function
  tb_context_jump(from.ctx, 0); // jump back to the from context
}

void SchedulerImpl::Loop() {
  LOG(INFO) << "scheduler: " << id_ << " enter loop";

  SchedulerTLS::instance() = this;

  TaskManager::NewTaskList newTaskList;
  TaskManager::ReadyTaskList readyTaskList;

  while (!stop_) {
    int events = epoll_->Wait(wait_ms_);
    if (stop_) break;

    if (unlikely(events == -1)) {
      if (errno != EINTR) {
        LOG(ERROR) << "epoll wait error";
      }
      continue;
    }

    for (int i = 0; i < events; ++i) {
      epoll_event& ev = (*epoll_)[i];
      // if fd is wake up fd, mostly not
      if (epoll_->is_wakeup_fd(ev)) {
        epoll_->HandleWakeUpEvent();
        continue;
      }
      int fd = epoll_->user_data(ev);
      auto& ctx = get_sock_ctx(fd);
      uint32 read_co = 0, write_co = 0;
      if ((ev.events | EPOLLIN) && !(ev.events | EPOLLOUT)) {
        read_co = ctx.get_read_co_id(id_);
      }
      if (!(ev.events | EPOLLIN) && (ev.events | EPOLLOUT)) {
        write_co = ctx.get_write_co_id(id_);
      }
      if (read_co) Resume(co_pool_[read_co]);
      if (write_co) Resume(co_pool_[write_co]);
    }

    do {
      task_mgr_.GetAllTasks(newTaskList, readyTaskList);
      if (!newTaskList.empty()) {
        LOG(TRACE) << ">> resume new tasks, num: " << newTaskList.size();
        for (Closure func : newTaskList) {
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

void SchedulerImpl::AddIoEvent(int fd, io_event_t event) {
  if (event == kEvRead) {
    epoll_->AddEvRead(fd, running_co_->id_);
  }
  epoll_->AddEvWrite(fd, running_co_->id_);
}

void SchedulerImpl::DelIoEvent(int fd, io_event_t event) {
  if (event == kEvRead) {
    epoll_->DelEvRead(fd, running_co_->id_);
  }
  epoll_->DelEvWrite(fd, running_co_->id_);
}

void SchedulerImpl::DelIoEvent(int fd) {
  epoll_->DelEvent(fd);
}

bool SchedulerImpl::on_stack(void* p) const {
  Stack* stack = &stacks_[running_co_->stack_id_];
  return (stack->p <= p) && (p < stack->top);
}

}  // namespace co

}  // namespace tit
