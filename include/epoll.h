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

  explicit Epoll(uint32 sched_id_);
  ~Epoll();

  /* IO event handle begin */

  bool AddEvRead(int fd, uint32 co_id) const;
  bool AddEvWrite(int fd, uint32 co_id) const;
  void DelEvRead(int fd, uint32 co_id) const;
  void DelEvWrite(int fd, uint32 co_id) const;
  void DelEvent(int fd) const;

  /* IO event handle end */

  /* wake up handle begin */

  bool is_wakeup_fd(const epoll_event& ev) const;
  void Wakeup();
  void HandleWakeUpEvent();

  /* wake up handle end */

  epoll_event& operator[](int i) { return ev_[i]; };
  int user_data(const epoll_event& ev) { return ev.data.fd; };

  /* epoll handle begin */

  int Wait(int ms);
  void Close() const;

  /* epoll handle end */

 private:

  static constexpr int kEventNum = 1024;

  int ep_;
  uint32 sched_id_;
  int wake_up_fd_;
  epoll_event* ev_;

  bool is_wakeup_;

};

}  // namespace co

}  // namespace tit



#endif  // TIT_COROUTINE_EPOLL_H
