//
// Created by titto on 2022/4/13.
//

#ifndef TIT_COROUTINE_EPOLL_H
#define TIT_COROUTINE_EPOLL_H

#include <sys/epoll.h>

#include "def.h"

namespace tit {

namespace co {

class Epoll {
 public:

  explicit Epoll(uint32 sched_id);
  ~Epoll();

  /* IO event handle begin */

  bool AddEvRead(uint32 sched_id, uint32 co);
  bool AddEvWrite(uint32 sched_id, uint32 co);
  void DelEvRead(uint32 sched_id, uint32 co);
  void DelEvWrite(uint32 sched_id, uint32 co);
  void DelEvent(uint32 sched_id);

  /* IO event handle end */

  /* wake up handle begin */

  bool is_wakeup_fd(const epoll_event& ev);
  void Signal(char x = 'c');
  void HandleWakeUpEvent();

  /* wake up handle end */

  epoll_event& operator[](int i);
  int user_data(const epoll_event& ev);

  /* epoll handle begin */

  int Wait(int ms);
  void Close();

  /* epoll handle end */

 private:

  static constexpr int kEventNum = 1024;

  int ep_;
  uint32 sched_id_;
  int wake_up_fd_;
  epoll_event* ev_;

  bool signal_;

};

}  // namespace co

}  // namespace tit



#endif  // TIT_COROUTINE_EPOLL_H
