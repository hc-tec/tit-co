//
// Created by titto on 2022/4/14.
//

#include "channel.h"

#include <cstring>

#include "atomic.h"
#include "coroutine.h"
#include "scheduler.h"

#include "log/logging.h"

namespace tit {

namespace co {

WaitCtx::Ptr ChannelImpl::CreateWaitCtxInfo(Coroutine *co, void *buf) {
  WaitCtx* wait_ctx;

  SchedulerImpl* scheduler = TLSScheduler::instance();
  bool on_stack = scheduler->on_stack(buf);

  if (on_stack) {
    wait_ctx = static_cast<struct WaitCtx*>(
        malloc( sizeof(WaitCtx) + blk_size_));
    wait_ctx->buf_ = wait_ctx + sizeof(WaitCtx);
  } else {
    wait_ctx = static_cast<struct WaitCtx*>(
        malloc(sizeof(WaitCtx)));
    wait_ctx->buf_ = buf;
  }
  wait_ctx->co_ = co;
  return CreateWaitCtx(wait_ctx);
}

void ChannelImpl::Read(void *p) {
  mutex_.Lock();
  if (rx_ != wx_) {
    memcpy(p, buf_ + rx_, blk_size_);
    rx_ += blk_size_;
    if (rx_ == buf_size_) rx_ = 0;
    mutex_.UnLock();
  } else {
    // empty
    if (!full_) {
      SchedulerImpl* scheduler = TLSScheduler::instance();
      Coroutine* co = scheduler->running();

      WaitCtx::Ptr wait_ctx = CreateWaitCtxInfo(co, p);
      wait_queue_.push(wait_ctx);
      mutex_.UnLock();


      co->wait_ctx_ = wait_ctx;

      if (co->scheduler_ != scheduler) co->scheduler_ = scheduler;

      if (ms_ != static_cast<uint32>(-1)) scheduler->AddTimer(ms_);
      scheduler->Yield();

      if (!scheduler->is_timeout()) {
        if (wait_ctx->buf_ != p) {
          memcpy(p, wait_ctx->buf_, blk_size_);
        }
      }
    } else {
      memcpy(p, buf_ + rx_, blk_size_);
      rx_ += blk_size_;
      if (rx_ == buf_size_) rx_ = 0;
      while (!wait_queue_.empty()) {
        WaitCtx::Ptr wait_ctx = wait_queue_.front();
        wait_queue_.pop();
        if (atomic_compare_swap(&wait_ctx->state_, kInit, kReady) == kInit) {
          memcpy(buf_ + wx_, wait_ctx->buf_, blk_size_);
          wx_ += blk_size_;
          if (wx_ == buf_size_) wx_ = 0;
          mutex_.UnLock();


          Coroutine* wait_co = wait_ctx->co_;
          static_cast<SchedulerImpl*>(wait_co->scheduler_)->AddReadyTask(wait_co);
          return;
        }
      }
      full_ = false;
      mutex_.UnLock();

    }
  }
}

void ChannelImpl::Write(const void* p) {
  mutex_.Lock();
  if (wx_ != rx_) {
    memcpy(buf_ + wx_, p, blk_size_);
    wx_ += blk_size_;
    if (wx_ == buf_size_) wx_ = 0;
    if (wx_ == rx_) full_ = true;
    mutex_.UnLock();
  } else {
    if (!full_) {
      while (!wait_queue_.empty())  {
        WaitCtx::Ptr wait_ctx = wait_queue_.front();
        wait_queue_.pop();
        if (atomic_compare_swap(&wait_ctx->state_, kInit, kReady) == kInit) {
          mutex_.UnLock();
          memcpy(wait_ctx->buf_, p, blk_size_);
          Coroutine* wait_co = wait_ctx->co_;
          static_cast<SchedulerImpl*>(wait_co->scheduler_)->AddReadyTask(wait_co);
          return;
        }
      }
      memcpy(buf_ + wx_, p, blk_size_);
      wx_ += blk_size_;
      if (wx_ == buf_size_) wx_ = 0;
      if (wx_ == rx_) full_ = true;
      mutex_.UnLock();
    } else {
      SchedulerImpl* scheduler = TLSScheduler::instance();
      Coroutine* co = scheduler->running();

      WaitCtx::Ptr wait_ctx = CreateWaitCtxInfo(co, const_cast<void*>(p));
      if (wait_ctx->buf_ != p) {
        memcpy(wait_ctx->buf_, p, blk_size_);
      }
      wait_queue_.push(wait_ctx);
      mutex_.UnLock();

      co->wait_ctx_ = wait_ctx;

      if (co->scheduler_ != scheduler) co->scheduler_ = scheduler;

      if (ms_ != static_cast<uint32>(-1)) scheduler->AddTimer(ms_);
      scheduler->Yield();

      if (!scheduler->is_timeout()) {}
    }
  }
}

}  // namespace co

}  // namespace tit
