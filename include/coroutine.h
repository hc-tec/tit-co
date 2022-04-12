//
// Created by titto on 2022/4/12.
//

#ifndef TIT_COROUTINE_COROUTINE_H
#define TIT_COROUTINE_COROUTINE_H

#include "closure.h"
#include "def.h"
#include "memory"
#include "stream.h"
#include "context/context.h"

namespace tit {

namespace co {

class Scheduler;

enum co_state : uint8 {
  kInit = 0,     // initial state
  kWait = 1,     // wait for an event
  kReady = 2,    // ready to resume
  kTimeout = 4,  // timeout
};

struct Coroutine {

  Coroutine();
  ~Coroutine();

  // init state, ctx, stack
  void Init();

  uint32 id_;  // coroutine id
  uint32 sched_id_;  // scheduler id
  uint8 stack_id_;  // stack index in scheduler share stack
  uint8 state_;  // coroutine state
  Stream stack_;  // private stack of coroutine

  tb_context_t ctx_;  // context, a pointer points to the stack bottom

  // a coroutine hold a function
  // when coroutine if first created, we should call the function
  // let function executes, once the function yield
  // next time coroutine resume, we needn't call it twice
  // just let scheduler resume the coroutine to last position the function runs
  // so we use union to save
  union {
    Closure func_;       // function the coroutine holding
    Scheduler* scheduler_;  // scheduler belong to
  };

};

}  // namespace co

}  // namespace tit



#endif  // TIT_COROUTINE_COROUTINE_H
