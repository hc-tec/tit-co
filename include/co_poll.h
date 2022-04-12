//
// Created by titto on 2022/4/12.
//

#ifndef TIT_COROUTINE_CO_POLL_H
#define TIT_COROUTINE_CO_POLL_H

#include <vector>

#include "def.h"

namespace tit {

namespace co {

struct Coroutine;

class Copoll {
 public:
  Copoll();
  ~Copoll() = default;

  // get idle coroutine from pool
  Coroutine* Pop();

  // push coroutine to idle list
  void Push(Coroutine* co);

 private:
  // default coroutine num
  const uint32 kPoolSize = 256;

  uint32 id_;  // id for coroutine
  std::vector<uint32> idle_co_ids_;  // save idle coroutines id
  std::vector<Coroutine> pool_;
};

}  // namespace co

}  // namespace tit



#endif  // TIT_COROUTINE_CO_POLL_H
