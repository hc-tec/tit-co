//
// Created by titto on 2022/4/14.
//

#ifndef TIT_COROUTINE_CHANNEL_H
#define TIT_COROUTINE_CHANNEL_H

#include <cstdlib>

#include <queue>
#include <memory>

#include "def.h"
#include "base/thread.h"


namespace tit {

namespace co {

struct Coroutine;

typedef base::MutexLock ThreadMutex;

typedef struct WaitCtx {

  WaitCtx() = default;

  WaitCtx(WaitCtx* ctx)
      : co_(ctx->co_),
        state_(ctx->state_),
        buf_(ctx->buf_) {
    ctx->co_ = nullptr;
    ctx->buf_ = nullptr;
  }

  WaitCtx(WaitCtx&& ctx)
      : co_(ctx.co_),
        state_(ctx.state_),
        buf_(ctx.buf_) {}

  typedef std::shared_ptr<WaitCtx> Ptr;

  Coroutine* co_;
  uint8 state_;
  void* buf_;
} WaitCtx;

static WaitCtx::Ptr CreateWaitCtx() {
  return std::make_shared<WaitCtx>();
}

static WaitCtx::Ptr CreateWaitCtx(WaitCtx* ctx) {
  return std::make_shared<WaitCtx>(ctx);
}

static WaitCtx::Ptr CreateWaitCtx(WaitCtx&& ctx) {
  return std::make_shared<WaitCtx>(std::move(ctx));
}


class ChannelImpl {
 public:
  // smart pointer type of this class
  typedef std::unique_ptr<ChannelImpl> Ptr;

  ChannelImpl(uint32 buf_size, uint32 blk_size, uint32 ms)
      : buf_size_(buf_size),
        blk_size_(blk_size),
        rx_(0),
        wx_(0),
        ms_(ms),
        full_(false) {
    buf_ = static_cast<char*>(malloc(buf_size));
  }
  // use this factory method to get a smart pointer version of this class
  // users don't need to care about what specific smart pointer is
  static Ptr Create(uint32 buf_size, uint32 blk_size, uint32 ms) {
    return std::make_unique<ChannelImpl>(buf_size, blk_size, ms);
  }

  // create wait context
  // buf is variable pointer from outside,
  // and it will keep in wait_ctx->buf
  WaitCtx::Ptr CreateWaitCtxInfo(Coroutine* co, void* buf);

  void Read(void* p);
  void Write(const void* p);

 private:
  char* buf_;
  uint32 buf_size_;
  uint32 blk_size_;
  uint32 rx_;
  uint32 wx_;
  uint32 ms_;
  bool full_;
  ThreadMutex mutex_;
  std::queue<WaitCtx::Ptr> wait_queue_;
};

template<typename T>
class Channel {
 public:
  Channel(int cap = 1, uint32 ms = -1) {
    impl = ChannelImpl::Create(cap * sizeof(T),
                               sizeof(T),
                               ms);
  }

  void operator<<(const T& p) {
    impl->Write(&p);
  }

  void operator>>(T& p) {
    impl->Read(&p);
  }

 private:
  ChannelImpl::Ptr impl;
};



}  // namespace co

}  // namespace tit


#endif  // TIT_COROUTINE_CHANNEL_H
