//
// Created by titto on 2022/4/11.
//

#ifndef TIT_COROUTINE_TIMER_MANAGER_H
#define TIT_COROUTINE_TIMER_MANAGER_H

#include <cstdint>
#include <map>
#include <vector>

namespace tit {

namespace co {

struct Coroutine;
typedef std::multimap<int, Coroutine*>::iterator timer_id_t;

// header of wait info
struct waitx_t {
  Coroutine* co;
  union { uint8_t state; void* dummy; };
};

class TimerManager {
 public:
  TimerManager()
      : timer_(),
        it_(timer_.end()) {}
  ~TimerManager() {}

  timer_id_t add_timer(int ms, Coroutine* co);

  void del_timer(const timer_id_t& it);

  timer_id_t end() {
    return timer_.end();
  }

  int check_timeout(std::vector<Coroutine*>& res);

 private:
  std::multimap<int, Coroutine*> timer_;
  std::multimap<int, Coroutine*>::iterator it_;
};


}  //  namespace co

}  // namespace tit



#endif  // TIT_COROUTINE_TIMER_MANAGER_H
